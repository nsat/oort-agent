# run api-server under memcheck
# execute several requests
# kill api-server

# set -e

WORKDIR=/tmp/test.$$/workdir
OUTPUT=/tmp/test.$$/app.out

APP=build/oort-server
SHIM=tests/utils/unit_shim.py

mkdir -p $WORKDIR

echo workdir is $WORKDIR

error() {
    if [ -n "$blob" ]; then
        echo "$blob";
    fi
    cat $OUTPUT
    exit 1
}

cleanup() {
    if [ -n "$APP_PID" ]; then
        echo stopping $APP
        kill -INT $APP_PID
    fi
    if [ -n "$SHIM_PID" ]; then
        echo stopping $SHIM pid $SHIM_PID
        kill -INT $SHIM_PID
    fi
}
trap error ERR
trap cleanup 0

jq_assert() {
    blob=$1
    path=$2
    value=$3
    tag=${4-assertion failure}
    check=$(echo "$blob" | jq -r $path)
    if [ "$check" != "$value" ]; then
        echo $4 failed:
        echo "Expected '$value'; got '$check'"
        false
    fi
}

cget() {
    var=$1
    shift
    eval $var='$(curl -Ss "$@")'
}

if [ "$FULL" == "y" ]; then
_FULL="--show-leak-kinds=all"
fi

# start the shim server
(
    cd server
    echo starting shim
    python3 $SHIM 2>/dev/null &
    trap "kill -INT $!" INT
    wait
) &
SHIM_PID=$!

ulimit -n 128
valgrind --tool=memcheck --leak-check=full $_FULL --error-exitcode=9 \
    $APP -w $WORKDIR -s oort-test -c can0 -n 12 -N 51 > $OUTPUT 2>&1 &
APP_PID=$!

# sleep to let application start up
while ! nc localhost 2005 < /dev/null ; do 
    echo -n .
    sleep 0.25
done

cget blob http://localhost:2005/collector/v1/info -X POST
jq_assert "$blob" .sysinfo.version 1.1 "Version check"
jq_assert "$blob" .sysinfo.workdir $WORKDIR/transfers "Working directory"

diskfree=$(df -B1 $WORKDIR | awk 'NR~2 {print $4}')
jq_assert "$blob" .sysinfo.diskfree $diskfree "Disk free"

send_file=/tmp/ubbbu
echo "hello, world" > $send_file

crc=$(python -c 'print("%0x" % (0xffffffff&__import__("zlib").crc32(__import__("sys").stdin.read().encode())))' < $send_file)

request="$(jq -n --arg sendfile $send_file '.filepath=$sendfile|.destination="ground"|.topic="test"')"
cget blob http://localhost:2005/sdk/v1/send_file -H "Content-type: application/json" -d "$request"
id=$(echo "$blob" | jq -r .UUID)
jq_assert "$(cat $WORKDIR/transfers/$id.meta.oort)" .file_info.crc32 $crc "Crc-32"
test -e $WORKDIR/transfers/$id.data.oort || (echo "transfer file not found" && false)

# tfrs
cget blob http://localhost:2005/sdk/v1/tfrs
jq_assert "$blob" .utc_time 1234
jq_assert "$blob" .ecef_pos_x 2.5

# adcs
cget blob http://localhost:2005/sdk/v1/adcs
jq_assert "$blob" .hk.unix_timestamp 1234.5
jq_assert "$blob" .hk.euler_angles.pitch 90
jq_assert "$blob" .mode SUNSPIN

# adcs-command
cget blob http://localhost:2005/sdk/v1/adcs -H 'Content-type: application/json' -d '{"command": "IDLE", "aperture": "X"}'
jq_assert "$blob" .status 400
jq_assert "$blob" .message "Aperture X"

kill $APP_PID
wait $APP_PID 2> /dev/null || true
unset APP_PID

# _valgrind_exit=$?
# if [ $_valgrind_exit != 0 ]; then
    # cat $OUTPUT
    # exit $_valgrind_exit
# fi

# scan valgrind output for "definitely lost" errors
if ! grep -q "definitely lost: 0 bytes in 0 blocks" $OUTPUT ; then
    echo Valgrind errors:
    cat $OUTPUT
    exit 1
else
    echo no errors from valgrind
    exit 0
fi
