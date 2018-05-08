#include "default/gfapi.h"
#include "zip_uppack.h"


static int CreateDir(const char *sPathName, mode_t mode)  
{  
	char DirName[256];  
	strcpy(DirName, sPathName);  
	int i, len = strlen(DirName);  
	if(DirName[len - 1] != '/')  
	strcat(DirName, "/");  
	
	len = strlen(DirName);  
	
	for(i=1; i<len; i++)  
	{  
		if(DirName[i]=='/')  
		{  
			DirName[i]   =   0;  
			if(access(DirName, 0)!=0)  
			{  
				if(mkdir(DirName,0755)==-1)  
				{   
					printf("CreateDir error\n");   
					return   -1;   
				}  
			}  
			DirName[i]   =   '/';  
		}  
	}  	
	return   0;  
} 


void zip_printf_head(Zip_head *head)
{
	Zip_filehead *H = &head->H;
	Zip_cenhead *C = &head->C;
	Zip_endhead *E = &head->E;
	char filename[64] = {0};
	char dirname[64] = {0};
	memset(filename, 0, sizeof(filename));
	memcpy(filename, H->filename, H->name_size);
	memset(dirname, 0, sizeof(dirname));
	memcpy(dirname, C->dirname, C->cen_name_size);


	printf("\n========== ZIP PRINTF HEAD==========\n\n");

	printf("H.head:%8x\n", H->head);
	printf("H.version:%04x\n", H->version);
	printf("H.zipbit:%04x\n", H->zipbit);
	printf("H.zipmode:%04x\n", H->zipmode);
	printf("H.last_time:%04x\n", H->last_time);
	printf("H.change_time:%04x\n", H->change_time);
	printf("H.crc_check:%08x\n", H->crc_check);
	printf("H.size:%08x\n", H->size);
	printf("H.last_size:%08x\n", H->last_size);
	printf("H.name_size:%04x\n", H->name_size);
	printf("H.exten_size:%04x\n", H->exten_size);
	printf("H.filename:%s\n", filename);

	printf("\n");

	printf("C.cen_head:%08x\n", C->cen_head);
	printf("C.cen_version:%04x\n", C->cen_version);
	printf("C.cen_Lversion:%04x\n", C->cen_Lversion);
	printf("C.cen_zipbit:%04x\n", C->cen_zipbit);
	printf("C.cen_zipmode:%04x\n",C->cen_zipmode);
	printf("C.cen_last_changetime:%04x\n", C->cen_last_changetime);
	printf("C.cen_last_changedate:%04x\n", C->cen_last_changedate);
	printf("C.cen_crc_check:%08x\n", C->cen_crc_check);
	printf("C.cen_size:%08x\n", C->cen_size);
	printf("C.cen_last_size:%08x\n", C->cen_last_size);
	printf("C.cen_name_size:%04x\n", C->cen_name_size);
	printf("C.cen_exten_size:%04x\n", C->cen_exten_size);
	printf("C.cen_anno_size:%04x\n", C->cen_anno_size);
	printf("C.cen_start_num:%04x\n", C->cen_start_num);
	printf("C.cen_inside_attr:%04x\n", C->cen_inside_attr);
	printf("C.cen_outside_attr:%08x\n", C->cen_outside_attr);
	printf("C.cen_head_offset:%08x\n", C->cen_head_offset);
	printf("dirname:%s\n", dirname);
	
	printf("\n");

	printf("C.end_head:%08x\n", E->end_head);
	printf("C.end_disk_num:%04x\n", E->end_disk_num);
	printf("C.end_cendisk_num:%04x\n", E->end_cendisk_num);
	printf("C.end_cendir_num:%04x\n", E->end_cendir_num);
	printf("C.end_cen_num:%04x\n", E->end_cen_num);
	printf("C.end_size:%08x\n", E->end_size);
	printf("C.end_start_offset:%08x\n", E->end_start_offset);
	printf("C.end_anno_size:%04x\n", E->end_anno_size);
}

int zip_check_head(char *RB)
{
	unsigned int head;
	memcpy(&head, RB, 4);
	switch(head){
		case 0x04034b50: return 1;
		case 0x02014b50: return 2;
		case 0x06054b50: return 3;
		default : break;
	}
	return -1;
}

