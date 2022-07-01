default: _default
.PHONY: help
## Available targets
help:
	@echo
	@sed -n -e 's/^## //p' Makefile
	@exit 1

.PHONY: test lint
# recursive wildcard function
# from https://stackoverflow.com/a/18258352/796950
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

OAPI_GENERATOR = openapitools/openapi-generator-cli:v4.2.3
OAPI_TARGET = cpp-pistache-server

SERVER_BASE = server

SPEC_DIR = ../spec

ifneq "${ARCH}" ""
# CMAKE_EXTRA=-D CMAKE_TOOLCHAIN_FILE=CMakeToolchain.txt 
BUILD=build-cross-${ARCH}
else
BUILD=build
LP_EXTRA=LIBPATH=${LIBDIR}
endif


SERVER_SRCS = \
	${SERVER_BASE}/impl/version.h \
	${SERVER_BASE}/impl/Adaptor.cpp \
	${SERVER_BASE}/impl/Adaptor.h \
	${SERVER_BASE}/impl/Adcs.cpp \
	${SERVER_BASE}/impl/Adcs.h \
	${SERVER_BASE}/impl/Agent.cpp \
	${SERVER_BASE}/impl/Agent.h \
	${SERVER_BASE}/impl/AgentUAVCANClient.cpp \
	${SERVER_BASE}/impl/AgentUAVCANClient.h \
	${SERVER_BASE}/impl/AgentUAVCANServer.cpp \
	${SERVER_BASE}/impl/AgentUAVCANServer.h \
	${SERVER_BASE}/impl/Cleaner.cpp \
	${SERVER_BASE}/impl/Cleaner.h \
	${SERVER_BASE}/impl/Config.cpp \
	${SERVER_BASE}/impl/Config.h \
	${SERVER_BASE}/impl/Files.cpp \
	${SERVER_BASE}/impl/Files.h \
	${SERVER_BASE}/impl/Log.cpp \
	${SERVER_BASE}/impl/Log.h \
	${SERVER_BASE}/impl/OnionLog.cpp \
	${SERVER_BASE}/impl/OnionLog.h \
	${SERVER_BASE}/main-api-server.cpp \
	${SERVER_BASE}/impl/SdkApiImpl.cpp \
	${SERVER_BASE}/impl/SdkApiImpl.h \
	${SERVER_BASE}/impl/CollectorApiImpl.cpp \
	${SERVER_BASE}/impl/CollectorApiImpl.h \
	${SERVER_BASE}/impl/Utils.cpp \
	${SERVER_BASE}/impl/Utils.h \
	${SERVER_BASE}/router/CollectorApiRouter.cpp \
	${SERVER_BASE}/router/CollectorApiRouter.h \
	${SERVER_BASE}/router/SdkApiRouter.cpp \
	${SERVER_BASE}/router/SdkApiRouter.h \
	${SERVER_BASE}/router/RouterUtil.h \
	${EMPTY}

TEST_SRCS= \
	${SERVER_BASE}/main-api-server-test.cpp \
	${SERVER_BASE}/tests/Utils_test.cpp \
	${SERVER_BASE}/tests/Log_test.cpp \
	${SERVER_BASE}/tests/Adaptor_test.cpp \
	${SERVER_BASE}/tests/Adcs_test.cpp \
	${SERVER_BASE}/tests/Agent_test.cpp \
	${SERVER_BASE}/tests/Healthcheck_test.cpp \
	${SERVER_BASE}/tests/Files_test.cpp \
	${SERVER_BASE}/tests/utils/hk_error.sh \
	${SERVER_BASE}/tests/utils/hk_garbage.sh \
	${SERVER_BASE}/tests/utils/hk_hanging.sh \
	${SERVER_BASE}/tests/utils/hk_invalid_int_metrics.sh \
	${SERVER_BASE}/tests/utils/hk_invalid_json.sh \
	${SERVER_BASE}/tests/utils/hk_long_array.sh \
	${SERVER_BASE}/tests/utils/hk_long_response.sh \
	${SERVER_BASE}/tests/utils/hk_success.sh \
	${SERVER_BASE}/tests/utils/hk_trailing_garbage.sh \
	${EMPTY}

