#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "uart.h"



int main(int argc, char *argv[])
{
	if(start_uart(NULL,9600,false) < 0)
		return -1;
	getchar();
	close_uart();
	return 0;
}

