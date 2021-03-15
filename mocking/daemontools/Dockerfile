FROM agent-base
        
COPY support/daemontools/ /service/

COPY ssh/ssh_host_rsa_key* /etc/ssh/

WORKDIR /root
COPY ssh/authorized_keys .ssh/
RUN chmod -R 700 .ssh/
RUN echo "#" > ~/.profile
RUN echo "ALL ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/all
RUN mkdir /var/run/sshd

COPY artifacts/oort-agent /oort-agent

COPY python-sdk-client.zip /tmp/
RUN pip install /tmp/python-sdk-client.zip
RUN rm /tmp/python-sdk-client.zip

COPY payload.py /payload.py

EXPOSE 2022
CMD ["svscan", "/service"]
