FROM ubuntu:18.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y software-properties-common \
        openssh-server ssh sudo rsync \
        python python-pip \
        daemontools monit

RUN pip install "python_dateutil >= 2.5.3" "urllib3 >= 1.15.1"
