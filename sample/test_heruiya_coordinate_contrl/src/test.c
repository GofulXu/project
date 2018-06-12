#include "default/gfapi.h"

int main(int argc, char *argv[])
{
	char buf[6] = {0x81, 0x07, 0x1, 0x07, 0x4, 0x81};

	unsigned short  x = 0, y = 0;
	unsigned short x_new = 0,y_new = 0;
#if 0
	unsigned short  x_coordinate = 0, y_coordinate = 0;
	unsigned short x_new_coordinate = 0,y_new_coordinate = 0;

	printf("buf: %02x---%02x---%02x---%02x---%02x---%02x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
	x_coordinate = buf[1] & 0xf;
	x_coordinate = (x_coordinate << 7) | (buf[2] & 0x7f);

	y_coordinate = buf[3] & 0xf;
	y_coordinate = (y_coordinate << 7) | (buf[4] & 0x7f);

	x_new_coordinate = (1950 - y_coordinate)*1366/1950;
	y_new_coordinate = x_coordinate*768/1980;
	printf("X,Y: %d %d, new X,Y: %d %d\n", x_coordinate, y_coordinate, x_new_coordinate, y_new_coordinate);

	buf[2] = x_coordinate << 8 >> 8 & 0xf;
	buf[1] = x_coordinate >> 7 & 0x7f;
	buf[4] = y_coordinate << 8 >> 8 & 0xf;
	buf[3] = y_coordinate >> 7 & 0x7f;

	printf("buf: %02x---%02x---%02x---%02x---%02x---%02x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);

#endif
//	y_new = y_new_coordinate;
//	x_new = x_new_coordinate;
	printf("input:x:y\n");
	scanf("%d:%d", &x_new, &y_new);
	x = y_new*1980/768;
	y = 1950 - x_new*1950/1366;
	printf("X,Y: %d %d, new X,Y: %d %d\n", x, y, x_new, y_new);

	buf[2] = x << 8 >> 8 & 0xf;
	buf[1] = x >> 7 & 0x7f;
	buf[4] = y << 8 >> 8 & 0xf;
	buf[3] = y >> 7 & 0x7f;

	printf("buf: %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
	return 0;
}
