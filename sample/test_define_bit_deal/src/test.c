#include "default/gfapi.h"



typedef enum _SEND_DATA_OFFSET{
	OFFSET_MARK_BIT = 0,
	OFFSET_PLAY_BIT = 8,
	OFFSET_NEXT_BIT = 16,
	OFFSET_PREV_BIT = 24,
#if 1
	OFFSET_VOLUME_BIT = 32,
	OFFSET_MUTE_BIT = 40,
	OFFSET_AREA_BIT = 48,
	OFFSET_OPE_BIT = 56,
#endif

}SEND_DATA_OFFSET;//485Databit_offset;

#define	 Refresh_ComDataBit_(x, y, z)	{(x) &= ~(0xff << (z));if(y){((x)|=((y) << (z)));}}
#define	 Get_ComBit_(x, y, z)		(y)=((x) << (24-(z)) >> (24))

static unsigned long send_data = 0;

void Refresh_ComDataBit( unsigned long *d_data, unsigned char d_bit, SEND_DATA_OFFSET d_offset)
{
	unsigned int senddata0 = *d_data << 32 >> 32;
	unsigned int senddata1 = *d_data >> 32;
	if(d_offset >= 0 && d_offset < 32)
		Refresh_ComDataBit_(senddata0, d_bit, d_offset)
	else if(d_offset >= 32 && d_offset < 64)
		Refresh_ComDataBit_(senddata1, d_bit, d_offset-32)
	*d_data = 0;
	*d_data |= senddata1;
	*d_data = *d_data << 32;
	*d_data |= senddata0;

	return;
}

unsigned char Get_ComBit( unsigned long *d_data, SEND_DATA_OFFSET d_offset)
{
	unsigned int senddata0 = *d_data << 32 >> 32;
	unsigned int senddata1 = *d_data >> 32;
	unsigned char databit = 0;
	if(d_offset >= 0 && d_offset < 32)
		Get_ComBit_(senddata0, databit, d_offset);
	else if(d_offset >= 32 && d_offset < 64)
		Get_ComBit_(senddata1, databit, d_offset-32);
	return databit;

}


int main(int argc, char *argv[])
{
	Refresh_ComDataBit(&send_data, 2, OFFSET_MARK_BIT);
	Refresh_ComDataBit(&send_data, 1, OFFSET_PLAY_BIT);
	Refresh_ComDataBit(&send_data, 0, OFFSET_NEXT_BIT);
	Refresh_ComDataBit(&send_data, 1, OFFSET_PREV_BIT);

	Refresh_ComDataBit(&send_data, 0, OFFSET_VOLUME_BIT);
	Refresh_ComDataBit(&send_data, 1, OFFSET_MUTE_BIT);
	Refresh_ComDataBit(&send_data, 0, OFFSET_AREA_BIT);
	Refresh_ComDataBit(&send_data, 1, OFFSET_OPE_BIT);
	
	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_MARK_BIT));
	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_PLAY_BIT));
	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_NEXT_BIT));
	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_PREV_BIT));

	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_VOLUME_BIT));
	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_MUTE_BIT));
	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_AREA_BIT));
	printf("\ncombit:%02x\n", Get_ComBit(&send_data, OFFSET_OPE_BIT));

	char data[8] = {0};
	memcpy(data, &send_data, 8);
	printf("\n\ndata:");
	for(int i = 0; i < 8; i++)
		printf(" %02x", data[i]);
	printf("\n\n");
	return 0;
}
