#include "sound.h"
#include "network.h"

/*****************************
**��½�û����ϴ��û��ʱ�
**const char* login_params����½����
*************************/
int sound_init(const char* login_params)
{
	int ret = MSPLogin(NULL, NULL, login_params);		//用户登录
	if (MSP_SUCCESS != ret)	{
		printf("MSPLogin failed , Error code %d.\n",ret);
		return ret; // login fail, exit the program
	}

	//printf("Want to upload the user words ? \n0: No.\n1: Yes\n");
	
	return MSP_SUCCESS;
}


/*****************************
*******openQA-datetime-calc-baike-faq-chat json解析
*************************/
void doit_ODCBFC(cJSON *json, char *text,const short doit_cond)
{
	
	printf("server:%s\n",cJSON_GetObjectItem(json, "service")->valuestring);
	printf("text:%s\n",cJSON_GetObjectItem(cJSON_GetObjectItem(json, "answer"), "text")->valuestring);
	
	if(doit_cond == DOIT_TESTBAKE)		//baike场景
	{
		if(strcmp(cJSON_GetObjectItem(json, "service")->valuestring,"baike") == 0)
		{
			memset(text, 0, 4096);	
			strcpy(text, cJSON_GetObjectItem(cJSON_GetObjectItem(json, "answer"), "text")->valuestring);
		}else if(NULL != cJSON_GetObjectItem(json, "moreResults"))
		{
			cJSON *pSubdata = cJSON_GetObjectItem(json, "moreResults");//取数组  
			//int arrySize=cJSON_GetArraySize(pSubdata);//数组大小  
			cJSON *tasklist=pSubdata->child;//子对象
			while(tasklist!=NULL)  
			{
				if(strcmp(cJSON_GetObjectItem(tasklist, "service")->valuestring, "baike") == 0)
				{
					memset(text, 0, 4096);	
					strcpy(text, cJSON_GetObjectItem(cJSON_GetObjectItem(tasklist, "answer"), "text")->valuestring);
				}
				tasklist=tasklist->next;  
			}
			
		}else{
			memset(text, 0, 4096);	
			strcpy(text, "小S找不到相关内容，老板可以换一种说法试试");
			
		}
		return;
		
	}else if(doit_cond == DOIT_TESTCHAT){
		if(strcmp(cJSON_GetObjectItem(json, "service")->valuestring,"baike"))
		{
			memset(text, 0, 4096);	
			strcpy(text, cJSON_GetObjectItem(cJSON_GetObjectItem(json, "answer"), "text")->valuestring);
			return;
		}else if(NULL != cJSON_GetObjectItem(json, "moreResults"))
		{
			cJSON *pSubdata = cJSON_GetObjectItem(json, "moreResults");//取数组  
			//int arrySize=cJSON_GetArraySize(pSubdata);//数组大小  
			cJSON *tasklist=pSubdata->child;//子对象
			while(tasklist!=NULL)  
			{
				
				memset(text, 0, 4096);	
				strcpy(text, cJSON_GetObjectItem(cJSON_GetObjectItem(tasklist, "answer"), "text")->valuestring);
				return;  
			}
			
		}else{
			memset(text, 0, 4096);	
			strcpy(text, "小S找不到相关内容，老板可以换一种说法试试");
			return;	
		}
		
	}else{
		memset(text, 0, 4096);	
		printf("server:%s\n",cJSON_GetObjectItem(json, "service")->valuestring);
		printf("text:%s\n",cJSON_GetObjectItem(cJSON_GetObjectItem(json, "answer"), "text")->valuestring);
		strcpy(text, cJSON_GetObjectItem(cJSON_GetObjectItem(json, "answer"), "text")->valuestring);
		if(NULL != cJSON_GetObjectItem(json, "moreResults"))
		{
			cJSON *pSubdata = cJSON_GetObjectItem(json, "moreResults");//取数组  
			//int arrySize=cJSON_GetArraySize(pSubdata);//数组大小  
			cJSON *tasklist=pSubdata->child;//子对象
			while(tasklist!=NULL)  
			{
				printf("server1:%s\n",cJSON_GetObjectItem(tasklist, "service")->valuestring);
				printf("text1:%s\n",cJSON_GetObjectItem(cJSON_GetObjectItem(tasklist, "answer"), "text")->valuestring);
				strcat(text, cJSON_GetObjectItem(cJSON_GetObjectItem(tasklist, "answer"), "text")->valuestring);
				tasklist=tasklist->next;  
			}
			
		}
		return ;

	}
	
	
}

