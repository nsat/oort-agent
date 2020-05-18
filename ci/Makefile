.PHONY: install lint

install-pr:
	fly -t oort set-pipeline -p oort-agent:pr -c oort-pr.yml

install-build:
	fly -t oort set-pipeline -p oort:sdk-builder -c oort-builder.yml -n

trigger-build:
	fly -t oort trigger-job -j oort:sdk-builder/build-sdk-armv8 
	fly -t oort trigger-job -j oort:sdk-builder/build-sdk-armv7a

install-release:
	fly -t oort set-pipeline -p oort:agent-release -c oort-agent-release.yml

trigger-release:
	fly -t oort trigger-job -j oort:agent-release/build-agent-x86
	fly -t oort trigger-job -j oort:agent-release/build-agent-armv7a
	fly -t oort trigger-job -j oort:agent-release/build-agent-armv8

install-release:
	fly -t oort set-pipeline -p oort:agent-release -c oort-agent-release.yml

trigger-release:
	fly -t oort trigger-job -j oort:agent-release/build-agent-x86
	fly -t oort trigger-job -j oort:agent-release/build-agent-armv7a
	fly -t oort trigger-job -j oort:agent-release/build-agent-armv8

lint:
	@echo hope this is ok
  
