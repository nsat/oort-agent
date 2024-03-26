FROM 784407737194.dkr.ecr.us-west-2.amazonaws.com/spire-base:xenial AS base
RUN apt-get update &&\
    apt-get -y \
        install build-essential docker.io \
        diffstat texinfo gawk chrpath wget cpio \
        language-pack-en \
        cmake \
        bash curl util-linux \
        python-pip python-dev libffi-dev \
        python3 \
        gcc g++ libc-dev make cmake git libz-dev iptables \
        inetutils-ping

RUN useradd spire

ENV DOCKER_VERSION=18.09.8 \
    DOCKER_COMPOSE_VERSION=1.24.1

WORKDIR /

RUN curl https://download.docker.com/linux/static/stable/x86_64/docker-${DOCKER_VERSION}.tgz | tar zx && \
    mv /docker/* /bin/ && \
    chmod +x /bin/docker* && \
    pip install docker-compose==${DOCKER_COMPOSE_VERSION} && \
    rm -rf /root/.cache

# Include functions to start/stop docker daemon
COPY docker-lib.sh /docker-lib.sh
COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["/bin/bash"]
