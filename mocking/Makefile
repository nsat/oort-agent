
docker-base:
	docker volume create -o type=tmpfs -o device=tmpfs -o o=size=10m worktmp
	docker build . -t agent-base
	touch docker-base

docker: docker-base artifacts/oort-agent files
	docker build . -f monit/Dockerfile -t agent-board-monit
	docker run --rm -ti --network=host \
		--hostname agent-board \
		-p 2022:2022 \
		--name agent-board \
		--init \
		-v `pwd`/dropbox:/dropbox \
		-v worktmp:/workdir agent-board-monit

docker-daemon: docker-base artifacts/oort-agent files
	docker build . -f daemontools/Dockerfile -t agent-board-daemon
	docker run --rm -ti --network=host \
		--hostname agent-board \
		-p 2022:2022 \
		--name agent-board-daemon \
		--init \
		-v `pwd`/dropbox:/dropbox \
		-v worktmp:/workdir agent-board-daemon

artifacts/oort-agent:
	$(MAKE) -C ../agent 
	cp ../agent/build/oort-server artifacts/oort-agent
	
files: ssh/ssh_host_rsa_key ssh/authorized_keys
	cp -r ../support/ ./
	cp ../SDK/*.zip ./

ssh/ssh_host_rsa_key ssh/authorized_keys:
	ssh-keygen -q -f ssh/ssh_host_rsa_key -N '' -t rsa
	ssh-add -L > ssh/authorized_keys
