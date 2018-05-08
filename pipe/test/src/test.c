#include <stdio.h>

int main(void)
{
	printf("\tThis application make in %s_%s,file name is %s.\n \
	Print from the function %s,the line %d.\n", __DATE__,   \
	 __TIME__, __FILE__, __FUNCTION__, __LINE__);
	return 0;
}
