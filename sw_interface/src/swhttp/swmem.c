/** 
 * @file swmem.c
 * @brief 可分块/可跟踪内存泄漏位置的内存管理方式
 * @author hujinshui / huanghuaming
 * @modify by xusheng
 * @date 2010-07-05
 */
#include "swapi_linux.h"
#include "swmem.h"
#include "swthrd.h"
#include "swtype.h"

#define ALIGN_SIZE(size)	((size+align) & ~align)		//整数对齐 , 这里的align：xm->align-1
#define ALIGN_PTR(p,size)	((size+align+(int)p) & ~align)  //指针((char*)p+size)对齐

typedef struct memheader  
{
#ifdef _DEBUG
	char filename[16];
	int line;
#endif
	int size;
	struct memheader *next;
}smemnode_t;

typedef struct
{
	char *buf;	//可支配的缓冲区
	int size;	//可支配的缓冲区大小
	int align;	//对齐字节数
#ifdef _DEBUG
	int maxpos;	//最大分配位置
#endif
	
	smemnode_t *first, *tail;	//其后有空闲块的结点，first是最前的空闲块结点，tail是最后的空闲块结点
	int state;	//当前状态: 0=空闲, 1=正在分配, 2=正在删除
	bool used;
	
	smemnode_t *list;	//分配链表 
}sxmem_t;


/** 
 * @brief 找到文件的位置 
 * 
 * @param path	文件路径	 
 * 
 * @return 返回一个文件路径
 */
static const char *GetPathFile( const char *path )
{
	int n = strlen(path)-1;

	while( 0 < n )
	{
		if( path[n] == '/' || path[n] == '\\' )
		{		
			return path+n+1;
		}
		n--;
	}

	return path;
}


/** 
 * @brief 找到内存p在链表中的位置 
 * 
 * @param h	内存句柄
 * @param *p	指向的内存地址 
 * 
 * @return 成功,返回链表节点; 失败,返回空值
 */
static smemnode_t* find( HANDLE h, void *p )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");     

	    return NULL;	
	}

	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node = xm->list;
	unsigned int align = xm->align-1;

	while( node )
	{
		if( p == (char*)ALIGN_PTR(node,sizeof(smemnode_t)) )
			break;
		node = node->next;
	}

	return node;
}


/** 
 * @brief	扫描链表看是否有free块的内存符合申请的要求 
 * 
 * @param h	内存句柄
 * @param size	已分配内存的大小
 * @param filename	所在的当前文件名
 * @param line	所在的当前行号
 * 
 * @return 成功返回分配后内存的起始地址; 否则,返回空值？？？？（不确定）
 */
static void* malloc_old( HANDLE h, int size ,const char *filename, int line )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");      

		return NULL;	
	}

	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node, *new_node;
	unsigned int align = xm->align-1;
	char *end;
	int len;
	int firstlen;	//第一个空闲块的大小

	if( xm->first==NULL )	 //检查头和buf之间的空隙;
	{
		if( xm->list == NULL )
		{
			return NULL;
		}

		node = xm->list;
		len = ((int)node) - ALIGN_PTR(xm->buf,sizeof(smemnode_t));
		if( size <= len )
		{
			//填充NODE
			new_node = (smemnode_t *)xm->buf;
			memset( new_node, 0, sizeof(smemnode_t) );
#ifdef _DEBUG
			strncpy( new_node->filename, GetPathFile(filename), sizeof(new_node->filename)-1 );
			new_node->line = line;
#endif
			new_node->size = ALIGN_SIZE( size );

			//插入到链表头;
			xm->list = new_node;
			new_node->next = node;
			xm->first = new_node;

			return (char*)ALIGN_PTR(new_node, sizeof(smemnode_t)); 
		}
		firstlen = len;
	}
	else
	{
		node = xm->first;
		firstlen = 0;
	}

	//检查相临两块之间的间隙是否大于下一块的长度;
	while( node->next )
	{
		end = (char*)ALIGN_PTR(node,sizeof(smemnode_t)) + ALIGN_SIZE(node->size);
		len = ((char*)node->next) - (char*)ALIGN_PTR(end, sizeof(smemnode_t));

		if( firstlen <= (int)align )	//如果第一空闲块太小，不够最低分配字节数，就修改这个指针，
		{
			xm->first = node;
			firstlen = len;
		}

		//找到符合要求的块;
		if( size <= len )
		{
			//填充NODE
			new_node = (smemnode_t *)end;
			memset( new_node, 0, sizeof(smemnode_t) );
#ifdef _DEBUG
			strncpy( new_node->filename, GetPathFile(filename), sizeof(new_node->filename)-1 );
			new_node->line = line;
#endif
			new_node->size = ALIGN_SIZE( size );

			//插入到链表;
			new_node->next = node->next;
			node->next = new_node;

			return (char*)ALIGN_PTR(new_node, sizeof(smemnode_t)); 
		}

		node=node->next;	//加入到分配链表;
	}

	return NULL;
}

