/** 
 * @file cwmp_authentication.c
 * @brief Http authentication
 * @author niujiuru
 * @date 2007-8-16
 */

// 详细请参考《RFC2617》协议标准

#ifndef __HTTPAUTHENTICATIONRFC2617_H__
#define __HTTPAUTHENTICATIONRFC2617_H__

#define HASHLEN 16
typedef char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN+1];
#define IN
#define OUT

#ifdef __cplusplus
extern "C"
{
#endif
/* calculate H(A1) as per HTTP Digest spec */
void DigestCalcHA1(
		IN char * pszAlg,	/* 算法名称 md5||md5-sess */
		IN char * pszUserName,	/* login user name */
		IN char * pszRealm,	/* realm name */
		IN char * pszPassword,	/* login password */
		IN char * pszNonce,	/* 服务器随机产生的nonce返回串 */
		IN char * pszCNonce,	/* 客户端随机产生的nonce串 */
		OUT HASHHEX SessionKey  /* H(A1) */
		);

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
			);
#ifdef __cplusplus
}
#endif

#endif
