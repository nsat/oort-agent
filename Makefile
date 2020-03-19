DIRS=spec agent ci
TOPTARGETS=lint

default: lint

# 
# rules to run command in all subdirectories
#
DIRTARGETS=$(addsuffix /.,$(DIRS))
$(TOPTARGETS): $(DIRTARGETS)
$(DIRTARGETS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: default lint $(DIRTARGETS)
