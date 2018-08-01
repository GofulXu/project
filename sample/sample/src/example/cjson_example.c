#include "default/gfapi.h"
#include "cJSON.h"

int main(int argc, char *argv[])
{
	cJSON *head = cJSON_CreateObject();
	cJSON *data1 = cJSON_CreateObject();
	cJSON *data2 = cJSON_CreateObject();
	cJSON *array = cJSON_CreateArray();
	cJSON_AddStringToObject(data1, "name", "goeful");
	cJSON_AddStringToObject(data2, "name", "jhon");
	cJSON_AddNumberToObject(data1, "age", 18);
	cJSON_AddNumberToObject(data2, "age", 20);
	cJSON_AddNullToObject(data1, "null");
	cJSON_AddNullToObject(data2, "null");
	cJSON_AddTrueToObject(data1, "true");
	cJSON_AddTrueToObject(data2, "true");
	cJSON_AddFalseToObject(data1, "false");
	cJSON_AddFalseToObject(data2, "false");
	cJSON_AddBoolToObject(data1, "bool", true);
	cJSON_AddBoolToObject(data2, "bool", false);
	cJSON_AddItemReferenceToArray(array, data1);		//不占用原有的数据
	cJSON_AddItemReferenceToArray(array, data2);
	cJSON_AddItemToObject(head, "array", array);
	cJSON_AddItemToObject(head, "data1", data1);
	cJSON_AddItemToObject(head, "data2", data2);

	printf("%s",cJSON_Print(head));
	printf("\n\n%s",cJSON_PrintUnformatted(head));

	cJSON_Delete(head);
	return 0;
}
