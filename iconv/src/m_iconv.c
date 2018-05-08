#include <stdio.h>
#include <iconv.h>
#include <string.h>
#include "m_iconv.h"

//代码转换:从一种编码转为另一种编码
int code_convert(char *from_charset, char *to_charset, char *indata, size_t insize, char *outdata, size_t outsize)
{
	size_t convsize = 0;
	iconv_t cd = iconv_open(to_charset, from_charset);
	if(cd < 0)
		return -1;

	memset(outdata, 0, outsize);
	convsize = iconv(cd,&indata,&insize,&outdata,&outsize);
	iconv_close(cd);
	return convsize;
}

int u2g(char *indata, size_t insize, char *outdata, size_t outsize)
{
	return code_convert("utf-8", "gb2312", indata, insize, outdata, outsize);
}

int g2u(char *indata, size_t insize, char *outdata, size_t outsize)
{
	return code_convert("gb2312", "utf-8", indata, insize, outdata, outsize);
}
