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
		printf("\t\t\t + db + table name to show db's paramerter\n");
		return 0;
	}

	db = database_open(argv[1]);
	printf("\n\t%s\t\n", argv[2]);
	database_show(db, argv[2]);
	database_close(db);

	return 0;
}
