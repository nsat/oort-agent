OPENAPI_IMAGE=openapitools/openapi-generator-cli:v4.2.3
.PHONY: lint

SPECS=oort-collector-api oort-sdk-api
CLIENTS=c python bash

SAMPLES=$(wildcard samples/python/*)

default: $(CLIENTS) collector

collector: python-collector-client/README.md

lint: 
	@echo ok

$(CLIENTS): %: %-sdk-client/README.md

%-sdk-client.zip: %-sdk-client/README.md ${SAMPLES}
	zip -ur $*-sdk-client.zip $*-sdk-client/ samples/$*/

%-sdk-client/README.md:
	docker run --rm \
	  -v /:/local -w /local/${CURDIR} \
	  ${OPENAPI_IMAGE} generate \
	  -i ../spec/oort-sdk-api.yml \
	  -o $*-sdk-client/ \
	  -g $* \
	  -p packageName=oort_sdk_client \
	  -t templates/$*

%-collector-client.zip: %-collector-client/README.md ${SAMPLES}
	zip -ur $*-collector-client.zip $*-collector-client/ samples/$*/

%-collector-client/README.md:
	docker run --rm \
	  -v /:/local -w /local/${CURDIR} \
	  ${OPENAPI_IMAGE} generate \
	  -i ../spec/oort-collector-api.yml \
	  -o $*-collector-client/ \
	  -g $* \
	  -p packageName=oort_collector_client \
	  -t templates/$*