/*****************************
*******weather json解析
*************************/
void doit_weather(cJSON *json, char *text)
{
	cJSON *pSubdata = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "data"),"result");//取数组  
	//int arrySize=cJSON_GetArraySize(pSubdata);//数组大小  
	cJSON *tasklist=pSubdata->child;//子对象
	int a = 1;
	char mic_msg[1024] = {0};
	char sourceName[32] = {0};
	memset(text, 0, 4096);
	while(tasklist!=NULL)  
	{
		/*
		printf("\n\n\n\n");
		printf("date:%s\n",cJSON_GetObjectItem(tasklist,"date")->valuestring);  						//预报时间
		printf("lastUpdateTime:%s\n",cJSON_GetObjectItem(tasklist,"lastUpdateTime")->valuestring); 		//最后更新时间 
		printf("city:%s\n",cJSON_GetObjectItem(tasklist,"city")->valuestring);  						//城市名称
		printf("weather:%s\n",cJSON_GetObjectItem(tasklist,"weather")->valuestring);  					//天气现象
		printf("windLevel:%d\n",cJSON_GetObjectItem(tasklist,"windLevel")->valueint); 				//风级
		printf("tempRange:%s\n",cJSON_GetObjectItem(tasklist,"tempRange")->valuestring);  				//气温范围
		printf("wind:%s\n",cJSON_GetObjectItem(tasklist,"wind")->valuestring);  						//风向以及风力
		printf("sourceName:%s\n",cJSON_GetObjectItem(tasklist,"sourceName")->valuestring);  			//来自
		*/
		memset(mic_msg, 0, 1024);
		memset(sourceName, 0, 32);
		strcpy(sourceName, cJSON_GetObjectItem(tasklist,"sourceName")->valuestring); 				//获取消息来源	
		strcat(mic_msg,cJSON_GetObjectItem(tasklist,"date")->valuestring);
		strcat(mic_msg,cJSON_GetObjectItem(tasklist,"city")->valuestring);
		strcat(mic_msg,cJSON_GetObjectItem(tasklist,"weather")->valuestring);
		sprintf(mic_msg,"%s，风级:%d", mic_msg, cJSON_GetObjectItem(tasklist,"windLevel")->valueint);
		strcat(mic_msg,"，气温范围:");
		strcat(mic_msg,cJSON_GetObjectItem(tasklist,"tempRange")->valuestring);
		strcat(mic_msg,"，");
		strcat(mic_msg,cJSON_GetObjectItem(tasklist,"wind")->valuestring);
		
		
		if(a)
		{
			/*
			printf("airQuality:%s\n",cJSON_GetObjectItem(tasklist,"airQuality")->valuestring); 			//空气质量
			printf("humidity:%s\n",cJSON_GetObjectItem(tasklist,"humidity")->valuestring); 				//湿度
			printf("pm25:%s\n",cJSON_GetObjectItem(tasklist,"pm25")->valuestring); 						//PM25值
			*/
			a = 0;
			strcat(mic_msg,"，空气质量:");
			strcat(mic_msg,cJSON_GetObjectItem(tasklist,"airQuality")->valuestring);
			strcat(mic_msg,"，相对湿度:");
			strcat(mic_msg,cJSON_GetObjectItem(tasklist,"humidity")->valuestring);
			sprintf(mic_msg,"%s，PM25值为:%s", mic_msg, cJSON_GetObjectItem(tasklist,"pm25")->valuestring);
			
			if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "datetime"), "dateOrig"))
			{
				strcat(text, cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "datetime"), "dateOrig")->valuestring);
				
				strcat(text,(mic_msg+10));
				strcat(text,"，本消息来自");
				strcat(text,sourceName);
				return ;
			}	
		}			
		
		strcat(text,mic_msg);
		strcat(text,"； ");
		//printf("\n%s\n", mic_msg);					
		tasklist=tasklist->next;  
		
		if(tasklist==NULL)
		{
			strcat(text,"，本消息来自");
			strcat(text,sourceName);
		}
	}  
	return ;
}

