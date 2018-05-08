#ifndef __ZIP_UPPACK_H__
#define __ZIP_UPPACK_H__

typedef struct _zip_filehead{
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
}Zip_filehead;

typedef struct _zip_cenhead{
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
}Zip_cenhead;

typedef struct _zip_endhead{
	unsigned int	end_head;
	unsigned short	end_disk_num;
	unsigned short	end_cendisk_num;
	unsigned short	end_cendir_num;
	unsigned short	end_cen_num;
	unsigned int	end_size;
	unsigned int	end_start_offset;
	unsigned short	end_anno_size;

	unsigned int m_size;
}Zip_endhead;

typedef struct _ziphead{
	Zip_filehead H;
	Zip_cenhead C;
	Zip_endhead E;
}Zip_head;



#if ZTASK

void zip_printf_head(Zip_head *head);
int zip_uppack(Zip_head *G, char *RB, unsigned long size, char *savepath);
Zip_head *zip_create_head(void);
void zip_free_head(Zip_head *head);

#else

size_t zip_uppack(char *bufptr, size_t size, char *savepath);

#endif

int zip_read_filehead(Zip_filehead *H, char *RB, bool is_create, char *savepath);
int zip_read_cenhead(Zip_cenhead *C, char *RB);
int zip_read_endhead(Zip_endhead *E, char *RB);

int zip_check_head(char *RB);


#endif /*__ZIP_UPPACK_H__*/
