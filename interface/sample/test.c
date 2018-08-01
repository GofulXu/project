#include "gfapi.h"
#include "gfthrd.h"
#include "gfnetworkhandle.h"
#include "operate.h"

int main(int argc, char *argv[])
{
	gf_networkhandle_init();
	while('q' != getchar());
	gf_networkhandle_exit();
	return 0;
}
