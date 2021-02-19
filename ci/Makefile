.PHONY: install lint

# Add hyphen at end, e.g. "red-"
EXPORT_COLOR ?= red-

TEAM = $(EXPORT_COLOR)constellation
PREFIX = oort
TARGET = $(EXPORT_COLOR)$(PREFIX)

login:
	fly -t $(TARGET) login --team-name=$(TEAM) --concourse-url=https://ci.spire.sh

install-pr:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):agent-pr -c oort-pr.yml

install-build:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):sdk-builder -c oort-builder.yml -n

trigger-build:
	fly -t $(TARGET) trigger-job -j $(PREFIX):sdk-builder/build-sdk-armv8
	fly -t $(TARGET) trigger-job -j $(PREFIX):sdk-builder/build-sdk-armv7a

install-release:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):agent-release -c oort-agent-release.yml

trigger-release:
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-x86
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-armv7a
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-armv8

install-release:
	fly -t $(TARGET) set-pipeline -p $(PREFIX):agent-release -c oort-agent-release.yml

trigger-release:
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-x86
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-armv7a
	fly -t $(TARGET) trigger-job -j $(PREFIX):agent-release/build-agent-armv8

lint:
	@echo hope this is ok
