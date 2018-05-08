/** 
 * @file cwmp_authentication.c
 * @brief Http authentication
 * @author niujiuru
 * @date 2007-8-16
 */

// 详细请参考《RFC2617》协议标准

#include "http_authentication.h"
#include "string.h"
#include "md5.h"

void CvtHex(IN HASH Bin, OUT HASHHEX Hex)
{
    unsigned short i;
    unsigned char j;
	
    for (i = 0; i < HASHLEN; i++) {
        j = (Bin[i] >> 4) & 0xf;
        if (j <= 9)
            Hex[i*2] = (j + '0');
		else
            Hex[i*2] = (j + 'a' - 10);
        j = Bin[i] & 0xf;
        if (j <= 9)
            Hex[i*2+1] = (j + '0');
		else
            Hex[i*2+1] = (j + 'a' - 10);
    };
    Hex[HASHHEXLEN] = '\0';
}

/* calculate H(A1) as per spec */
void DigestCalcHA1(
		IN char * pszAlg,	/* 算法名称 md5||md5-sess */
		IN char * pszUserName,	/* login user name */
		IN char * pszRealm,	/* realm name */
		IN char * pszPassword,	/* login password */
		IN char * pszNonce,	/* 服务器随机产生的nonce返回串 */
		IN char * pszCNonce,	/* 客户端随机产生的nonce串 */
		OUT HASHHEX SessionKey  /* H(A1) */
		)
{
	struct SWMD5Context Md5Ctx;
	HASH HA1;
	
	SWMD5Init(&Md5Ctx);
	SWMD5Update(&Md5Ctx, pszUserName, strlen(pszUserName));
	SWMD5Update(&Md5Ctx, ":", 1);
	SWMD5Update(&Md5Ctx, pszRealm, strlen(pszRealm));
	SWMD5Update(&Md5Ctx, ":", 1);
	SWMD5Update(&Md5Ctx, pszPassword, strlen(pszPassword));
	SWMD5Final(HA1, &Md5Ctx);
#ifdef WIN32
    if (stricmp(pszAlg, "md5-sess") == 0) {
#else
	if (strcasecmp(pszAlg, "md5-sess") == 0) {
#endif
		SWMD5Init(&Md5Ctx);
		SWMD5Update(&Md5Ctx, HA1, HASHLEN);
		SWMD5Update(&Md5Ctx, ":", 1);
		SWMD5Update(&Md5Ctx, pszNonce, strlen(pszNonce));
		SWMD5Update(&Md5Ctx, ":", 1);
		SWMD5Update(&Md5Ctx, pszCNonce, strlen(pszCNonce));
		SWMD5Final(HA1, &Md5Ctx);
	};
	CvtHex(HA1, SessionKey);
}

/* calculate request-digest/response-digest as per HTTP Digest spec */
void DigestCalcResponse(
			IN HASHHEX HA1,           /* H(A1) */
			IN char * pszNonce,       /* nonce from server */
			IN char * pszNonceCount,  /* 8 hex digits */
			IN char * pszCNonce,      /* client nonce */
			IN char * pszQop,         /* qop-value: "", "auth", "auth-int" */
			IN char * pszMethod,      /* method from the request */
			IN char * pszDigestUri,   /* requested URL */
			IN HASHHEX HEntity,       /* H(entity body) if qop="auth-int" */
			OUT HASHHEX Response      /* request-digest or response-digest */
			)
{
	struct SWMD5Context Md5Ctx;
	HASH HA2;
	HASH RespHash;
	HASHHEX HA2Hex;
	
	// calculate H(A2)
	SWMD5Init(&Md5Ctx);
	SWMD5Update(&Md5Ctx, pszMethod, strlen(pszMethod));
	SWMD5Update(&Md5Ctx, ":", 1);
	SWMD5Update(&Md5Ctx, pszDigestUri, strlen(pszDigestUri));
#ifdef WIN32
	if (stricmp(pszQop, "auth-int") == 0) {
#else
	if (strcasecmp(pszQop, "auth-int") == 0) {
#endif
		SWMD5Update(&Md5Ctx, ":", 1);
		SWMD5Update(&Md5Ctx, HEntity, HASHHEXLEN);
	};
	SWMD5Final(HA2, &Md5Ctx);
	CvtHex(HA2, HA2Hex);
	
	// calculate response
	SWMD5Init(&Md5Ctx);
	SWMD5Update(&Md5Ctx, HA1, HASHHEXLEN);
	SWMD5Update(&Md5Ctx, ":", 1);
	SWMD5Update(&Md5Ctx, pszNonce, strlen(pszNonce));
	SWMD5Update(&Md5Ctx, ":", 1);
    if (*pszQop) {
		SWMD5Update(&Md5Ctx, pszNonceCount, strlen(pszNonceCount));
		SWMD5Update(&Md5Ctx, ":", 1);
		SWMD5Update(&Md5Ctx, pszCNonce, strlen(pszCNonce));
		SWMD5Update(&Md5Ctx, ":", 1);
		SWMD5Update(&Md5Ctx, pszQop, strlen(pszQop));
		SWMD5Update(&Md5Ctx, ":", 1);
	};
	SWMD5Update(&Md5Ctx, HA2Hex, HASHHEXLEN);
	SWMD5Final(RespHash, &Md5Ctx);
	CvtHex(RespHash, Response);
};