CMAKE_SRCS= \
	${SERVER_BASE}/CMakeLists.txt \
	${SERVER_BASE}/CMakeToolchain.txt \
	${SERVER_BASE}/patch/onion.patch \
	${EMPTY}

THIRDPARTY_SRCS = \
	${SERVER_BASE}/thirdparty/libuavcan \
	${SERVER_BASE}/thirdparty/libuavcan_linux \
	${EMPTY}

USSP_SRCS= \
	${SERVER_BASE}/ussp/tfrs/223.ReceiverNavigationState.uavcan \
	${SERVER_BASE}/ussp/payload/225.PayloadAdcsCommand.uavcan \
	${SERVER_BASE}/ussp/payload/226.PayloadHealthCheck.uavcan \
	${SERVER_BASE}/ussp/payload/233.PayloadAdcsFeed.uavcan \
	$(SERVER_BASE)/ussp/payload/AcsMode.uavcan \
	$(SERVER_BASE)/ussp/payload/TargetT.uavcan \
	$(SERVER_BASE)/ussp/payload/QuatT.uavcan \
	$(SERVER_BASE)/ussp/payload/XyzFloatT.uavcan \
	${EMPTY}

OVERLAY_SRCS=$(SERVER_SRCS) $(TEST_SRCS) $(CMAKE_SRCS)

OVERLAY_FILES=$(addprefix ${BUILD}/,$(OVERLAY_SRCS))
OVERLAY_THIRDPARTY_FILES=$(addprefix ${BUILD}/,$(THIRDPARTY_SRCS))
OVERLAY_USSP_FILES=$(addprefix ${BUILD}/,$(USSP_SRCS))
ALL_OVERLAY_FILES=$(OVERLAY_FILES) $(OVERLAY_THIRDPARTY_FILES) $(OVERLAY_USSP_FILES)

DIRS=${BUILD} ${BUILD}/${SERVER_BASE} \
	${BUILD}/${SERVER_BASE}/thirdparty \
	${BUILD}/${SERVER_BASE}/impl \
	${BUILD}/${SERVER_BASE}/tests \
	${BUILD}/${SERVER_BASE}/tests/utils \
	${BUILD}/${SERVER_BASE}/router \
	${BUILD}/${SERVER_BASE}/patch \
	${BUILD}/${SERVER_BASE}/ussp \
	${BUILD}/${SERVER_BASE}/ussp/payload \
	${BUILD}/${SERVER_BASE}/ussp/tfrs \
	${EMPTY}

UAVCAN_DEPS=${BUILD}/${SERVER_BASE}/thirdparty/libuavcan/libuavcan/libuavcan.a \
	${BUILD}/${SERVER_BASE}/dsdlc_generated \
	${EMPTY}

## default - build oort-server in build subdir
_default: ${BUILD}/oort-server

dirs: ${DIRS}

.SECONDEXPANSION:
$(DIRS): ${BUILD}%: | $$(@D)
	mkdir $@

$(OVERLAY_FILES): | ${BUILD}/oort-collector-api.bpl ${BUILD}/oort-sdk-api.bpl dirs
$(OVERLAY_THIRDPARTY_FILES): | dirs
$(OVERLAY_USSP_FILES): | dirs

${BUILD}/%.bpl: ${SPEC_DIR}/%.yml | dirs
	docker run --rm \
		-v /:/local \
		-w /local/${CURDIR} \
		-u`id -u` \
		$(OAPI_GENERATOR) generate \
		-i $< \
		-g $(OAPI_TARGET) \
		-o ${BUILD}/$(SERVER_BASE)/ \
		--config $(SPEC_DIR)/oapi-config.yaml
	@touch -c -d@0 $(OVERLAY_FILES)
	touch $@

