.PHONY: help
## Available targets
help:
	@echo
	@sed -n -e 's/^## //p' Makefile
	@exit 1

.PHONY: install-pr install-build trigger-build install-release trigger-release lint

TEAM = constellation
PREFIX = oort
TARGET = $(PREFIX)

## login - login to concourse
login:
	fly -t $(TARGET) login --team-name=$(TEAM) --concourse-url=https://ci.spire.sh

## install-pr - install the pull-request pipeline
install-pr:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):agent-pr -c oort-pr.yml

## install-buildsdk - install the client sdk builder pipeline
install-buildsdk:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):sdk-release -c oort-sdk-release.yml -n

## trigger-buildsdk - manually trigger the client sdk builder pipeline
trigger-buildsdk:
	fly -t $(TARGET) trigger-job -j $(PREFIX):sdk-release/build-python-SDK
	fly -t $(TARGET) trigger-job -j $(PREFIX):sdk-release/build-c-SDK

## install-build - install the cross-compiler builder pipeline
install-build:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):sdk-builder -c oort-builder.yml -n

## trigger-build - manually trigger the cross-compiler builder pipeline
trigger-build:
	fly -t $(TARGET) trigger-job -j $(PREFIX):sdk-builder/build-sdk-armv8
	fly -t $(TARGET) trigger-job -j $(PREFIX):sdk-builder/build-sdk-armv7a

## install-release - install the release builder pipeline
install-release:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):agent-release -c oort-agent-release.yml

## trigger-release - manually trigger the release builder pipeline for all architectures
trigger-release:
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-x86
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-armv7a
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-armv8

## lint - spread good cheer.
lint:
	@echo hope this is ok
