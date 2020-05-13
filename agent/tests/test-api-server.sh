# run api-server under memcheck
# execute several requests
# kill api-server

# set -e

WORKDIR=/tmp/test.$$/workdir
OUTPUT=/tmp/test.$$/app.out

APP=build/oort-server

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

ulimit -n 128
valgrind --tool=memcheck --leak-check=full $_FULL --error-exitcode=9 \
    $APP -w $WORKDIR -s oort-test > $OUTPUT 2>&1 &
APP_PID=$!

# sleep to let application start up
while ! nc localhost 2005 < /dev/null ; do 
    echo -n .
    sleep 0.25
done

cget blob http://localhost:2005/collector/v1/info -X POST
jq_assert "$blob" .sysinfo.version 1.0 "Version check"
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