# blueprint included files
${BUILD}/%.binc: ${SPEC_DIR}/%.yml
	touch $@

${BUILD}/oort-sdk-api.bpl: ${BUILD}/adcs-schema.binc ${BUILD}/adcs-hk.binc

${BUILD}/oort-sdk-api: ${BUILD}/oort-sdk-api.bpl 
	touch $@ 

${BUILD}/oort-collector-api: ${BUILD}/oort-collector-api.bpl
	touch $@

.PHONY: autogen
## autogen - regenerate all the automatically generated files
autogen: $(BUILD)/oort-sdk-api $(BUILD)/oort-collector-api $(ALL_OVERLAY_FILES)

$(OVERLAY_FILES): ${BUILD}/${SERVER_BASE}/%: ${SERVER_BASE}/%
	cp $< $@

$(OVERLAY_THIRDPARTY_FILES): ${BUILD}/${SERVER_BASE}/thirdparty/%: ${SERVER_BASE}/thirdparty/%
	cp -r $< $@

$(OVERLAY_USSP_FILES): ${BUILD}/${SERVER_BASE}/ussp/%: ${SERVER_BASE}/ussp/%
	cp $< $@

${BUILD}/${SERVER_BASE}/Makefile: $(ALL_OVERLAY_FILES) ${BUILD}/oort-sdk-api ${BUILD}/oort-collector-api $(addprefix ${BUILD}/,$(CMAKE_SRCS))
	cd ${BUILD}/$(SERVER_BASE) && \
		cmake ${CMAKE_EXTRA} . && \
		make ONION Catch2 NLOHMANN

${BUILD}/${SERVER_BASE}/thirdparty/libuavcan/libuavcan/libuavcan.a: ${BUILD}/${SERVER_BASE}/Makefile $(ALL_OVERLAY_FILES)
	cd ${BUILD}/$(SERVER_BASE) && \
		$(MAKE) uavcan

${BUILD}/${SERVER_BASE}/dsdlc_generated: ${BUILD}/${SERVER_BASE}/Makefile $(ALL_OVERLAY_FILES) ${BUILD}/${SERVER_BASE}/thirdparty/libuavcan/libuavcan/libuavcan.a
	cd ${BUILD}/$(SERVER_BASE) && \
		$(MAKE) dsdlc

${BUILD}/${SERVER_BASE}/api-server: ${BUILD}/${SERVER_BASE}/Makefile $(ALL_OVERLAY_FILES) $(UAVCAN_DEPS)
	rm -f $@
	cd ${BUILD}/$(SERVER_BASE) && \
		$(MAKE) api-server/fast

${BUILD}/${SERVER_BASE}/test_agent: ${BUILD}/${SERVER_BASE}/Makefile $(ALL_OVERLAY_FILES) $(UAVCAN_DEPS)
	cd ${BUILD}/$(SERVER_BASE) && \
		$(MAKE) test_agent/fast 

${BUILD}/oort-server: ${BUILD}/${SERVER_BASE}/api-server
	cp ${BUILD}/${SERVER_BASE}/api-server $@

# .PHONY: ${SERVER_BASE}/impl/version.h
${SERVER_BASE}/impl/version.h:
	sh mk-version.sh > $@

## full - build everything
full: ${BUILD}/${SERVER_BASE}/Makefile $(ALL_OVERLAY_FILES)
	cd ${BUILD}/$(SERVER_BASE) && \
		cmake ${CMAKE_EXTRA} -D${LP_EXTRA} . && \
		$(MAKE) ${LP_EXTRA}
	cp ${BUILD}/${SERVER_BASE}/api-server ${BUILD}/oort-server

## test - run unit tests wrapper
test: ${BUILD}/oort-server
	bash tests/test-api-server.sh

