#!/bin/bash
if [  -n "$(uname -a | grep Ubuntu)" ]
then
    #echo "ubuntu linux"
    cd /usr/src/linux-headers-$(uname -r)/certs && sudo openssl req -new -x509 -newkey rsa:2048 -keyout signing_key.pem -outform DER -out signing_key.x509 -nodes -subj "/CN=Owner/"
else
    #echo "not ubuntu linux - (RHEL/Rocky/CentOS)" 
    cd /lib/modules/$(uname -r)/build/certs && sudo openssl req -new -x509 -newkey rsa:2048 -keyout signing_key.pem -outform DER -out signing_key.x509 -nodes -subj "/CN=Owner/"
fi
cd - && sudo make install
