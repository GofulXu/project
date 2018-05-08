#ifndef SWMD5_H
#define SWMD5_H

#ifdef __alpha
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif

struct SWMD5Context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

/** 
 * @brief Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 * @param context 
 */
void SWMD5Init(struct SWMD5Context *context);
/** 
 * @brief Update context to reflect the concatenation of another buffer full
 * of bytes.
 * @param context 
 * @param buf 
 * @param len 
 */
void SWMD5Update(struct SWMD5Context *context, unsigned char const *buf,
	       unsigned len);
/** 
 * @brief Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 * @param digest 
 * @param context 
 */
void SWMD5Final(unsigned char digest[16], struct SWMD5Context *context);
/** 
 * @brief The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 * @param buf 
 * @param in 
 */
void SWMD5Transform(uint32 buf[4], uint32 const in[16]);

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */
typedef struct SWMD5Context SWMD5_CTX;

#endif /* !SWMD5_H */