/** 
 * @brief 对已分配的内存进行初始化 
 * 
 * @param buf 	已分配的内存地址
 * @param size 	已分配内存的大小 
 * @param align 字节分配的方式
 * 
 * @return 成功,返回内存句柄; 失败,返回空值
 */
HANDLE sw_mem_init( char *buf, int size, int align )
{

	sxmem_t *xm = (sxmem_t *)(((align-1+(int)buf)/align)*align);

	memset( xm, 0, sizeof(sxmem_t) );
	xm->align = align;
	align--;
	xm->buf = (char *)ALIGN_PTR( xm, sizeof(sxmem_t) );		//可分配内存的起点
	xm->size = buf + size - xm->buf;						//可分配内存的大小

	return xm;
}

/** 
 * @brief 释放内存句柄
 * 
 * @param h 内存句柄
 */
void sw_mem_exit( HANDLE h )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");     

	   	return ;	
	}

#ifdef _DEBUG
	sw_mem_check( h );
#endif
}

/** 
 * @brief 释放内存句柄, 不做内存泄露检查, 提高执行效率
 * 
 * @param h 内存句柄
 */
void sw_mem_exit_nocheck(HANDLE h)
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");     
	   	return ;	
	}
}

/** 
 * @brief 内存清空
 * 
 * @param h 内存句柄
 */
void sw_mem_reset( HANDLE h )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");       

		return ;
	}

	((sxmem_t *)h)->list = NULL;
}

/** 
 * @brief 检查是否有未释放的内存 
 * 
 * @param h 内存句柄
 */
void sw_mem_check( HANDLE h )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");  

		return ;	
	}

	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node = xm->list;
	int node_count, used_size, available_size, max_available;
	char *p, *end;
	int available;
	unsigned int align = xm->align-1;

	while( node )
	{
#ifdef _DEBUG
		printf( "===>MEMORY LEAK: %s, line %d, %d bytes\n", node->filename, node->line, node->size );
#endif
		node = node->next;
	}
#ifdef _DEBUG
	printf( "MAX_POS=%d\n", xm->maxpos );
#endif

	node_count = 0;
	used_size = 0;
	available_size = 0;
	max_available = 0;

	if( xm->list )
	{
		available = (int)xm->list - ALIGN_PTR(xm->buf,sizeof(smemnode_t));
		if( available > 0 )
		{
			if( max_available < available )
			{		
				max_available = available;
			}
			available_size += available;
		}
		for( node = xm->list; node; node = node->next )
		{
			used_size += node->size;

			if( node->next )
			{
				end = (char*)ALIGN_PTR(node,sizeof(smemnode_t)) + ALIGN_SIZE(node->size);
				available = ((char*)node->next) - (char*)ALIGN_PTR(end, sizeof(smemnode_t));
			}
			else
			{
				p = (char*)ALIGN_PTR( node, sizeof(smemnode_t) );
				end = (char *)ALIGN_PTR( p, node->size );
				p = (char*)ALIGN_PTR( end, sizeof(smemnode_t) );
				available = ALIGN_SIZE( xm->buf+xm->size - p );
			}
			if( available > 0 )
			{
				if( max_available < available )
				{	
					max_available = available;
				}
				available_size += available;
			}
			node_count ++;
		}
	}
	else
	{
		p = (char*)ALIGN_PTR( xm->buf, sizeof(smemnode_t) );
		available_size = ALIGN_SIZE( xm->buf+xm->size - p );
		max_available = available_size;
	}

	printf( "%d nodes, %d bytes allocated.\tmemory 0x%x, %d total bytes, %d bytes available, %d bytes max block\n", node_count, used_size, h, xm->size, available_size, max_available );
}

