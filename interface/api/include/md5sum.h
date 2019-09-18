/*************************************************************************
	> File Name: md5sum.h
	> Author: vinsion
	> Mail: vinsion@vinsion.mail 
	> Created Time: Fri 30 Aug 2019 09:55:24 AM +06
 ************************************************************************/

#ifndef __MD5SUM_H__
#define __MD5SUM_H__

int GetContextMD5Ex(char *Context, int Len, char *Buf, int Size);
int GetContextMD5(char *Context, int Len, unsigned char *Buf, int Size);
int GetFileMD5Ex(char *FileName, char *Buf, int Size);
int GetFileMD5(char *FileName, unsigned char *Buf, int Size);
int GetUuidEx(char *Buf, int Size);
int GetUuid(char *Buf, int Size);

#endif/*__MD5SUM_H__*/

