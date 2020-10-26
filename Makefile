DIRS=spec agent ci
TOPTARGETS=lint

default: lint

# 
# rules to run command in all subdirectories
#
DIRTARGETS=$(addsuffix /.,$(DIRS))
$(TOPTARGETS): $(DIRTARGETS)
$(DIRTARGETS):
	$(MAKE) -C $@ ${MAKECMDGOALS}

unittest: agent/.

.PHONY: default lint unittest $(DIRTARGETS)
