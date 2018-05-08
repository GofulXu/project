#ifndef __MBASE64_H__
#define __MBASE64_H__


int base64_encode(char *in_str, int in_len, char *out_str);


int base64_decode(char *in_str, int in_len, char *out_str);

void encode(FILE * fp_in, FILE * fp_out);

void decode(FILE * fp_in, FILE * fp_out);

#endif /*__MBASE64_H__*/
