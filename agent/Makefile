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
	${SERVER_BASE}/impl/Agent.cpp \
	${SERVER_BASE}/impl/Agent.h \
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
	$(EMPTY)

TEST_SRCS= \
	${SERVER_BASE}/main-api-server-test.cpp \
	${SERVER_BASE}/tests/Utils_test.cpp \
	${SERVER_BASE}/tests/Log_test.cpp \
	${SERVER_BASE}/tests/Agent_test.cpp \
	$(EMPTY)

CMAKE_SRCS= \
	${SERVER_BASE}/CMakeLists.txt \
	${SERVER_BASE}/CMakeToolchain.txt \
	${SERVER_BASE}/patch/onion.patch \
	$(EMPTY)

OVERLAY_SRCS=$(SERVER_SRCS) $(TEST_SRCS) $(CMAKE_SRCS)

OVERLAY_FILES=$(addprefix ${BUILD}/,$(OVERLAY_SRCS))

DIRS=${BUILD} ${BUILD}/${SERVER_BASE} \
	${BUILD}/${SERVER_BASE}/impl \
	${BUILD}/${SERVER_BASE}/tests \
	${BUILD}/${SERVER_BASE}/router \
	${BUILD}/${SERVER_BASE}/patch \
	${EMPTY}

default: ${BUILD}/oort-server

dirs: ${DIRS}

.SECONDEXPANSION:
$(DIRS): ${BUILD}%: | $$(@D)
	mkdir $@

$(OVERLAY_FILES): | ${BUILD}/oort-collector-api.bpl ${BUILD}/oort-sdk-api.bpl dirs

${BUILD}/%.bpl: ${SPEC_DIR}/%.yml | dirs
	docker run --rm \
		-v /:/local \
		-w /local/${CURDIR} \
		$(OAPI_GENERATOR) generate \
		-i $< \
		-g $(OAPI_TARGET) \
		-o ${BUILD}/$(SERVER_BASE)/ \
		--config $(SPEC_DIR)/oapi-config.yaml
	@rm -f $(OVERLAY_FILES)
	touch $@

${BUILD}/oort-sdk-api: ${BUILD}/oort-sdk-api.bpl 
	touch $@ 

${BUILD}/oort-collector-api: ${BUILD}/oort-collector-api.bpl
	touch $@

.PHONY: autogen
autogen: $(BUILD)/oort-sdk-api $(BUILD)/oort-collector-api $(OVERLAY_FILES)

$(OVERLAY_FILES): ${BUILD}/${SERVER_BASE}/%: ${SERVER_BASE}/%
	cp $< $@

${BUILD}/${SERVER_BASE}/Makefile: ${BUILD}/oort-sdk-api ${BUILD}/oort-collector-api $(addprefix ${BUILD}/,$(CMAKE_SRCS))
	cd ${BUILD}/$(SERVER_BASE) && \
		cmake ${CMAKE_EXTRA} . && \
		make ONION Catch2 NLOHMANN

${BUILD}/${SERVER_BASE}/api-server: $(OVERLAY_FILES) ${BUILD}/${SERVER_BASE}/Makefile 
	rm -f $@
	cd ${BUILD}/$(SERVER_BASE) && \
		$(MAKE) api-server/fast 

${BUILD}/${SERVER_BASE}/test_agent: $(OVERLAY_FILES) ${BUILD}/${SERVER_BASE}/Makefile 
	cd ${BUILD}/$(SERVER_BASE) && \
		$(MAKE) test_agent/fast 

${BUILD}/oort-server: ${BUILD}/${SERVER_BASE}/api-server
	cp ${BUILD}/${SERVER_BASE}/api-server $@

# .PHONY: ${SERVER_BASE}/impl/version.h
${SERVER_BASE}/impl/version.h:
	sh mk-version.sh > $@

full: $(OVERLAY_FILES) ${BUILD}/${SERVER_BASE}/Makefile 
	cd ${BUILD}/$(SERVER_BASE) && \
		cmake ${CMAKE_EXTRA} -D${LP_EXTRA} . && \
		$(MAKE) ${LP_EXTRA}
	cp ${BUILD}/${SERVER_BASE}/api-server ${BUILD}/oort-server

test: ${BUILD}/oort-server
	bash tests/test-api-server.sh

unittest: ${BUILD}/${SERVER_BASE}/test_agent
	${BUILD}/${SERVER_BASE}/test_agent

run: ${BUILD}/oort-server
	${BUILD}/oort-server -w /tmp/workdir.1

server: ${BUILD}/oort-server

lint:
	cpplint $(SERVER_SRCS)

.PHONY: clean-cmake clean-objs clean-overlay clean
clean-cmake: 
	cd ${BUILD} && \
	find . -name CMakeFiles -o -name CMakeCache.txt |xargs rm -rf

clean-objs: dirs
	cd ${BUILD} && \
	find . -name *.[ao] -o -name '*.so.*' -o -name '*.so' |xargs rm -rf

clean-overlay: dirs
	rm -f ${BUILD}/oort-sdk-api ${BUILD}/oort-collector-api ${BUILD}/*.bpl
	rm -f $(OVERLAY_FILES)

clean: clean-cmake clean-objs clean-overlay

cmake: $(OVERLAY_FILES) ${BUILD}/${SERVER_BASE}/Makefile 
	cd ${BUILD}/$(SERVER_BASE) && \
		cmake ${CMAKE_EXTRA} . && \
		make ONION Catch2 NLOHMANN

.PHONY: cross

BUILD_DOCKER_BASE = 784407737194.dkr.ecr.us-west-2.amazonaws.com/oort-builder/
# BUILD_DOCKER_BASE = sdk-

cross:
	$(MAKE) BUILD=build-cross-${ARCH} autogen
	docker run --rm \
		-v ${CURDIR}:/local \
		-w /local \
		-u`id -u` \
		${BUILD_DOCKER_BASE}${ARCH} make BUILD=build-cross-${ARCH} -j 4 ${TARGETS}

build-cross-%/oort-server: ${OVERLAY_SRCS}
	${MAKE} ARCH=$* cross TARGETS=$@
