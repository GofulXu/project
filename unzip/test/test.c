#include "default/gfapi.h"

typedef struct _head{
	unsigned int head;
	unsigned short version;
	unsigned short zipbit;
	unsigned short zipmode;
	unsigned short last_time;
	unsigned short change_time;
	unsigned int crc_check;
	unsigned int size;
	unsigned int last_size;
	unsigned short name_size;
	unsigned short exten_size;

	unsigned int m_size;
    char *filename;
	char *filebuf;	
}m_head;

typedef struct _cen_head{
	unsigned int	cen_head;
	unsigned short	  cen_version;
	unsigned short	  cen_Lversion;
	unsigned short	  cen_zipbit;
	unsigned short	  cen_zipmode;
	unsigned short	  cen_last_changetime;
	unsigned short	  cen_last_changedate;
	unsigned int	cen_crc_check;
	unsigned int	cen_size;
	unsigned int	cen_last_size;
	unsigned short	cen_name_size;
	unsigned short	cen_exten_size;
	unsigned short	cen_anno_size;
	unsigned short	cen_start_num;
	unsigned short	cen_inside_attr;
	unsigned int	cen_outside_attr;
	unsigned int	cen_head_offset;

	unsigned int m_size;
	char *dirname;
}m_cen_head;

typedef struct end_head{
	unsigned int	end_head;
	unsigned short	end_disk_num;
	unsigned short	end_cendisk_num;
	unsigned short	end_cendir_num;
	unsigned short	end_cen_num;
	unsigned int	end_size;
	unsigned int	end_strat_offset;
	unsigned short	end_anno_size;

	unsigned int m_size;
}m_end_head;

typedef struct _ziphead{
	m_head H;
	m_cen_head C;
	m_end_head E;
}m_ziphead;

int check_head(char *RB)
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

int create_head(m_head *H, char *RB, bool is_create)
{
	unsigned int head;
	memcpy(&head, RB, 4);
	if(head != 0x04034b50)
		return -2;
	if(RB == NULL)
		return -1;

	FILE *fq;
	char filename[64] = {0};
	char *p = (char *)H;
	unsigned int count;

	memset(p, 0, sizeof(m_head));
	memcpy(p, RB, 14);
	memcpy(p+16, RB+14, 16);
	H->filename = RB + 30;
	H->filebuf = RB + 30 + H->name_size;
		memset(filename, 0, sizeof(filename));
		memcpy(filename, H->filename, H->name_size);
#ifdef DEBUG
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
		if(filename[H->name_size -1] == '/')
		{
			if(NULL == (mdir = opendir(filename)))
			{
				mkdir(filename, 0775);
				printf("mkdir %s\t suc filename:%s\n", filename, filename);
			}
			if(mdir)
				closedir(mdir);
		}else
		{
			for(ret = H->name_size-1; ret > 0; ret--)
			{
				if(filename[ret] == '/')
					break;
			}
			if(ret > 0)
			{
				char dirname[64] = {0};
				strncpy(dirname, filename, ret);
				if(NULL == (mdir = opendir(dirname)))
				{
					mkdir(dirname, 0775);
					printf("mkdir %s\t filename:%s suc\n", dirname, filename);
				}
				if(mdir)
					closedir(mdir);
			}
			fq = fopen(filename, "wb");
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
#if 0
		char dirname[64] = {0};
		if(NULL != (dirname = strstr(filename, "\/"))
			access()
#endif
	}
	H->m_size =  (30  + H->name_size + H->size + H->exten_size);
	return H->m_size;
}

int create_cenhead(m_cen_head *C, char *RB)
{
	unsigned int head;
	memcpy(&head, RB, 4);
	if(head != 0x02014b50)
		return -2;
	if(RB == NULL)
		return -1;

	char dirname[64];
	char *p = (char *)C;
	memset(p, 0, sizeof(m_cen_head));
	memcpy(p, RB, 38);
	memcpy((p + 40), (RB + 38), 8);
	C->dirname = RB + 46;
	memset(dirname, 0, sizeof(dirname));
	memcpy(dirname, C->dirname, C->cen_name_size);

#ifdef DEBUG
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
int create_endhead(m_end_head *E, char *RB)
{
	unsigned int head;
	memcpy(&head, RB, 4);
	if(head != 0x06054b50)
		return -2;
	if(RB == NULL)
		return -1;

	char *p = (char *)E;
	memset(p, 0, sizeof(m_cen_head));
	memcpy(p, RB, 22);
#ifdef DEBUG
	printf("C.end_head:%08x\n", E->end_head);
	printf("C.end_disk_num:%04x\n", E->end_disk_num);
	printf("C.end_cendisk_num:%04x\n", E->end_cendisk_num);
	printf("C.end_cendir_num:%04x\n", E->end_cendir_num);
	printf("C.end_cen_num:%04x\n", E->end_cen_num);
	printf("C.end_size:%08x\n", E->end_size);
	printf("C.end_strat_offset:%08x\n", E->end_strat_offset);
	printf("C.end_anno_size:%04x\n", E->end_anno_size);
#endif
	E->m_size = 22;
	return E->m_size;
}

int main(int argc, char *argv[])
{
	struct stat mst;
	if(argc < 2)
		return 0;
	stat(argv[1], &mst);
	if(mst.st_size > 1024*1024*12)
	{
		printf("size:%ldM%ldk so big \n", mst.st_size/1024/1024, mst.st_size/(1024*1024));
		return 0;
	
	}
	char *RB = (char *)malloc(mst.st_size+1);
	FILE *fd = fopen(argv[1], "rb");
	fread(RB, 1024, mst.st_size/1024+1, fd);
	fclose(fd);

	m_ziphead G;

	int ret = 0, num = 0;
	while(1)
	{
		ret = check_head(RB + num);
		if(ret == 1)
			num += create_head(&G.H, RB + num, true);
		else if(ret == 2)
			num += create_cenhead(&G.C, RB + num);
		else if(ret == 3)
			num += create_endhead(&G.E, RB + num);
		else
			break;
#ifdef DEBUG
		printf("\n\n\n\nnum: %d\n", num);
#endif
	}
#if 0
   	ret = create_head(&G.H, RB, true);

	printf("\n\n\n\nret: %d\n", ret);

	ret += create_cenhead(&G.C, RB + ret);
	printf("\n\n\n\nret: %d\n", ret);

	ret += create_endhead(&G.E, RB + ret);


	memcpy(&G.E.end_head, RB + mst.st_size - 22, 4);
	printf("\n\n\nC.end_head:%08x\n", G.E.end_head);
//	memcpy(H.cen_head, RB+30+H.name_size+H.size, 4);


#endif

	free(RB);


	return 0;
}
