default: lint

.PHONY: help
## Available targets
help:
	@echo
	@sed -n -e 's/^## //p' Makefile
	@exit 1

DIRS=spec agent ci
TOPTARGETS=lint

## default - run lint

## docs - regenerate slate docs
docs:
	docker run --rm \
	-v ${PWD}/docs:/srv/slate/build \
	-v ${PWD}/oort-docs:/srv/slate/source/includes \
	spire-slate bundle exec middleman build

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
