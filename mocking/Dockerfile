FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install -y software-properties-common \
        openssh-server ssh sudo rsync \
        python3 python3-pip \
        daemontools monit \
        iproute2

RUN pip3 config --global set global.index_url https://devpi.cloud.spire.com/root/prod
RUN pip3 install "python_dateutil >= 2.5.3" "urllib3 >= 1.15.1" "uavcan==1.0.0.dev33" "pypredict"
USER 1000