/** 
 * @brief 获取总的内存大小
 * 
 * @param h 内存句柄
 * 
 * @return 总的内存大小
 */
int sw_mem_get_total_size( HANDLE h )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");      

	   	return -1;	
	}

	return ((sxmem_t *)h)->size;
}

/** 
 * @brief 取历史上最大已分配尺寸
 * 
 * @param h 内存句柄
 * 
 * @return 最大已分配尺寸的大小
 */
int sw_mem_get_max_size( HANDLE h )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");      

	   	return -1;	
	}

#ifdef _DEBUG
	return ((sxmem_t *)h)->maxpos;
#else
	return sw_mem_get_cur_size( h );
#endif
}

/** 
 * @brief 取现在最大分配尺寸
 * 
 * @param h 内存句柄
 * 
 * @return 现在分配尺寸的大小
 */
int sw_mem_get_cur_size( HANDLE h )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");      
	   	
		return -1;	
	}

	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node;
	char *p;
	unsigned int align = xm->align-1;
	int size = 0;

	if( xm->tail )
	{
		node = xm->tail;
		p = (char*)ALIGN_PTR( node, sizeof(smemnode_t) );
		size = p-xm->buf + node->size;
	}

	return size;
}

/** 
 * @brief 是否没有分配已分配的节点
 * 
 * @param h 内存句柄
 * 
 * @return 如果没有已分配的节点,则返回真(true);否则,返回假(false)
 */
bool sw_mem_is_empty( HANDLE h )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");    
	 	
		return 1;	
	}

	sxmem_t *xm = (sxmem_t *)h;

	return xm->list == NULL;
}

/** 
 * @brief 从内存句柄所指向的内存中分配一段内存
 * 
 * @param h 内存句柄
 * @param size 分配内存的大小
 * @param filename 所在的当前文件名 
 * @param line 所在的当前行号
 * 
 * @return 成功返回分配后内存的地址; 否则,返回空值
 */
void *sw_mem_alloc( HANDLE h, int size, const char *filename, int line )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");		

		return NULL;
	}

	sxmem_t *xm = (sxmem_t*)h;
	smemnode_t *tail = NULL;
	smemnode_t *node;
	unsigned int align = xm->align-1;
	char *p;

WAIT:
	while( xm->used )
	{	
		sw_thrd_delay( 10 );
	}
	xm->used = 1;
	if( xm->state )
	{	
		goto WAIT;
	}
	xm->state = 1;

	//从free块分配
	p = (char *)malloc_old( h, size, filename, line );
	if( p )
	{
#ifdef _DEBUG
		if( xm->maxpos < (p-xm->buf) + size )
		{	
			xm->maxpos = (p-xm->buf) + size;
		}
#endif

		xm->state = 0;
		xm->used = 0;

		return p;
	}

	//第一次申请
	if( xm->list == NULL )
	{	
		node = (smemnode_t *)xm->buf;
	}
	else
	{
		tail = xm->tail;

		p = (char*)ALIGN_PTR( tail, sizeof(smemnode_t) );	//最后那个结点的buf
		node = (smemnode_t *)ALIGN_PTR( p, tail->size );	//最后那个结点的buf的尾，可以作为下一结点地址
	}

	p = (char*)ALIGN_PTR( node, sizeof(smemnode_t) );	//获取结点的buf

	//检查是否有足够的内存
	if( xm->buf+xm->size < p+size )
	{
		printf( "no enough memory\n" );
		xm->state = 0;
		xm->used = 0;

		return NULL;
	}

	//填充node
	memset( node, 0, sizeof(smemnode_t) );
	node->size = ALIGN_SIZE( size );
#ifdef _DEBUG
	strncpy( node->filename, GetPathFile(filename), sizeof(node->filename)-1 );
	node->line = line;
