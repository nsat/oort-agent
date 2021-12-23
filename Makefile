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
	-v ${PWD}/documentation:/srv/slate/source/includes \
	spire-slate bundle exec middleman build

## docs-serve - regenerate slate docs
docs-serve:
	docker run --rm --net=host \
	-v ${PWD}/docs:/srv/slate/build \
	-v ${PWD}/documentation:/srv/slate/source/includes \
	spire-slate bundle exec middleman server --bind-address 0.0.0.0

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
