## default - run lint
default: lint

.PHONY: help
## Available targets
help:
	@echo
	@sed -n -e 's/^## //p' Makefile
	@exit 1

DIRS=spec agent ci
TOPTARGETS=lint

# 
# rules to run command in all subdirectories
#
DIRTARGETS=$(addsuffix /.,$(DIRS))
$(TOPTARGETS): $(DIRTARGETS)
$(DIRTARGETS):
	$(MAKE) -C $@ ${MAKECMDGOALS}

## unittest - run unit tests
unittest: agent/.

.PHONY: default lint unittest $(DIRTARGETS)
