FROM docker-registry.cloud.spire.com/spire-base:xenial AS base
# FROM amidos/dcind:2
RUN apt-get update &&\
    apt-get -y \
        install build-essential docker.io \
        diffstat texinfo gawk chrpath wget cpio \
        language-pack-en \
        cmake
FROM base AS builder-poky-sumo

RUN useradd spire
WORKDIR /W
RUN chown spire /W
USER spire
RUN git clone --depth 1 -b sumo git://github.com/nsat/poky

USER root
WORKDIR /W/poky/build
RUN chown spire /W/poky/build
COPY --chown=spire:spire conf/ conf/
WORKDIR /W/poky
COPY --chown=spire:spire meta-oort/ meta-oort/

USER spire
ENV LANG=en_US.UTF-8
RUN bash -c ". oe-init-build-env && bitbake meta-toolchain -c populate_sdk"

FROM builder-poky-sumo AS sdk-install-armv8

USER root
COPY --from=builder-poky-sumo /W/poky/build/tmp/deploy/sdk/poky-glibc-x86_64-meta-toolchain-aarch64-toolchain-2.5.1.sh /tmp/
RUN bash /tmp/poky-glibc-x86_64-meta-toolchain-aarch64-toolchain-2.5.1.sh -y 

FROM base as sdk-armv8
COPY --from=sdk-install-armv8 /opt/poky /opt/poky
COPY init.sh /init.sh
WORKDIR /W
ENTRYPOINT ["/init.sh"]

CMD ["/bin/bash"]
