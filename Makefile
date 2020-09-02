DIRS=spec agent ci
TOPTARGETS=lint

default: lint

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
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: docs default lint $(DIRTARGETS)
