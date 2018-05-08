#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "m_iconv.h"

static void printf02hex(char *warn_text, char *recv,int size)
{
   int i = 0;
   char tbuf[4];
   char log_buf[1024*4];
   memset(log_buf, 0, sizeof(log_buf));
   for(i = 0; i < size; i++)
   {
       memset(tbuf,0,sizeof(tbuf));
       sprintf(tbuf," %02x", recv[i]);
       strcat(log_buf, tbuf);
   }    
   printf("%s: %s\n", warn_text, log_buf);
} 

int main(void)
{
	char outdata[512] = {0};
	char outdata2[512] = {0};
	char indata[512] = {0};
	printf02hex("test", "\r\n", 2);
	printf("please chang the code to test input\n");
	scanf("%s", indata);
	printf02hex("indata", indata, 200);
	code_convert("utf-8", "unicode", indata, sizeof(indata), outdata, sizeof(outdata));
	printf02hex("outdata", outdata, 200);
	code_convert("unicode", "utf-8", outdata, sizeof(outdata), outdata2, sizeof(outdata2));
	printf02hex("outdata2", outdata2, 200);
	return 0;
}