/*****************************
*******pm25 json解析
*************************/
void doit_pm25(cJSON *json, char *text)
{
	cJSON *pSubdata = cJSON_GetObjectItem(cJSON_GetObjectItem(json, "data"),"result");//取数组  
	//int arrySize=cJSON_GetArraySize(pSubdata);//数组大小  
	cJSON *tasklist=pSubdata->child;//子对象
	char mic_msg[1024] = {0};
	char sourceName[64] = {0};
	memset(text, 0, 4096);
	while(tasklist!=NULL)  
	{
		/*
		printf("\n");
		printf("publishDateTime:%s\n",cJSON_GetObjectItem(tasklist,"publishDateTime")->valuestring);  	//发布日期
		printf("area:%s\n",cJSON_GetObjectItem(tasklist,"area")->valuestring); 							//地区 
		printf("subArea:%s\n",cJSON_GetObjectItem(tasklist,"subArea")->valuestring);  						//区域
		printf("aqi:%d\n",cJSON_GetObjectItem(tasklist,"aqi")->valueint);  					//空气质量指数(AQI)
		printf("quality:%s\n",cJSON_GetObjectItem(tasklist,"quality")->valuestring); 				//空气质量描述
		printf("pm25:%d\n",cJSON_GetObjectItem(tasklist,"pm25")->valueint);  				//颗粒物(粒径小于等于2.5)
		printf("positionName:%s\n",cJSON_GetObjectItem(tasklist,"positionName")->valuestring);  						//监测点名称
		*/
		memset(mic_msg, 0, 1024);
		memset(sourceName, 0, 64);
		strcpy(sourceName, cJSON_GetObjectItem(tasklist,"sourceName")->valuestring); 				//获取消息来源
		strcat(sourceName,cJSON_GetObjectItem(tasklist,"publishDateTime")->valuestring);
		
		strcat(mic_msg,cJSON_GetObjectItem(tasklist,"area")->valuestring);
		strcat(mic_msg,cJSON_GetObjectItem(tasklist,"subArea")->valuestring);
		sprintf(mic_msg,"%s，空气质量指数：%d，%s", mic_msg, cJSON_GetObjectItem(tasklist,"aqi")->valueint, cJSON_GetObjectItem(tasklist,"quality")->valuestring);
		sprintf(mic_msg,"%s，PM2.5值为:%d，监控点：%s", mic_msg, cJSON_GetObjectItem(tasklist,"pm25")->valueint, cJSON_GetObjectItem(tasklist,"positionName")->valuestring);
		
		strcat(text,mic_msg);
		strcat(text,"；  ");
		//printf("\n%s\n", mic_msg);					
		tasklist=tasklist->next; 
		if(tasklist==NULL)
		{
			strcat(text,"，本消息来自");
			strcat(text,sourceName);
		}
	}  
	return ;
}


/*****************************
*******fan json解析
*************************/
int doit_fan_smartHome(cJSON *json, char *text)
{
	memset(text, 0, 4096);
	if(!strcmp(cJSON_GetObjectItem(json, "operation")->valuestring, "SET")){		//确定操作类型为设置
		if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "开")){	//开操作
		
			if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "开关")){	//开关电源操作
				if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location")){	//具体内容
				
				printf("%s风扇已打开\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				sprintf(text, "%s风扇已打开\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				//具体开关电源操作	cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring
					if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "厨房") == 0){
						send_to_ROBOT("Zib:S:w0002");
					}else if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "卧室") == 0)
						send_to_ROBOT("Zib:S:w0002");
					return 0;//设置成功
				}else{
					//默认开关操作
					send_to_ROBOT("Zib:S:w0001");
					printf("默认风扇已打开\n");
					sprintf(text, "默认风扇已打开\n");
					return 0;//设置成功
				}
					
				
			}else{
				printf("%s功能已开启\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				sprintf(text, "%s功能已开启\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				//更改具体开关设置	attr
				
				return 0;//设置成功
			}
		}
		if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "关")){	//关操作
		
			if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "开关")){	//开关电源操作
			
				if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location")){	//具体内容
				
				printf("%s风扇已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				sprintf(text, "%s风扇已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				//具体开关电源操作	cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring
					if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "厨房") == 0){
						send_to_ROBOT("Zib:S:w0000");
					}else if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "卧室") == 0)
						send_to_ROBOT("Zib:S:w0000");
					return 0;//设置成功
				}else{
					//默认开关操作
					send_to_ROBOT("Zib:S:w0000");
					printf("默认风扇已关闭\n");
					sprintf(text, "默认风扇已关闭\n");
					return 0;//设置成功
				}
				
				
			}else{
				printf("%s功能已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				sprintf(text, "%s功能已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				//更改具体开关设置	attr
				
				
				return 0;//设置成功
			}
		
		}
		
	}
	strcpy(text, cJSON_GetObjectItem(json, "text")->valuestring);
	return -1;//设置失败
}


