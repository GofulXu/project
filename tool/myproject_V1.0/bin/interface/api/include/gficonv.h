#ifndef __GFICONV_H__
#define __GFICONV_H__

#ifdef __cplusplus
extern "C"{
#endif

int code_convert(char *from_charset, char *to_charset, char *indata, size_t insize, char *outdata, size_t outsize);

int u2g(char *indata, size_t insize, char *outdata, size_t outsize);

int g2u(char *indata, size_t insize, char *outdata, size_t outsize);

#ifdef __cplusplus
}
#endif

#endif /*__GFICONV_H__*/
