ARG SDK=poky-glibc-x86_64-meta-toolchain-cortexa9hf-neon-toolchain-2.3.2.sh
ARG ARCH=cortexa9

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

COPY deploy/$SDK /tmp
RUN bash /tmp/$SDK -y

FROM base as sdk
ARG ARCH
COPY --from=install /opt/poky /opt/poky
COPY repository/builder/$ARCH/init.sh /init.sh
WORKDIR /W
ENTRYPOINT ["/init.sh"]

CMD ["/bin/bash"]