/*****************************
*******switch json解析
*************************/
int doit_switch_smartHome(cJSON *json, char *text)
{
	memset(text, 0, 4096);
	
	if(!strcmp(cJSON_GetObjectItem(json, "operation")->valuestring, "SET")){		//确定操作类型为设置
		if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "开")){	//开操作
		
			if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "开关")){	//开关电源操作
				if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location")){	//具体内容
				
				printf("%s开关已打开\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				sprintf(text,"%s开关已打开\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				//具体开关电源操作	cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring
				
				return 0;//设置成功
				}else{
					//默认开关操作
					printf("默认开关已打开\n");
					sprintf(text,"默认开关已打开\n");
					return 0;//设置成功
				}
					
				
			}else{
				printf("%s功能已开启\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				sprintf(text, "%s功能已开启\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				//更改具体开关设置	attr
				if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "模式1") == 0){
						send_to_ROBOT("Zib:S:w0012");
						
					}
				return 0;//设置成功
			}
		}
		if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "关")){	//关操作
		
			if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "开关")){	//开关电源操作
			
				if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location")){	//具体内容
				
				printf("%s开关已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				sprintf(text, "%s开关已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				//具体开关电源操作	cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring
				
				return 0;//设置成功
				}else{
					//默认开关操作
					printf("默认开关已关闭\n");
					sprintf(text, "默认开关已关闭\n");
					return 0;//设置成功
				}
				
				
			}else{
				printf("%s功能已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				sprintf(text, "%s功能已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				//更改具体开关设置	attr
				
				return 0;//设置成功
			}
		
		}
		
	}	
	strcpy(text, cJSON_GetObjectItem(json, "text")->valuestring);
	return -1;//设置失败
}


/*****************************
*******light json解析
*************************/
int doit_light(cJSON *json, char *text)
{
	memset(text, 0, 4096);
	
	if(!strcmp(cJSON_GetObjectItem(json, "operation")->valuestring, "SET")){		//确定操作类型为设置
		if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "开")){	//开操作
		
			if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "开关")){	//开关电源操作
				if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location")){	//具体内容
				printf("%s灯已打开\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				sprintf(text,"%s灯已打开\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				//具体开关电源操作	cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring
					if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "厨房") == 0){
							send_to_ROBOT("Zib:S:w0004");
						}else if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "卧室") == 0)
							send_to_ROBOT("Zib:S:w0008");
					return 0;//设置成功
				}else{
					//默认开关操作
					send_to_ROBOT("Zib:S:w0004");
					printf("默认灯已打开\n");
					sprintf(text,"默认灯已打开\n");
					return 0;//设置成功
				}
					
				
			}else{
				printf("%s功能已开启\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				sprintf(text,"%s功能已开启\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				//更改具体开关设置	attr
				
				return 0;//设置成功
			}
		}
		if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "关")){	//关操作
		
			if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "开关")){	//开关电源操作
				
				if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location")){	//具体内容
				
				printf("%s灯已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				sprintf(text,"%s灯已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring);
				//具体开关电源操作	cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring
					if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "厨房") == 0){
						send_to_ROBOT("Zib:S:w0000");
					}else if(strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "location"), "room")->valuestring, "卧室") == 0)
						send_to_ROBOT("Zib:S:w0000");
				return 0;//设置成功
				}else{
					//默认开关操作
					send_to_ROBOT("Zib:S:w0000");
					printf("默认灯已关闭\n");
					sprintf(text,"默认灯已关闭\n");
					return 0;//设置成功
				}
				
				
			}else{
				printf("%s功能已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				sprintf(text,"%s功能已关闭\n", cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring);
				//更改具体开关设置	attr
				
				return 0;//设置成功
			}
		
		}
		
	}	
	strcpy(text, cJSON_GetObjectItem(json, "text")->valuestring);
	return -1;//设置失败
}


