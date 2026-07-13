DEFAULT_OPENAPI_IMAGE=openapitools/openapi-generator-cli:v7.20.0
C_OPENAPI_IMAGE=openapitools/openapi-generator-cli:v6.6.0
PY2_OPENAPI_IMAGE=openapitools/openapi-generator-cli:v4.2.3
# v4.2.3 doesn't include an arm64 imge and its OpenJDK 8 JVM deadlocks under
# enumlation user qemu-user, so we cant just run it using `--platform=linux/amd64` on arm64 hosts.
# The generator jar is platform independent so we extact this from the image
# and run that under a mutl-arch supporting JRE.
# NOTE: When ``OAPI_VERSION`` is upgraded to a newer version which includes an arm64 image, this can be removed.
PY2_OAPI_JAR=build/openapi-generator-cli-v4.2.3.jar
OAPI_RUNNER=eclipse-temurin:21-jre
OAPI_JAR_IN_IMAGE=/opt/openapi-generator/modules/openapi-generator-cli/target/openapi-generator-cli.jar
.PHONY: lint python3-test python3-lint python3-dist python3-devpi-upload

ROOT=$(realpath ${CURDIR}/..)
CUR=$(notdir ${CURDIR})

OORT_AGENT ?= $(realpath $(CURDIR)/..)/agent/build/oort-server
PY3_TEST_DIR = samples/python3-functional

SPECS=oort-collector-api oort-sdk-api
CLIENTS=c python python3 bash

DEVPI_URL ?= https://devpi.cloud.spire.com
DEVPI_INDEX ?= root/prod

SAMPLES=$(wildcard samples/python/*)

sdk_build = docker run --rm \
	  -v ${ROOT}:/local -w /local/${CUR} \
	  -u `id -u` \
	  ${2} generate \
	  -i ../spec/oort-sdk-api.yml \
	  -o ${3}-sdk-client/ \
	  -g ${1} \
	  -p packageName=oort_sdk_client \
	  -t templates/${3} \
	  -c templates/${3}/config.yaml

default: $(CLIENTS) collector

collector: python-collector-client/README.md

lint:
	@echo ok

$(CLIENTS): %: %-sdk-client/README.md

%-sdk-client.zip: %-sdk-client/README.md ${SAMPLES}
	zip -ur $*-sdk-client.zip $*-sdk-client/ samples/$*/

%-sdk-client/README.md:
	$(call sdk_build,$*,$(DEFAULT_OPENAPI_IMAGE),$*)

# C client currently built with this particular version. A newer one might have
# improvements, but we don't have spare time or adequate testing to change this
# right now.
c-sdk-client/README.md:
	$(call sdk_build,c,$(C_OPENAPI_IMAGE),c)

# openapi-generator deprecated py2 support and reused the generator name
# "python" for py3. Map both make targets to the python generator, and
# build py2 with the last generator version that supported it.
python3-sdk-client/README.md:
	$(call sdk_build,python,$(DEFAULT_OPENAPI_IMAGE),python3) -p packageVersion=$(shell cat ../VERSION)
	cp -r samples/python3-functional python3-sdk-client/test/functional
	BIN=$(ROOT)/agent/build/oort-server
	if [ -f "$$BIN" ]; then
		cp -p "$$BIN" python3-sdk-client/test/functional/oort-server
	else
		echo "WARNING: $$BIN not built; functional tests require OORT_AGENT"
	fi

$(PY2_OAPI_JAR): ../agent/extract-openapi-jar.sh
	mkdir -p $(dir $@)
	$< $(PY2_OPENAPI_IMAGE) $(OAPI_JAR_IN_IMAGE) $@

python-sdk-client/README.md: $(PY2_OAPI_JAR)
	docker run --rm \
	  -v ${ROOT}:/local -w /local/${CUR} \
	  -u `id -u` \
	  --entrypoint java \
	  $(OAPI_RUNNER) \
	  -jar $(PY2_OAPI_JAR) generate \
	  -i ../spec/oort-sdk-api.yml \
	  -o python-sdk-client/ \
	  -g python \
	  -p packageName=oort_sdk_client \
	  -t templates/python \
	  -c templates/python/config.yaml

.ONESHELL:
python3-test: python3
	pwd
	echo "START DIR: $$(pwd)"
	cd python3-sdk-client
	poetry install -q --with dev
	cd $(PY3_TEST_DIR)
	poetry install -q --with dev && \
		OORT_AGENT=$(OORT_AGENT) poetry run pytest . -v --cov=oort_sdk_client

.ONESHELL:
python3-lint:
	pwd
	echo "START DIR: $$(pwd)"
	#   Always reformat the python3-sdk-client
	cd python3-sdk-client
	poetry run black .

	cd ./samples/python3-functional
	poetry install -q --with dev
	poetry run black --check .

python3-dist: python3
	cd python3-sdk-client && pip3 install build && python3 -m build

python3-devpi-upload: python3-dist
	pip3 install devpi-client==7.3.0
	devpi use $(DEVPI_URL)/$(DEVPI_INDEX)
	devpi login $$DEVPI_USER --password=$$DEVPI_PASSWORD
	devpi upload --from-dir python3-sdk-client/dist/

%-collector-client.zip: %-collector-client/README.md ${SAMPLES}
	zip -ur $*-collector-client.zip $*-collector-client/ samples/$*/

%-collector-client/README.md:
	docker run --rm \
	  -v ${ROOT}:/local -w /local/${CUR} \
	  -u `id -u` \
	  ${DEFAULT_OPENAPI_IMAGE} generate \
	  -i ../spec/oort-collector-api.yml \
	  -o $*-collector-client/ \
	  -g $* \
	  -p packageName=oort_collector_client \
	  -t templates/$*

.PHONY: c-test
c-test:
	$(MAKE) -C samples/c test OORT_AGENT=$(OORT_AGENT)

