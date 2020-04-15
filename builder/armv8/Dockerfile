ARG SDK=poky-glibc-x86_64-meta-toolchain-aarch64-toolchain-2.5.1.sh 
ARG ARCH=armv8

FROM ubuntu:xenial AS base
RUN apt-get update &&\
    apt-get -y \
        install build-essential docker.io \
        diffstat texinfo gawk chrpath wget cpio \
        language-pack-en \
        python \
        cmake
        
FROM base AS install
ARG SDK
USER root

COPY deploy/build/tmp/deploy/sdk/$SDK /tmp
RUN bash /tmp/$SDK -y

FROM base as sdk
ARG ARCH
COPY --from=install /opt/poky /opt/poky
COPY repository/builder/$ARCH/init.sh /init.sh
WORKDIR /W
ENTRYPOINT ["/init.sh"]

CMD ["/bin/bash"]