/*****************************
*******musicPlayer_smartHome json解析
*************************/
int doit_musicPlayer_smartHome(cJSON *json, char *text)
{
	memset(text, 0, 4096);
	if(!strcmp(cJSON_GetObjectItem(json, "operation")->valuestring, "SET")){		//确定操作类型为设置
			if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "开关")){	//开关操作
				//具体操作
				if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "播放")){	//播放操作
					//播放操作
					send_to_ROBOT("Mus:play");
					printf("播放\n");
					sprintf(text, "播放\n");
					return 0;//设置成功
				}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "暂停")){	
					//暂停操作
					send_to_ROBOT("Mus:pause");
					printf("暂停\n");
					sprintf(text, "暂停\n");
					return 0;//设置成功
				}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "停止")){	
					//停止操作
					send_to_ROBOT("Mus:stop");
					printf("停止\n");
					sprintf(text, "停止\n");
					return 0;//设置成功
				}
				
			}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "歌曲顺序")){	//歌曲顺序操作
				//具体操作
				if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "上一首")){	//开关操作
					//上一首操作
					send_to_ROBOT("Mus:last");
					printf("上一首\n");
					sprintf(text, "上一首\n");
					return 0;//设置成功
				
				}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "下一首")){
					//下一首操作
					send_to_ROBOT("Mus:next");
					printf("下一首\n");
					sprintf(text, "下一首\n");
					return 0;//设置成功
					
				}
			}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "音量")){	//音量操作
				//具体操作
				if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue"), "direct")->valuestring, "+")){	
				
					if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue"), "offset")){
						//具体加多少音量offset
						send_to_ROBOT("Mus:sub+");
						printf("音量+n\n");
						sprintf(text, "音量+%d\n",cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue"), "offset")->valueint);
						return 0;//设置成功
						
					}else{
						//加音量
						send_to_ROBOT("Mus:sub+");
						printf("音量+\n");
						sprintf(text, "音量+\n");
						return 0;//设置成功
						
					}
					
				
				}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue"), "direct")->valuestring, "-")){	
				
					if(NULL != cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue"), "offset")){
						//具体减多少音量offset
						
						send_to_ROBOT("Mus:sub-");
						printf("音量-n\n");
						sprintf(text, "音量-%d\n",cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue"), "offset")->valueint);
						return 0;//设置成功
						
					}else{
						//减音量
						send_to_ROBOT("Mus:sub-");
						printf("音量-\n");
						sprintf(text, "音量-\n");
						return 0;//设置成功
							
					}
				}
			}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "爱情")){	//爱情类型切换操作
				//具体操作
				if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "设置")){	
					//
					send_to_ROBOT("Mus:suba");
					printf("爱情\n");
					sprintf(text, "正在更改设置为爱情模式\n");
					return 0;//设置成功
				}
			}else if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attr")->valuestring, "随机播放")){	//随机播放切换操作
				//具体操作
				if(!strcmp(cJSON_GetObjectItem(cJSON_GetObjectItem(cJSON_GetObjectItem(json, "semantic"), "slots"), "attrValue")->valuestring, "设置")){	
					//
					send_to_ROBOT("Mus:subb");
					printf("随机播放\n");
					sprintf(text, "正在更改设置为随机播放模式\n");
					return 0;//设置成功
				}
			}
	}	
	strcpy(text, cJSON_GetObjectItem(json, "text")->valuestring);
	return -1;//设置失败
}