## unittest - just unit tests.  TESTS passed to test program
unittest: ${BUILD}/${SERVER_BASE}/test_agent
	${BUILD}/${SERVER_BASE}/test_agent ${TESTS}

COMPOSE_FILE = ../mocking/docker-compose.yml

## unittest-it - docker-compose integration test.  TESTS passed to test program
unittest-it: TESTS ?= '[adcs-integration]'
unittest-it: ${BUILD}/${SERVER_BASE}/test_agent
	trap 'docker-compose -f ${COMPOSE_FILE} down' 0 ;\
	export TESTS=${TESTS} UID GID=$$(id -g); \
	docker-compose -f ${COMPOSE_FILE} run unittest

## run-it - docker-compose integration test
run-it: $(BUILD)/oort-server
	export UID GID=$$(id -g); \
	docker-compose -f ${COMPOSE_FILE} up oort-agent bouncy

## run - start up oort-server locally
run: ${BUILD}/oort-server
	mkdir /tmp/workdir.1 || true
	${BUILD}/oort-server -l debug -w /tmp/workdir.1

## runcan -- run with can interface enabled
runcan: ${BUILD}/oort-server
	${BUILD}/oort-server -w /tmp/workdir.1 -l debug -c can0 -n 12 -N 51

## gdb-runcan -- run under gdb with can interface enabled
gdb-runcan: ${BUILD}/oort-server
	gdb --args ${BUILD}/oort-server -w /tmp/workdir.1 -l debug -c can0 -n 12 -N 51

## vg-runcan -- run under valgrind with can interface enabled
vg-runcan: ${BUILD}/oort-server
	valgrind --tool=memcheck --show-leak-kinds=all --leak-check=full ${BUILD}/oort-server -w /tmp/workdir.1 -l debug -c can0 -n 12

## server - build oort-server 
server: ${BUILD}/oort-server

## lint - run cpplint against server sources
lint:
	cpplint $(SERVER_SRCS)

.PHONY: clean-cmake clean-objs clean-overlay clean
## clean-cmake - remove cmake generated files
clean-cmake: 
	cd ${BUILD} && \
	find . -name CMakeFiles -o -name CMakeCache.txt |xargs rm -rf

## clean-objs - remove object files
clean-objs: dirs
	cd ${BUILD} && \
	find . -name *.[ao] -o -name '*.so.*' -o -name '*.so' |xargs rm -rf

## clean-overlay - remove overlay files (files that are explicitly replacing
##   automatically generated ones)
clean-overlay: dirs
	rm -f ${BUILD}/oort-sdk-api ${BUILD}/oort-collector-api ${BUILD}/*.bpl
	rm -f $(OVERLAY_FILES)
	rm -rf $(OVERLAY_THIRDPARTY_FILES)
	rm -rf $(OVERLAY_USSP_FILES)
	rm -rf ${BUILD}/${SERVER_BASE}/dsdlc_generated

## clean - clean all the things
clean: clean-cmake clean-objs clean-overlay

## cmake - run cmake commands and build exernal dependencies
cmake: ${BUILD}/${SERVER_BASE}/Makefile $(ALL_OVERLAY_FILES)
	cd ${BUILD}/$(SERVER_BASE) && \
		cmake ${CMAKE_EXTRA} . && \
		make ONION Catch2 NLOHMANN

.PHONY: cross

BUILD_DOCKER_BASE = sdk-

## cross - run the build for a cross-compiler, using the cross-compiler builder docker
cross:
	$(MAKE) BUILD=build-cross-${ARCH} autogen
	docker run --rm \
		-v ${CURDIR}:/local \
		-w /local \
		-u`id -u` \
		${BUILD_DOCKER_BASE}${ARCH} make BUILD=build-cross-${ARCH} -j 4 ${TARGETS}

build-cross-%/oort-server: ${OVERLAY_SRCS}
	${MAKE} ARCH=$* cross TARGETS=$@
