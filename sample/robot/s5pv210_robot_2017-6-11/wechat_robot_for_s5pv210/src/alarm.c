#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handler_clied()
{
	printf("Hello\n");
	system("aplay ./1.wav");
	printf("end\n");
	signal(SIGALRM,handler_clied);//让内核做好准备，一旦接受到SIGALARM信号,就执行 handler
	alarm(5);
} 

void handler() {
	
signal(SIGALRM,handler_clied);//让内核做好准备，一旦接受到SIGALARM信号,就执行 handler
alarm(5);
}
/*
main()
{
int i;
handler();
for(i=1;i<21;i++){
printf("sleep %d ...\n",i);
sleep(1);
}
} */