/***********解析json信息，返回对应回答语句****************/
int doit(char *text,const short doit_cond)
{
	cJSON *json;
	int ii = 0;
	json=cJSON_Parse(text);
	if (!json){
		printf("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	}else
	{			
		if(NULL != cJSON_GetObjectItem(json, "service"))
		{
			if(doit_cond == DOIT_VOICE)		//语音识别
			{
				if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "fan_smartHome")){		//风扇json信息
					printf("\n fan_smartHome \n\n");
					doit_fan_smartHome(json, text);
				}else if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "switch_smartHome")){		//开关json信息	
					printf("\n switch_smartHome \n\n");
					doit_switch_smartHome(json, text);
				}else if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "light_smartHome")){		//灯json信息	
					printf("\n light \n\n");
					doit_light(json, text);
				}else if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "musicPlayer_smartHome")){		//音乐json信息	
					printf("\n musicPlayer_smartHome \n\n");
					doit_musicPlayer_smartHome(json, text);
				}else if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "weather")){		//天气json信息
					printf("\n weather4444 \n\n");
					doit_weather(json, text);
				}else if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "pm25")){		//pm25 json信息	
					printf("\n pm25 \n\n");
					doit_pm25(json, text);
				}else if((!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "openQA")) || 				\
						(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "datetime"))  || 				\
						(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "calc"))  || 				\
						(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "baike"))  || 				\
						(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "faq"))  || 				\
						(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "chat")) ){		//pm25 json信息	
					printf("\n openQA \n\n");
					doit_ODCBFC(json, text, doit_cond);
				}
			}else{																			//文字交流
			
					if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "weather")){		//天气json信息
						printf("\n weather \n\n");
						doit_weather(json, text);
					}else if(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "pm25")){		//pm25 json信息	
						printf("\n pm25 \n\n");
						doit_pm25(json, text);
					}else if((!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "openQA")) || 				\
							(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "datetime"))  || 				\
							(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "calc"))  || 				\
							(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "baike"))  || 				\
							(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "faq"))  || 				\
							(!strcmp(cJSON_GetObjectItem(json, "service")->valuestring, "chat")) ){		//pm25 json信息	
					//	printf("\n openQA \n\n");
						doit_ODCBFC(json, text, doit_cond);
					}
			}
			
			
		}else{
			
			if(doit_cond == DOIT_VOICE)		//语音识别
			{
				
				//简约行走控制命令	小汕前进，小汕后退，小汕左转，小汕右转，	早上前进。	
				printf("len:%d\n", strlen(cJSON_GetObjectItem(json, "text")->valuestring));
				printf("text%s\n", cJSON_GetObjectItem(json, "text")->valuestring);	
					
				if(NULL != strstr(cJSON_GetObjectItem(json, "text")->valuestring, "前进")){
					
					//前进处理
					for(ii = 0; ii < 10; ii++){
						send_to_ROBOT("Zib:R:A");
						usleep(100000);
					}
					printf("前进\n");
					memset(text, 0, 4096);
					strcpy(text,cJSON_GetObjectItem(json, "text")->valuestring);
					cJSON_Delete(json);	
					return MSP_SUCCESS;
				}else if(NULL != strstr(cJSON_GetObjectItem(json, "text")->valuestring, "后退")){
					
					//后退处理
					for(ii = 0; ii < 10; ii++){
						send_to_ROBOT("Zib:R:B");
						usleep(100000);
					}
					printf("后退\n");
					memset(text, 0, 4096);
					strcpy(text,cJSON_GetObjectItem(json, "text")->valuestring);
					cJSON_Delete(json);	
					return MSP_SUCCESS;
				}else if(NULL != strstr(cJSON_GetObjectItem(json, "text")->valuestring, "左转")){
					
					//左转处理
					for(ii = 0; ii < 10; ii++){
						send_to_ROBOT("Zib:R:C");
						usleep(100000);
					}
					printf("左转\n");
					memset(text, 0, 4096);
					strcpy(text,cJSON_GetObjectItem(json, "text")->valuestring);
					cJSON_Delete(json);	
					return MSP_SUCCESS;
				}else if(NULL != strstr(cJSON_GetObjectItem(json, "text")->valuestring, "右转")){
					
					//右转处理
					for(ii = 0; ii < 10; ii++){
						send_to_ROBOT("Zib:R:D");
						usleep(100000);
					}
					printf("右转\n");
					memset(text, 0, 4096);
					strcpy(text,cJSON_GetObjectItem(json, "text")->valuestring);
					cJSON_Delete(json);	
					return MSP_SUCCESS;
				}else if(NULL != strstr(cJSON_GetObjectItem(json, "text")->valuestring, "打开机械手")){
					
					//右转处理
					send_to_ROBOT("Hand:d");
					printf("正在打开机械手\n");
					memset(text, 0, 4096);
					strcpy(text,"正在打开机械手");
					cJSON_Delete(json);	
					return MSP_SUCCESS;
				}else if(NULL != strstr(cJSON_GetObjectItem(json, "text")->valuestring, "关闭机械手")){
					
					//右转处理
					send_to_ROBOT("Hand:Z");
					printf("正在关闭机械手\n");
					memset(text, 0, 4096);
					strcpy(text,"正在关闭机械手");
					cJSON_Delete(json);	
					return MSP_SUCCESS;
				}
			}
			memset(text, 0, 4096);
			sprintf(text, "小S找不到关于  \"%s\"  内容，老板可以换一种说法试试。", cJSON_GetObjectItem(json, "text")->valuestring);
			printf("text:%s\n",text);
		}
	}		
	
	cJSON_Delete(json);	
	return MSP_SUCCESS;
}


