/*************************************************************************
	> File Name: Parse.c
	> Author: vinsion
	> Mail: vinsion@vinsion.mail 
	> Created Time: Wed 18 Sep 2019 07:57:56 AM +06
 ************************************************************************/

#include<stdio.h>
#include <stdio.h>
#include <string.h>

int Hex2Str(char *Hex, int SizeHex, char *Str, int SizeStr)
{
    static unsigned char ConText[] = "0123456789ABCDEF";
    int Size = 0, i = 0;
    if(SizeStr < SizeHex*2)
	Size = SizeStr;
    else
	Size = SizeHex*2;
    
    if(Size%2)
	Size--;
    //printf("SizeHex:%d\tSizeStr:%d\tSize:%d\n", SizeHex, SizeStr, Size);

    for(i = 0; i < Size; i+=2)
    {
	Str[i] = ConText[(Hex[i/2] >> 4) & 0x0f];
	Str[i+1] = ConText[Hex[i/2] & 0x0f];
    }
    return 0;
}

int Str2Hex(char *Str, int SizeStr, char *Hex, int SizeHex)
{
    int Size = 0, i = 0;
    if(SizeHex*2 < SizeStr)
	Size = SizeHex*2;
    else
	Size = SizeStr;
    
    if(Size%2)
	Size--;

    for(i = 0; i < Size; i++)
    {
	sscanf(Str + i*2, "%02x", Hex + i);
    }
     return 0;
}



