#!/bin/bash
while true
do
	#打印出当前的jboss进程：grep jboss查询的jboss进程，grep -v "grep" 去掉grep进程
	jmsThread=`ps -ef | grep gdms | grep iot | grep -v "grep"`
	echo $jmsThread

	#查询jboss进程个数：wc -l 返回行数
	count=`ps -ef | grep gdms | grep iot | grep -v "grep" | wc -l`
	echo $count

	if [ $count -gt 0 ]; then
	 #若进程还未关闭，则脚本sleep几秒
	 echo sleep $count
	 sleep 10
	else
	 #若进程已经关闭，则跳出循环
	 ./iot
	fi
done

#if [ $count -eq 0 ]; then
# echo "nohup startMethodServer.sh &"
# nohup startMethodServer.sh &
#else
# echo "It's better to check the thread!!!"
#fi
