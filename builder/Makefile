.PHONY: docker armv7a armv8

docker: armv7a armv8

armv7a:
	docker build armv7a --target builder-poky -t builder-poky
	docker build armv7a --target sdk -t sdk

armv8:
	docker build armv8 --target builder-poky-sumo -t builder-poky-sumo
	docker build armv8 --target sdk-armv8 -t sdk-armv8