int voice_again(const char *test, const char *session_begin_params_write)
{
	/* 合成语音 */
	printf("开始合成 ...\n");
			
	int ret = text_to_speech(test, "/xww/wav/sound.wav", session_begin_params_write);
	if (MSP_SUCCESS != ret)
	{
		printf("text_to_speech failed, error code: %d.\n", ret);
	}
	printf("合成完毕\n");
	
	
	//复读
	
	//system("aplay "/xww/wav/sound.wav");
	return MSP_SUCCESS;
	
}


/****************
***文字识别----语义识别---智能对答
session_begin_params = 语义识别对应配置		session_begin_params = 智能对答对应配置
****************/
int voice_teSmart_answer(const char *test, const char *session_begin_params, const char *session_begin_params_write,const short doit_cond)
{
	/* 合成语音 */
	//printf("开始合成 ...\n");
			
	int ret = text_to_speech(test, "/xww/wav/sound.wav", session_begin_params_write);
	if (MSP_SUCCESS != ret)
	{
		printf("text_to_speech failed, error code: %d.\n", ret);
		strcpy(g_result, "系统错误，识别出错");
		return ret;
	}
	//printf("合成完毕\n");
	
	//语义识别：开始识别--获取对答消息
	ret = demo_file("/xww/wav/sound.wav", session_begin_params);
	if(MSP_SUCCESS != ret)
	{
		MSPLogout(); // Logout...
		strcpy(g_result, "系统错误，识别出错");
		return ret;
	}
	//识别成功
	ret = doit(g_result, doit_cond);
	if(ret != 0)
	{
		printf("\n\n%s\n", g_result);
		printf("doit error code = %d\n",ret);
		strcpy(g_result, "老板，小S听不清楚您再说什么，您可以说话大声点，或者换另外一种方式");
		//system("aplay "/xww/wav/error1.wav");
		return ret;
	}
	printf("\n\n%s\n", g_result);
	/*
	ret = voice_again(g_result,session_begin_params_write);
	if(ret != 0)		//解析出错
	{
		printf("text_to_speech error code = %d\n",ret);
		system("aplay "/xww/wav/error1.wav");
		return ret;
	}*/
	
	return MSP_SUCCESS;
	
}

/****************
***语义识别----智能对答-语义识别
session_begin_params = 语义识别对应配置	session_begin_params = 智能对答对应配置
****************/
int voice_Smart_answer(const char *session_begin_params, const char *session_begin_params_write,const short doit_cond)
{
	printf("\n\n\nread_tell the speech from microphone\n");
	printf("Speak in 10 seconds\n");

	int ret = demo_mic(session_begin_params);
	if(MSP_SUCCESS != ret)
	{
		MSPLogout(); // Logout...
		strcpy(g_result, "系统错误，识别出错");
		return ret;
	}
	printf("10 sec passed\n");
	//识别成功
	ret = doit(g_result, doit_cond);
	if(ret != 0)		//解析出错
	{
		printf("doit error code = %d\n",ret);
//		system("aplay "/xww/wav/error1.wav");
		strcpy(g_result, "老板，小S听不清楚您再说什么，您可以说话大声点，或者换另外一种方式");
		return ret;
	}
	printf("g_result\n\n%s\n", g_result);
	
	return MSP_SUCCESS;
}
