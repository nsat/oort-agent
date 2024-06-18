OPENAPI_IMAGE=openapitools/openapi-generator-cli:v7.6.0
.PHONY: lint validate ${SPECS}

SPECS=oort-collector-api oort-sdk-api

lint: validate

validate: ${SPECS}

${SPECS}:
	docker run --rm \
	  -v /:/local -w /local/${CURDIR} ${OPENAPI_IMAGE} validate -i $@.yml
