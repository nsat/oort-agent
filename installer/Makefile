# recursive wildcard function
# from https://stackoverflow.com/a/18258352/796950
rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

INSTALLERS=\
	install-armv7a.zip install-armv8.zip install-x86.zip \
	install-armv7a.tar.gz install-armv8.tar.gz install-x86.tar.gz 

$(filter %.zip,${INSTALLERS}): install-%.zip: install-%/oort-agent $(call rwildcard,install,*)
	cp -rp install/* install-$*/
	zip -ru $@ install-$*/

$(filter %.tar.gz,${INSTALLERS}): install-%.tar.gz: install-%/oort-agent $(call rwildcard,install,*)
	cp -rp install/* install-$*/
	tar czf $@ install-$*/

install-%/oort-agent: ../agent/build-cross-%/oort-server
	test -d install-$* || mkdir install-$*
	cp ../agent/build-cross-$*/oort-server $@

../agent/build-cross-%/oort-server:
	cd ../agent && ${MAKE} build-cross-$*/oort-server
