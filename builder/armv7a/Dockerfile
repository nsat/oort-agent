FROM docker-registry.cloud.spire.com/spire-base:xenial AS base
# FROM amidos/dcind:2
RUN apt-get update &&\
    apt-get -y \
        install build-essential docker.io \
        diffstat texinfo gawk chrpath wget cpio \
        cmake
FROM base AS builder-poky

RUN useradd spire
WORKDIR /W
RUN chown spire /W
USER spire
RUN git clone --depth 1 -b fido git://github.com/nsat/poky

USER root
WORKDIR /W/poky/build
RUN chown spire /W/poky/build
COPY --chown=spire:spire conf/ conf/
WORKDIR /W/poky
COPY --chown=spire:spire meta-oort/ meta-oort/

USER spire
RUN bash -c ". oe-init-build-env && bitbake meta-toolchain -c populate_sdk"

FROM builder-poky AS sdk-install

USER root
COPY --from=builder-poky /W/poky/build/tmp/deploy/sdk/poky-glibc-x86_64-meta-toolchain-armv7a-vfp-toolchain-1.8.2.sh /tmp/
RUN bash /tmp/poky-glibc-x86_64-meta-toolchain-armv7a-vfp-toolchain-1.8.2.sh -y 

FROM base as sdk
COPY --from=sdk-install /opt/poky /opt/poky
COPY init.sh /init.sh
WORKDIR /W
ENTRYPOINT ["/init.sh"]

CMD ["/bin/bash"]
