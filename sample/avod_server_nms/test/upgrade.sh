#!/bin/bash

pkill -f node

if [ -f "/home/node.tar" ] ;
then

	if [ -f "/usr/local/bin/node" ] ;
	then
		rm /usr/local/bin/node -rf
	fi

	if [ -f "/usr/local/bin/npm" ] ;
	then
		rm /usr/local/bin/npm -rf
	fi

	chmod 775 /home/node.tar
	if [ -d "/home/node" ] ;
	then  
		rm /home/node -rf
	fi  

	cd /home

	tar xvf ./node.tar
	rm ./node.tar -rf
	chmod 775 /home/node -R

	ln -s /home/node/bin/node /usr/local/bin/node
	ln -s /home/node/bin/npm /usr/local/bin/npm

	cd -

fi



if [ -f "/home/client.tar" ] ;
then
	chmod 775 /home/client.tar
	if [ -d "/home/client" ] ;
	then  
		rm /home/client -rf
	fi  

	cd /home
	tar xvf ./client.tar
	rm ./client.tar -rf
	chmod 775 /home/client -R
	cd -

fi