int zip_read_filehead(Zip_filehead *H, char *RB, bool is_create, char *savepath)
{
	unsigned int head, len = 0;
	memcpy(&head, RB, 4);
	if(head != 0x04034b50)
		return -2;
	if(RB == NULL)
		return -1;

#ifdef F_OPEN
	FILE *fq;
	unsigned int count;
#else
	int fq;
#endif
	char filename[64] = {0};
	char *p = (char *)H;

	memset(p, 0, sizeof(Zip_filehead));
	memcpy(p, RB, 14);
	memcpy(p+16, RB+14, 16);
	H->filename = RB + 30;
	H->filebuf = RB + 30 + H->name_size;
	memset(filename, 0, sizeof(filename));
	if(NULL != savepath)
	{
		len = strlen(savepath);
		if(len < 32 && savepath[len-1] == '/')
		{
			strncat(filename, savepath, len);
		}
	}
	strncat(filename, H->filename, H->name_size);
#ifdef DEBUG_ALL
	printf("H.head:%8x\n", H->head);
	printf("H.version:%04x\n", H->version);
	printf("H.zipbit:%04x\n", H->zipbit);
	printf("H.zipmode:%04x\n", H->zipmode);
	printf("H.last_time:%04x\n", H->last_time);
	printf("H.change_time:%04x\n", H->change_time);
	printf("H.crc_check:%08x\n", H->crc_check);
	printf("H.size:%08x\n", H->size);
	printf("H.last_size:%08x\n", H->last_size);
	printf("H.name_size:%04x\n", H->name_size);
	printf("H.exten_size:%04x\n", H->exten_size);
	printf("H.filename:%s\n", filename);
#endif
	if(true == is_create)
	{
		int ret;
		DIR *mdir = NULL;
		if(filename[H->name_size + len -1] == '/')
		{
			if(NULL == (mdir = opendir(filename)))
			{
				CreateDir(filename, 0775);
#ifdef DEBUG
				printf("CreateDir %s\t suc filename:%s\n", filename, filename);
#endif
			}
			if(mdir)
				closedir(mdir);
		}else
		{
			for(ret = H->name_size-1; ret > 0; ret--)
			{
				if(filename[ret + len] == '/')
					break;
			}
			if(ret > 0)
			{
				char dirname[64] = {0};
				strncpy(dirname, filename, ret + len);
				if(NULL == (mdir = opendir(dirname)))
				{
					CreateDir(dirname, 0775);
#ifdef DEBUG
					printf("CreateDir %s\t filename:%s suc\n", dirname, filename);
#endif
				}
				if(mdir)
					closedir(mdir);
			}
#ifdef F_OPEN
			fq = fopen(filename, "wb");
			if(fq != NULL)
			{
				count = H->size/1024;
				if(count)
					fwrite(H->filebuf, 1024, count, fq);
				count = H->size%1024;
				if(count)
					fwrite(H->filebuf + H->size/1024*1024, count, 1, fq);
				fclose(fq);
#ifdef DEBUG
				printf("create file:%s suc\n", filename);
#endif
			}
#else
			fq = open(filename, O_CREAT|O_TRUNC|O_WRONLY, 0775);
			if(fq > 0)
			{
				ret = 0;
				while(ret < H->size)
				{
					if(ret/1024 == H->size/1024)
						ret += write(fq, H->filebuf + ret, H->size - ret);
					else
						ret += write(fq, H->filebuf + ret, 1024);
				}
				if(ret != H->size)
					printf("write err ret:%d\tsize:%d\n", ret, H->size);
				close(fq);
#ifdef DEBUG
				printf("create file:%s suc\n", filename);
#endif
			}
#endif
		}
#if 0
		char dirname[64] = {0};
		if(NULL != (dirname = strstr(filename, "\/"))
			access()
#endif
	}
	H->m_size =  (30  + H->name_size + H->size + H->exten_size);

	return H->m_size;
}

int zip_read_cenhead(Zip_cenhead *C, char *RB)
{
	unsigned int head;
	memcpy(&head, RB, 4);
	if(head != 0x02014b50)
		return -2;
	if(RB == NULL)
		return -1;

	char dirname[64];
	char *p = (char *)C;
	memset(p, 0, sizeof(Zip_cenhead));
	memcpy(p, RB, 38);
	memcpy((p + 40), (RB + 38), 8);
	C->dirname = RB + 46;
	memset(dirname, 0, sizeof(dirname));
	memcpy(dirname, C->dirname, C->cen_name_size);

#ifdef DEBUG_ALL
	printf("C.cen_head:%08x\n", C->cen_head);
	printf("C.cen_version:%04x\n", C->cen_version);
	printf("C.cen_Lversion:%04x\n", C->cen_Lversion);
	printf("C.cen_zipbit:%04x\n", C->cen_zipbit);
	printf("C.cen_zipmode:%04x\n",C->cen_zipmode);
	printf("C.cen_last_changetime:%04x\n", C->cen_last_changetime);
	printf("C.cen_last_changedate:%04x\n", C->cen_last_changedate);
	printf("C.cen_crc_check:%08x\n", C->cen_crc_check);
	printf("C.cen_size:%08x\n", C->cen_size);
	printf("C.cen_last_size:%08x\n", C->cen_last_size);
	printf("C.cen_name_size:%04x\n", C->cen_name_size);
	printf("C.cen_exten_size:%04x\n", C->cen_exten_size);
	printf("C.cen_anno_size:%04x\n", C->cen_anno_size);
	printf("C.cen_start_num:%04x\n", C->cen_start_num);
	printf("C.cen_inside_attr:%04x\n", C->cen_inside_attr);
	printf("C.cen_outside_attr:%08x\n", C->cen_outside_attr);
	printf("C.cen_head_offset:%08x\n", C->cen_head_offset);
	printf("dirname:%s\n", dirname);
#endif
	C->m_size = (46 + C->cen_name_size + C->cen_exten_size + C->cen_anno_size);

	return C->m_size;
}
int zip_read_endhead(Zip_endhead *E, char *RB)
{
	unsigned int head;
	memcpy(&head, RB, 4);
	if(head != 0x06054b50)
		return -2;
	if(RB == NULL)
		return -1;

	char *p = (char *)E;
	memset(p, 0, sizeof(Zip_endhead));
	memcpy(p, RB, 22);
#ifdef DEBUG_ALL
	printf("C.end_head:%08x\n", E->end_head);
	printf("C.end_disk_num:%04x\n", E->end_disk_num);
	printf("C.end_cendisk_num:%04x\n", E->end_cendisk_num);
	printf("C.end_cendir_num:%04x\n", E->end_cendir_num);
	printf("C.end_cen_num:%04x\n", E->end_cen_num);
	printf("C.end_size:%08x\n", E->end_size);
	printf("C.end_start_offset:%08x\n", E->end_start_offset);
	printf("C.end_anno_size:%04x\n", E->end_anno_size);
#endif
	E->m_size = 22;
	return E->m_size;
}
#ifdef ZTASK
int zip_uppack(Zip_head *G, char *RB, unsigned long size, char *savepath)
{
	int ret = 0, num = 0;
	DIR *mdir = NULL;
	char msavepath[64];
	if(RB == NULL || G == NULL)
		return -1;

	if(NULL != savepath)
	{
		memset(msavepath, 0, sizeof(msavepath));
		strcpy(msavepath, savepath);

		if(msavepath[strlen(msavepath) -1] != '/')
			msavepath[strlen(msavepath)] = '/';

		if(NULL == (mdir = opendir(msavepath)))
		{
			CreateDir(msavepath, 0775);
		}
		if(mdir)
			closedir(mdir);
	}

	while(num < size)
	{
		ret = zip_check_head(RB + num);
		if(ret == 1)
			num += zip_read_filehead(&G->H, RB + num, true, msavepath);
		else if(ret == 2)
			num += zip_read_cenhead(&G->C, RB + num);
		else if(ret == 3)
			num += zip_read_endhead(&G->E, RB + num);
		else
		{
#ifdef DEBUG_ALL
			printf("check head err\n");
			break;
#endif
		}
#ifdef DEBUG_ALL
		printf("num: %d size:%ld\n", num, size);
#endif
	}
	if(num == size)
	{
#ifdef DEBUG_ALL
		printf("do zip suc num: %d size:%ld\n", num, size);
#endif
		return num;
	}
	return -1;
}
#else
size_t zip_uppack(char *bufptr, size_t size, char *savepath)
{
	int ret = 0;
	size_t num = 0;
	Zip_head G;
	DIR *mdir = NULL;
	char msavepath[64];

	if(bufptr == NULL)
		return -1;
	
	if(NULL != savepath)
	{
		memset(msavepath, 0, sizeof(msavepath));
		strcpy(msavepath, savepath);

		if(msavepath[strlen(msavepath) -1] != '/')
			msavepath[strlen(msavepath)] = '/';

		if(NULL == (mdir = opendir(msavepath)))
		{
			CreateDir(msavepath, 0775);
		}
		if(mdir)
			closedir(mdir);
	}

	while(num < size)
	{
		ret = zip_check_head(bufptr + num);
		if(ret == 1)
			num += zip_read_filehead(&G.H, bufptr + num, true, msavepath);
		else if(ret == 2)
			num += zip_read_cenhead(&G.C, bufptr + num);
		else if(ret == 3)
			num += zip_read_endhead(&G.E, bufptr + num);
		else
		{
#ifdef DEBUG_ALL
			printf("check head err\n");
			break;
#endif
		}
#ifdef DEBUG_ALL
		printf("num: %ld size:%ld\n", num, size);
#endif
	}
	if(num == size)
	{
#ifdef DEBUG_ALL
		printf("do zip suc num: %ld size:%ld\n", num, size);
#endif
		return num;
	}
#ifdef DEBUG_ALL
		printf("do zip err num: %ld size:%ld\n", num, size);
#endif
	return -1;
}
#endif

#ifdef ZTASK
Zip_head *zip_create_head(void)
{
	Zip_head *G = (Zip_head *)malloc(sizeof(Zip_head));
	return G;
}

void zip_free_head(Zip_head *head)
{
	if(NULL != head)
		free(head);
	return ;
}
#endif
