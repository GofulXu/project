#!/bin/bash

i=1
while [ i=1 ]
do
cd /home/client
ps >> /home/client/process
var1= grep node /home/client/process >> /home/client/process1
#echo $var1
#declare -i a=$?
if [ $? -eq 0 ];
then 
#echo "node main.js running"
cd /
else
pkill -f node
#echo "node main.js"
node main.js&
fi
rm /home/client/process
rm /home/client/process1

if [ -f "/root/upgrade.sh" ] ;
then
chmod 775 /root/upgrade.sh
/root/upgrade.sh
rm /root/upgrade.sh -rf
fi

sleep 5
done