#endif

	//把当前节点挂到链表尾部
	if( tail == NULL )
	{	
		xm->list = node;
	}
	else
	{	
		tail->next = node;
	}
#ifdef _DEBUG
	if( xm->maxpos < (p-xm->buf) + size )
	{	
		xm->maxpos = (p-xm->buf) + size;
	}
#endif

	xm->tail = node;
	xm->state = 0;
	xm->used = 0;

	return p;
}

/** 
 * @brief 			释放内存句柄所指向的一段内存
 * 
 * @param h 		内存句柄
 * @param p 		指向的内存地址
 * @param filename 	所在的当前文件名
 * @param line 		所在的当前行号
 */
void sw_mem_free( HANDLE h, void *p, const char *filename, int line )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");      

	   	return ;	
	}

	sxmem_t *xm = (sxmem_t *)h;
	smemnode_t *node;
	smemnode_t *last = NULL;
	unsigned int align = xm->align-1;

WAIT:
	while( xm->used )
	{	
		sw_thrd_delay( 10 );
	}
	xm->used=1;
	if( xm->state )
	{	
		goto WAIT;
	}
	xm->state = 2;

	//寻找node
	for( node=xm->list; node; node=node->next )
	{
		if( p == (void*)ALIGN_PTR(node,sizeof(smemnode_t)) )
		{		
			break;
		}
		last = node;
	}

	//释放一块不存在的内存
	if( node == NULL )
	{
		printf( "sw_mem_free error: %s, line %d, memory 0x%x\n", GetPathFile(filename), line, p );
	}
	else
	{
		//释放的为头节点
		if( node == xm->list )
		{	
			xm->list = node->next;
		}
		else
		{	
			last->next = node->next;
		}

		if( xm->first >= node )
		{	
			xm->first = last;
		}
		if( xm->tail == node )
		{	
			xm->tail = last;
		}
	}

	xm->state = 0;
	xm->used = 0;
}

/** 
 * @brief 从内存句柄指向的内存中复制字符串
 * 
 * @param h 内存句柄
 * @param s 指向的字符串
 * @param filename 所在的当前文件名
 * @param line 所在的当前行号
 * 
 * @return 成功,返回复制后的字符串指针; 否则,返回空值
 */
char *sw_mem_strdup( HANDLE h, const char *s, const char *filename, int line )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");      
	    
		return NULL;	
	}

	char *p = (char*)sw_mem_alloc( h, strlen(s)+1, filename, line );

	if( p )
	{	
		memcpy(p,s,strlen(s)+1);
	}

	return p;
}

/** 
 * @brief 在原有内存的基础上重新申请内存
 * 
 * @param h 内存句柄
 * @param p 指向原有的内存
 * @param size 分配内存的大小
 * @param filename 所在的当前文件名
 * @param line 所在的当前行号
 * 
 * @return 成功,返回实际分配后的新地址; 否则,返回空值
 */
void *sw_mem_realloc( HANDLE h, void *p, int size, const char *filename, int line )
{
	if( NULL == (sxmem_t *)h ) 
	{
		printf("input error,handle is NULL");      

		return NULL;	
	}

	void *buf = NULL;
	sxmem_t *xm = (sxmem_t*)h;
	smemnode_t* node = NULL;
	unsigned int align = xm->align-1;

	//定位;
	if( p )
	{
WAIT:
		while( xm->used )
		{	
			sw_thrd_delay( 10 );
		}
		xm->used=1;
		if( xm->state )
		{	
			goto WAIT;
		}
		xm->state = 1;
		node = find( h, p );
		if( node &&
				(size<=node->size ||
				 (char*)ALIGN_PTR(node,sizeof(smemnode_t))+size <= ( node->next ? (char*)node->next : xm->buf + xm->size) ) )
		{
			node->size = ALIGN_SIZE( size );
			xm->state = 0;
			xm->used = 0;

			return p;
		}
		xm->state = 0;
		xm->used = 0;
	}

	//内存不够,重新分配;
	buf = sw_mem_alloc( h, size, filename, line );
	if( p )
	{
		//拷贝内容;
		memcpy( buf, p, node ? node->size:size );
		//释放原来的内存;
		sw_mem_free( h, p, filename, line );
	}
	
	return buf;
}


















