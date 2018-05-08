#ifndef __M_ICONV_H__
#define __M_ICONV_H__


int code_convert(char *from_charset, char *to_charset, char *indata, size_t insize, char *outdata, size_t outsize);

int u2g(char *indata, size_t insize, char *outdata, size_t outsize);

int g2u(char *indata, size_t insize, char *outdata, size_t outsize);


#endif /*__M_ICONV_H__*/
