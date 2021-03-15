FROM agent-base
        
COPY support/monit/monit.agent /etc/monit/conf.d/
COPY support/init.d/oort-agent /etc/init.d/

COPY ssh/ssh_host_rsa_key* /etc/ssh/

RUN echo 'set httpd port 2812 and allow localhost' >> /etc/monit/monitrc
RUN echo 'check process sshd with pidfile /var/run/sshd.pid start = "/usr/sbin/sshd -p 2022"'  >> /etc/monit/monitrc

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
COPY support/monit/monit.payload /etc/monit/conf.d

EXPOSE 2022
CMD ["monit", "-d", "15", "-I"]
