#include "gfapi.h"
#include "sqlite3/operate.h"
#include "gfparamerter.h"

#define VALUE_SIZE	256
static sqlite3 *db = NULL;

int main(int argc, char *argv[])
{

	if(argc <= 1)
		return 0;
	if(strstr(argv[1], "help"))
	{
		printf("\t\t\t + db file name to show db's paramerter\n");
		return 0;
	}

	db = database_open(argv[1]);
	printf("\n\tsystem\t\n");
	database_show(db, SYSTEM_PARA_TABLE);
	printf("\n\tgeneral\t\n");
	database_show(db, GENERAL_PARA_TABLE);
	database_close(db);

	return 0;
}
