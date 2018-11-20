#include "stdafx.h"
#include "swjsonparser.h"

typedef struct _json_parse_
{
	char *keystartpos;	/* 属性的关键字开始位置 */
	int keylen;	/* 属性的关键字结束位置 */
	int objtype;	/* 关键字属性值的类型(json字串,json数组,json对象(不区分数值) */
	char *valuestartpos;	/* 属性值起始位置 */
	char *valueendpos;	/* 属性值结束位置 */
	struct _json_parse_	*children;	/* 子json位置 */
	struct _json_parse_	*brother;	/* 兄弟json */
}json_parser;

static json_parser* json_parse_obj(json_parser *header, char **json, HANDLE *parserbuf, int *parsernum, int recursion_depth);
static json_parser* json_parse_array(json_parser *header, char **json, HANDLE *parserbuf, int *parsernum, int recursion_depth);
typedef enum {
	JSON_ALPHA		= 1,
	JSON_DIGITAL	= 2,
	JSON_BEG_END	= 4,
	JSON_CONTRL		= 8,
	JSON_SEPARATOR	= 0x10,
	JSON_ESCAPE		= 0x20,
	JSON_TYPESETTING	= 0x40,
	JSON_HEX		= 0x81
}jsasciitype;
static int m_ascii2type[256] = 
{
	JSON_BEG_END, 0,  0,  0,  0,  0, 0, 0,
	JSON_CONTRL,  JSON_TYPESETTING, JSON_TYPESETTING,JSON_TYPESETTING, JSON_TYPESETTING,  JSON_TYPESETTING, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	JSON_TYPESETTING, 0, JSON_BEG_END, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, JSON_SEPARATOR, 0, 0, 0,
	JSON_DIGITAL, JSON_DIGITAL, JSON_DIGITAL, JSON_DIGITAL, JSON_DIGITAL, JSON_DIGITAL, JSON_DIGITAL, JSON_DIGITAL, 
	JSON_DIGITAL, JSON_DIGITAL, JSON_SEPARATOR, 0, 0 , 0, 0, 0, 
	0, JSON_HEX, JSON_HEX, JSON_HEX, JSON_HEX, JSON_HEX, JSON_HEX, JSON_ALPHA,
	JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA,
	JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA,
	JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_SEPARATOR, JSON_ESCAPE, JSON_SEPARATOR,  0, 0, 
	0, JSON_HEX, JSON_HEX, JSON_HEX, JSON_HEX, JSON_HEX, JSON_HEX, JSON_ALPHA,
	JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA,
	JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_ALPHA,
	JSON_ALPHA, JSON_ALPHA, JSON_ALPHA, JSON_SEPARATOR, 0, JSON_SEPARATOR, 0 ,0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

/*
 *@brief:返回本字符串前面的字符是否会引起本字符串为转意字符串
 *		本处的转意字符串只判断\b,\f,\n,\r,\t,\",\\,\u,\/
 *		不判断%形式的转义字符,目的是给json解析串使用,\", \\
*/
static inline bool be_Escape_Sequence(unsigned char *str)
{
	int i = 0;
	str--;
	while ( *str-- == '\\' )
		i++;
	if ( i & 0x01 )
		return true;
	else
		return false;
}
/* 查找到对应的'\"'位置，其引号不能是转意字符(此为json中定义的string类型) */
static inline char *find_correspond_quotedbl(char *str, char quotel, int *escape)
{
	*escape = 0;
	unsigned char *p = (unsigned char *)str;
	while ( *p && (*p != quotel || be_Escape_Sequence(p) )  )//JSON_BEG_END != m_ascii2type[*p]
	{
		if (*p == '\\')
			*escape = 1;
		p++;
	}
	if ( *p )
		return (char*)p;
	return NULL;
}
/* 查找到数字型json值结束位置,数字类型
(+-0~9[.] 0~9[e,E]-+0~9,还可以是16进制表示0x??,0B二进制表示，八进制,目前只判断结束标示为,},],不检查合法性*/
static inline char *find_correspond_integer(char *str)
{
//	while ( *str != ',' && *str != '\0' && *str != '}' && *str != ']')
//		str++;
	unsigned char *p = (unsigned char *)str;
	while ( JSON_SEPARATOR != m_ascii2type[*p] && *p )
		p++;
	if ( *p == ',' || *p == '}' || *p == ']' )
		return (char*)(p-1);
	return NULL;
}
static inline char *find_json_separator(char *str, int *bescape)
{
//	while ( *str != ',' && *str != '\0' && *str != '}' && *str != ']' && *str != ':' && *str != '{' && *str != '[' )
//		str++;
	*bescape = 0;
	unsigned char *p = (unsigned char *)str;
	while ( JSON_SEPARATOR != m_ascii2type[*p] && *p )
	{
		if (*p == '\\')
			*bescape = 1;
		p++;
	}
	if ( *p == ',' || *p == '}' || *p == ']' || *p == ':')
		return (char*)(p-1);
	return NULL;
}
/* 分配一节点空间 */
static json_parser* json_malloc_node(HANDLE *parser, int *parsernum)
{
	if ( *parsernum <= 0 )
	{
		return NULL;
	}
	json_parser *p = (json_parser*)(*parser);
	*parser = (HANDLE)(((uint32_t *)(*parser)) + sizeof(json_parser));
	*parsernum = *parsernum - 1;
	p->keystartpos = NULL;
	p->valuestartpos = NULL;
	p->children = NULL;
	p->brother = NULL;
	return (json_parser*)p;
}
/* {"key":123, "key2":"value", key3:456} */
/* {key:[{city:"beijin"},{city:“上海"}], "key2":{city:"beijin",pepole:1234}, key3:456} */
/* [{city:"beijin"},{city:“上海"}] */
/* string格式为"xxxx"格式 */
/* value格式为:string,number,object, array, true(1), false(0), null */
/* 解析json数组 */
static json_parser* json_parse_array(json_parser *header, char **json, HANDLE *parserbuf, int *parsernum, int recursion_depth )
{
	if (recursion_depth >= 128)
		return NULL;/* 在我们的应用中不应该出现超长的多重json字串 */
	json_parser *node1 = NULL, *node = NULL;
	char *p = *json;
	while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p]) // *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'
		p++;
	if ( *p != '[' )/* 不是json数组类 */
		return NULL;
	p++;
	header->objtype = JSON_ARR_VALUE;	
	while ( *p != ']' )
	{
		while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p]) // *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'
			p++;/* 去掉排版字符 */
		if ( *p == ',' )
		{/* 判段空的属性对只有,创建一空的记录信息 */
			if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
				return NULL;
			node->keylen = 0;
			node->valueendpos = NULL;
			node->objtype = 0;
		}
		else if ( *p == '\"' || *p == '\'')
		{/*数组对象里的string对象*/
			if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
				return NULL;
			int bescape;
			node->valuestartpos = p++;
			if ((node->valueendpos = find_correspond_quotedbl(p, *(p-1), &bescape)) == NULL )
				return NULL;
			p = node->valueendpos+1;
			if (bescape) 
				node->objtype = JSON_STRING_ESC;
			else
				node->objtype = JSON_STRING_VALUE;
		}
		else if ( *p == '{' )
		{/*数组对象里的对象*/
			if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
				return NULL;
			node->valuestartpos = p;
			if ( json_parse_obj(node, &p, parserbuf, parsernum, recursion_depth+1) == NULL )
				return NULL;
			node->valueendpos = p++;
			node->objtype = JSON_OBJ_VALUE;
		}
		else if ( *p == '[' )
		{/*数组对象里的数组*/
			if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
				return NULL;
			node->valuestartpos = p;
			if ( json_parse_array(node, &p, parserbuf, parsernum, recursion_depth+1) == NULL )
				return NULL;
			node->valueendpos = p++;
			node->objtype = JSON_ARR_VALUE;
		}
		else if ( *p == '-' || *p == '+' || ( '0' <= *p && *p <= '9' )   )
		{/*数组对象里的数值*/
			if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
				return NULL;
			node->valuestartpos = p;
			if ((node->valueendpos = find_correspond_integer(p)) == NULL )
				return NULL;
			p = node->valueendpos+1;
			node->objtype = JSON_NUMBER_VALUE;
		}
		else if ( isalpha(*p) )
		{/* 不允许出现关键字节属性值对如a1:value */
			int bescape;
			char *end = find_json_separator(p+1, &bescape);
			int slen;
			if ( end == NULL || *(end+1) == ':' )/* 不允许出现关键字节属性值对 */
				return NULL;
			if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
				return NULL;
			slen = end - p + 1;
			node->valuestartpos = p-1;/* 虚拟一个引号 */
			if (bescape)
				node->objtype = JSON_STRING_ESC;
			else
				node->objtype = JSON_STRING_VALUE;
			node->valueendpos = end+1;
			p = node->valueendpos;/* 虚拟一个引号 */
			if ( slen == 4 && strncasecmp(&node->valuestartpos[1], "true", 4) == 0 )
			{/*bool变量*/
				node->objtype = JSON_BOOL_VALUE ;
				node->valuestartpos = (char*)1;
				node->valueendpos = NULL;
			}
			else if ( slen == 4 && strncasecmp(&node->valuestartpos[1], "null", 4) == 0 )
			{/* null */
				node->objtype = JSON_NULL_VALUE ;
				node->valuestartpos = NULL;
				node->valueendpos = NULL;
			}
			else if ( slen == 5 && strncasecmp(&node->valuestartpos[1], "false", 5) == 0 )
			{/*bool变量*/
				node->objtype = JSON_BOOL_VALUE ;
				node->valuestartpos = (char*)0;
				node->valueendpos = NULL;
			}
		}
		else if ( *p == ']' ) /* 结束 */
			break;
		else /* 非法 */
		{
			return NULL;
		}
		if ( node1 == NULL )
		{/* 加入到json对象的节点中 */		
			header->children = node;
			node1 = node;
		}
		else
		{
			node1->brother = node;
			node1 = node;
		}
		/* 去掉本属性对的结束标示符, */
		while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p]) // *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'
			p++;
		if ( *p != ',' )
			break;
		p++;
	}
	if ( *p != ']' )/* 错误的对象表达式 */
	{
		return NULL;
	}
	*json = p;
	header->valueendpos = p;
	return header;
}
/* 解析json对象,对象的属性必须是string类型:value */
static json_parser* json_parse_obj(json_parser *header, char **json, HANDLE *parserbuf, int *parsernum, int recursion_depth)
{
	if (recursion_depth >= 128)
		return NULL;/* 在我们的应用中不应该出现超长的多重json字串 */
	json_parser *node1 = NULL, *node = NULL;
	char *p = *json;
	char *keyendpos = NULL;
	while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p]) // *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'
		p++;
	if ( *p != '{' )/* 不是json对象类 */
		return NULL;
	p++;
	header->objtype = JSON_OBJ_VALUE;
	while ( *p != '}' )
	{	
		while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p]) // *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'
			p++;/* 去掉排版字符 */
		if ( *p == ',' )
		{/* 判段空的属性对只有,创建一空的记录信息 */
			if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
				return NULL;
			node->keylen = 0;
			node->valueendpos = NULL;
			node->objtype = 0;
		}/*end if *p == ',' */
		else if ( *p == '\"' || *p == '\'' || isalpha(*p) )
		{/* 有可能是string:value对 */
			if ( *p == '\"' || *p == '\'')
			{
				if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
					return NULL;
				int bescape;
				node->keystartpos = p++;
				if ( (keyendpos = find_correspond_quotedbl(p, *(p-1), &bescape)) == NULL )
					return NULL;/* 无法找到string的结束字符返回错误 */
				p = keyendpos + 1;
				node->keylen = (int)(*keyendpos) - (int)(*node->keystartpos) - 1;
				if (bescape)node->keylen = 0-node->keylen;
				/* 属性值分隔符开始 */
				while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p] ) // *p == ' ' || *p == '\t' || *p == '\n' 
					p++;
				if ( *p != ':' )
					return NULL;
			}
			else
			{
				int bescape;
				char *end = find_json_separator(p+1, &bescape);
				if ( end == NULL || *(end+1) != ':' )
					return NULL;
				if ( (node = json_malloc_node(parserbuf, parsernum)) == NULL )
					return NULL;
				node->keystartpos = p-1;
				p = end+1;
				/* 最末为数字或者字母 */	
				while ( !isalnum(*end	) )
					end--;
				node->keylen = (int)*end - (int)(*node->keystartpos);
				if (bescape)node->keylen = 0-node->keylen;
			}
			p++;
			while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p] ) // *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'
				p++;
			/* 属性值分隔符结束 */
			/* json对象对的值域判断 */
			if ( *p == '\"' || *p == '\'')
			{/* 是一string属性 */
				int bescape;
				node->valuestartpos = p++;
				if ((node->valueendpos = find_correspond_quotedbl(p, *(p-1), &bescape)) == NULL )
					return NULL;
				p = node->valueendpos+1;
				if (bescape)
					node->objtype = JSON_STRING_ESC;
				else
					node->objtype = JSON_STRING_VALUE;
			}
			else if ( *p == '{' )
			{/* 对象 */
				node->objtype = JSON_OBJ_VALUE ;
				node->valuestartpos = p;
				if ( json_parse_obj(node, &p, parserbuf, parsernum, recursion_depth+1) == NULL )
					return NULL;/* 发生错误 */
				node->valueendpos = p++;
			}
			else if ( *p == '[' )
			{/* 数组 */
				node->objtype = JSON_ARR_VALUE ;
				node->valuestartpos = p;
				if ( json_parse_array(node, &p, parserbuf, parsernum, recursion_depth+1) == NULL )
					return NULL;/* 发生错误 */
				node->valueendpos = p++;
			}
			else if ( *p == '-' || *p == '+' || ( '0' <= *p && *p <= '9' )  )
			{/* 数字字串 */
				node->valuestartpos = p;
				if ((node->valueendpos = find_correspond_integer(p)) == NULL )
					return NULL;
				p = node->valueendpos+1;
				node->objtype = JSON_NUMBER_VALUE;
			}
			else if ( strncasecmp(p, "false", 5) == 0 || strncasecmp(p, "true", 4) == 0 )
			{/*bool变量*/
				int bv = ( *p == 'f' || *p == 'F' ) ? 0 : 1;
				node->objtype = JSON_BOOL_VALUE ;
				node->valuestartpos = (char *)&bv;
				node->valueendpos = NULL;
				if ( bv )
					p += 4;
				else
					p += 5;
			}
			else if ( strncasecmp(p, "null", 4) == 0 )
			{/* null */
				node->objtype = JSON_NULL_VALUE ;
				node->valuestartpos = NULL;
				node->valueendpos = NULL;
				p +=4;
			}
			else if ( *p == ',' || *p == '}' )
			{/* null */
				node->objtype = JSON_NULL_VALUE ;
				node->valuestartpos = NULL;
				node->valueendpos = NULL;
			}
			else /* 非法 */
				return NULL;

		}/*end else if ( *p == '\"' )属性对象对*/
		else if ( *p == '}' )/*对象结束*/
			break;
		else/* 错误的对象表达式 */
		{
			return NULL;
		}

		if ( node1 == NULL )
		{/* 加入到json对象的节点中 */		
			header->children = node;
			node1 = node;
		}
		else
		{
			node1->brother = node;
			node1 = node;
		}
		/* 去掉本属性对的结束标示符, */
		while ( JSON_TYPESETTING == m_ascii2type[(unsigned char)*p]) // *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r'
			p++;
		if ( *p != ',' )
			break;
		p++;
	}
	if ( *p != '}' )/* 错误的对象表达式 */
	{
		return NULL;
	}
	*json = p;
	header->valueendpos = p;
	return header;
}

/*
 *@brief json解析器初始化接口,初始化解析器,不进行内存分配,使用原始字串,所以其json字串在整个获取属性值对前不能被修改
*
*@param:json const char*原始需要解析的json字串
*	jsonlen int 需要解析的json字串长度,小于等于0时解析整个长度
*	parserbuf HANDLE 解析结果空间起始地址
*	parserlen int 解析结果空间长度,如果地址非空其长度不能小于等于0
*
*@return 解析器的第一个节点位置
*/
HANDLE sw_json_init(const char *json, HANDLE parserbuf, int parserbuflen)
{
	if ( json == NULL || *json == '\0' || parserbuf == NULL || parserbuflen < (int)(sizeof(json_parser)) )
	{
		return NULL;
	}
	int parsernum = parserbuflen / sizeof(json_parser);
	char *p = (char*)json;
	HANDLE ret = NULL;
	while ( *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' )
		p++;
	json_parser *root = json_malloc_node(&parserbuf, &parsernum);
	root->valuestartpos = p;
	if ( *p == '[' )
		ret =  (HANDLE)json_parse_array(root, (char**)&json, &parserbuf, &parsernum, 0);
	else if ( *p == '{' )
		ret =  (HANDLE)json_parse_obj(root, (char**)&json, &parserbuf, &parsernum, 0);
	root->valueendpos = (char *)json;
	return ret;
}

static inline unsigned char Char2Hex(unsigned char c)
{
	if (m_ascii2type[(int)c] == JSON_DIGITAL)
	{
		return c-'0';
	}
	else if (m_ascii2type[(int)c] == JSON_HEX)
	{
		if ('F' < c)
			return c-'A'+0x0a;
		else
			return c-'a'+0x0a;
	}
	return 0xff;
}
static bool transHexCode(unsigned char *src, unsigned char *hex)
{
	unsigned char h0, h1;
	h0 = Char2Hex(src[0]);
	if (h0 == 0xff)return false;
	h1 = Char2Hex(src[1]);
	if (h1 == 0xff)return false;
	*hex++ = (h0 << 4) | h1;
	h0 = Char2Hex(src[2]);
	if (h0 == 0xff)return false;
	h1 = Char2Hex(src[3]);
	if (h1 == 0xff)return false;
	*hex++ = (h0 << 4) | h1;
	return true;
}
static int escape_mem_copy(unsigned char *dest, int dlen, char *src, int slen)
{
	if (dlen < 0) dlen = 0;
	if (slen < 0) dlen = 0;
	int destlen = dlen;
	int srclen = slen;
	while (srclen && destlen)
	{
		if (*src != '\\')
		{
			*dest++ = *src++;
			srclen--;
			destlen--;
		}
		else
		{
			src++;
			if (--srclen == 0)
			{
				*dest++ = *(src-1);
				destlen--;
				break;
			}
			srclen--;
			destlen--;
			if (*src == '"' || *src == '\\' || *src == '/')
			{
				*dest++ = *src++;
			}
			else if (*src == 'b')
			{
				*dest++ = 0x08;//esc
				src++;
			}
			else if (*src == 'f')
			{
				*dest++ = 0x0C;//\f
				src++;
			}
			else if (*src == 'r')
			{
				*dest++ = 0x0D;//\r
				src++;
			}
			else if (*src == 'n')
			{
				*dest++ = 0x0A;
				src++;
			}
			else if (*src == 't')
			{
				*dest++ = 0x09;
				src++;
			}
			else if (*src == '0')
			{
				*dest++ = 0x00;
				src++;
			}
			else if (*src == 'v')
			{
				*dest++ = 0x0B;
				src++;
			}
			else if (*src == 'a')
			{
				*dest++ = 0x07;
				src++;
			}
			else if (*src == 'e')
			{
				*dest++ = 0x1B;
				src++;
			}
			else if (*src == 'u')
			{//\u1234 \uabef
				src++;
				if (srclen < 4)
				{//不是完整的转义直接全copy
					*dest++ = *(src-2);
					src--;
					srclen++;
				}
				else
				{
					unsigned char hex[2];
					if (transHexCode((unsigned char*)src, hex))
					{
						*dest++ = hex[0];
						if (destlen--)
						{
							*dest++ = hex[1];
						}
						src += 4;
						srclen -= 4;
					}
					else
					{
						*dest++ = *(src-2);
						src--;
						srclen++;
					}
				}
			}
			else
			{
				*dest++ = *(src-1);
				srclen++;
			}
		}
	}
	return dlen-destlen;
}
static int key_info_copy(json_parser *p, char *key, int keysize)
{
	if ( key != NULL && 1 <= keysize && p->keystartpos != NULL )
	{
		int klen = p->keylen;
		keysize--;
		if (klen > 0)
		{
			klen = (klen < keysize) ? klen : keysize;
			memmove(key, p->keystartpos+1, klen);
			key[klen] = '\0';
		}
		else
		{
			klen = escape_mem_copy((unsigned char*)key, keysize, p->keystartpos+1, 0-klen);
			key[klen] = '\0';
		}
		return klen;
	}
	return 0;
}
static int attr_info_copy(json_parser *p, char *key, int keysize, unsigned char *attr, int attrsize)
{
	if (key != NULL && 1 <= keysize)
		*key = '\0';
	if (attr != NULL && 1 <= attrsize)
		*attr = '\0';
	if (p == NULL)
		return -1;
	int vlen = 0;
	if ( p->objtype == JSON_BOOL_VALUE )
	{
		key_info_copy(p, key, keysize);
		if ( attr != NULL && 0 < attrsize)
		{
			snprintf((char*)attr, attrsize, "%d", (int)(*p->valuestartpos));
		}
		return 1;
	}
	else if ( p->objtype == JSON_NULL_VALUE )
	{
		key_info_copy(p, key, keysize);
		return 0;
	}
	else if ( p->objtype == JSON_NUMBER_VALUE )
	{
		key_info_copy(p, key, keysize);
		if (attr != NULL && 0 < attrsize && p->valuestartpos != NULL)
		{
			vlen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) + 1;
			attrsize--;
			vlen = (attrsize < vlen ) ? attrsize : vlen;
			memmove(attr, p->valuestartpos, vlen);
			attr[vlen] = '\0';
		}
		return vlen;
	}
	else if ( p->objtype == JSON_STRING_VALUE )
	{
		key_info_copy(p, key, keysize);
		if (attr != NULL && 0 < attrsize && p->valuestartpos != NULL)
		{
			vlen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) - 1;
			attrsize--;
			vlen = (attrsize < vlen ) ? attrsize : vlen;
			memmove(attr, p->valuestartpos+1, vlen);
			attr[vlen] = '\0';
		}
		return vlen;
	}
	else if ( p->objtype == JSON_STRING_ESC)
	{
		key_info_copy(p, key, keysize);
		if (attr != NULL && 0 < attrsize && p->valuestartpos != NULL)
		{//需要转义
			vlen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) - 1;
			attrsize--;
			vlen = escape_mem_copy(attr, attrsize, p->valuestartpos+1, vlen);
			attr[vlen] = '\0';
		}
		return vlen;
	}
	else 
	{/* 数组或者对象 */
		key_info_copy(p, key, keysize);
		if (attr != NULL && 0 < attrsize && p->valuestartpos != NULL)
		{
			vlen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) + 1;
			attrsize--;
			vlen = (attrsize < vlen ) ? attrsize : vlen;
			memmove(attr, p->valuestartpos, vlen);
			attr[vlen] = '\0';
		}
		return vlen;
	}
}
/*
 *@brief 获取关键字key的属性(有可能是对象,数组,string,int,空,错误),
 *	如果node为对象在子节点中查找(方便调用)
 *	如果node为数组返回错
 *	其它在后续兄弟节点中查找
 *
 *@param:key char*关键字名称
 *	keylen int 关键字长度(小于等于0时比较整个关键字)
 *	bstrcase bool 关键字比较是否区别大小写
 *	value uint8_t*关键字属性值地址
 *	valuelen int 属性字串长度
 *	node HANDLE 解析器的节点地址
 *
 *@return -1报错，>= 0 拷贝到目标value的数据长度
 */
int sw_json_get_value(HANDLE node, const char *key, int keylen, bool bstrcase, char *value, int valuelen)
{
	if ( key == NULL || value == NULL || valuelen <= 0 || node == NULL )
	{
		if ( value != NULL && 0 < valuelen)
			value[0] = '\0';
		return -1;
	}
	json_parser *root = (json_parser*)node;
	json_parser *p = NULL;
	keylen = ( keylen <= 0 ) ? (int)strlen(key) : keylen;
	value[0] = '\0';
	if ( keylen == 0 )
		return -1;
	if ( root->objtype == JSON_ARR_VALUE )
		return -1;
	if ( root->objtype == JSON_OBJ_VALUE )/* json对象从子节点获取 */
		p = root->children;
	else 
		p = node;
	char tmpkey[512];
	while ( p != NULL )
	{
		if ( p->keystartpos != NULL )
		{
			int pkeylen = p->keylen;
			char *pkey = p->keystartpos+1;
			if (pkeylen < 0)
			{
				pkeylen = key_info_copy(p, tmpkey, sizeof(tmpkey));
				pkey = &tmpkey[0];
			}
			if ( pkeylen == keylen && 
			     ((!bstrcase && memcmp(key, pkey, keylen) == 0) || 
			     (bstrcase && strncasecmp(key, pkey, keylen) == 0)) )
			{/* 找到了 */
				return attr_info_copy(p, NULL, 0, (unsigned char*)value, valuelen);
			}
		}
		p = p->brother;
	}
	return -1;
}

static int get_attr_from_key(char *attr, int size, const char **key)
{
	if (**key == '\0')
		return 0;
	const char *p = *key;
	int i = 0;
	char *s = attr;
	size--;
	if (*p == '[')//[i]
	{
		p++;
		while ('0' <= *p && *p <= '9' && i < size)
		{
			*s++ = *p++;
			i++;
		}
		if (i >= size )
			return 0;
		if (*p == ']')
		{
			if (*(p+1) == '.' && *(p+2) == '[')//[].[]非法
				return 0;
			*key = p+1;
			*s = '\0';
			return JSON_ARR_VALUE;
		}
		return 0;
	}
	while (*p && *p != '[' && i < size)
	{
		if (*p == '.')
		{
			int c = 0;
			char *t = s;
			while (t != attr && *(t-1) == '\\')
			{
				c++;
				t--;
			}
			if ((c%2) == 0)
			{
				if (*(p+1) == '[')//.[]是非法的
					return 0;
				*key = p;
				*s = '\0';
				return JSON_STRING_VALUE;
			}
			s--;
		}
		*s++ = *p++;
		i++;
	}
	if (i >= size)
		return 0;
	*key = p;
	*s = '\0';
	return JSON_STRING_VALUE;
}
/*
 *@brief 获取字符串表达式的节点,
 {at1:v1,at2:[r1,r2,r3],at3:{at31:v31,at32:v32},at4:[{a1:v1,a2:v2},{a1:v11,a2:v12}]}--->at1=v1, at2=[r1,r2,r3], at2[2]=r3,	at3.at32=v32, at4[1].a2=v12
 [{rat11:rv11,rat12:rv12},{rat21:rv21,rat22:rv22,rat23:[rv231,rv232,rv233]},{rat31:rv31,rat32:rv32}]---->[1].rat23[1]=rv232
 [[rv001,rv002,rv003],[{ra21:rv21,ra22:rv22}],[rv301,{ra31:rv31,ra32:rv32},rv303]]--->[0][1]=rv002, [1][0].ra22=rv22, [2][2]=rv303;
 如果以[i]开头的话node节点必须是数组类型,	[]abc是错误的表达式,[].abc是正确的表达式,**.[]是非法的表达式
 "\\."这里的逗号就转为属性串了, []如果不是数字的话表达式错,'[',']'为特殊符号
 *@param:
 *	node: 开始节点,1:如果[i]开头的话,node必须为数组类型的(非数组中元素),2:如果Node节点是json对象的话，搜索匹配是从其字节点开始，3：否则是Node节点同一级开始匹配
 		如果不清楚是哪种类型最好是从root开始的表达式匹配
 *	expression int json表达式，每个属性长度不要超过128个字节(目前没有那个接口有超过64字节的)
 *	bstrcase bool 关键字比较是否区别大小写
 *	node HANDLE *解析器的地址(其属性值对获取是从其子节点中获取的)
 *
 *@return NULL没有找到或者非法的表达式 非null找到对应的节点
 */
HANDLE sw_json_get_expression_node(HANDLE node, const char *expression, bool bstrcase)
{
	if ( expression == NULL || *expression == '\0' || node == NULL )
	{
		return NULL;
	}
	char attr[512];//我们的应用中不会超过128字节的属性
	json_parser *p = (json_parser*)node;
	int index = 0;
	int objtype;
	bool first = true;
	while (p)
	{
		objtype = get_attr_from_key(attr, sizeof(attr), &expression);
		if (objtype == 0)
		{
			return NULL;
		}
		if (objtype == JSON_ARR_VALUE)
		{
			index = atoi(attr);
			if (p->objtype != JSON_ARR_VALUE)
			{
				return NULL;
			}
			first = false;
			int i = 0;
			p = p->children;
			while (i < index && p)
			{
				p = p->brother;
				i++;
			}
			if (p == NULL)
				return NULL;
			if (*expression == '\0')
				return p;//定位到最后一个位置
			if (*expression == '.')
				expression++;
			else if (*expression != '[')
			{
				return NULL;
			}
		}
		else
		{
			int keylen = strlen(attr);
			if ( p->objtype != JSON_OBJ_VALUE && first == false) {
				return NULL;
			} else if ( p->objtype == JSON_OBJ_VALUE ) {
				p = p->children;
			}
			if (keylen == 0) {
				return NULL;
			}
			first = false;
			while (p != NULL)
			{
				if ( p->keystartpos != NULL )
				{
					char tmpkey[512];
					int pkeylen = p->keylen;
					char *pkey = p->keystartpos+1;
					if (pkeylen < 0)
					{
						pkeylen = key_info_copy(p, tmpkey, sizeof(tmpkey));
						pkey = &tmpkey[0];
					}
					if ( pkeylen == keylen && 
				     ((!bstrcase && memcmp(attr, pkey, keylen) == 0) || 
				     (bstrcase && strncasecmp(attr, pkey, keylen) == 0)) )
				    {
				    	break;
				    }
				}
				p = p->brother;
			}
			if (p == NULL)
				return NULL;
			if (*expression == '\0')
				return p;//定位到最后一个位置
			if (*expression != '[')
				expression++;
		}
	}
	return NULL;
}
/*
 *@brief 获取字符串表达式的节点,
 * 参考sw_json_get_expression_node说明
 *	key uint8_t *关键字缓冲区
 *	keysize int 关键字缓冲区长度
 *	attr uint8_t *属性字串缓冲区
 *	attrsize int 属性字串缓冲区长度
 *@return JSON_NUMBER_VALUE数值,JSON_STRING_VALUE字符串,
 *	JSON_BOOL_VALUE布尔,JSON_NULL_VALUE字串为空
 *	JSON_OBJ_VALUE对象类型,JSON_ARR_VALUE数组类型,
 *	0时为空，-1报错
 */
int sw_json_get_expression_attribute(HANDLE node, const char *expression, bool bstrcase, char *key, int keysize, char *attr, int attrsize)
{
	if (key != NULL && 1 <= keysize)
		*key = '\0';
	if (attr != NULL && 1 <= attrsize)
		*attr = '\0';
	json_parser *p = sw_json_get_expression_node(node, expression, bstrcase);
	if (p == NULL)
		return -1;
	attr_info_copy(p, key, keysize, (unsigned char*)attr, attrsize);
	if (p->objtype == JSON_STRING_ESC)
		return JSON_STRING_VALUE;
	else
		return p->objtype;
}

/*
 *@brief 获取关键字key的属性
  *	如果node为对象在子节点中查找(方便调用)
 *	如果node为数组返回错
 *	其它在后续兄弟节点中查找
 *
 *@param:key char*关键字名称
  *	keylen int 关键字长度(小于等于0时比较整个关键字)
 *	bstrcase bool 关键字比较是否区别大小写
 *	attr uint8_t**关键字属性值地址
 *		如果属性值为字串,数值,特殊类型将其值拷贝到attr中
 *		如果其属性值为数组,对象,修改attr地址为node解析器地址(本解析器的root节点)
 *	valuelen int 属性字串长度
 *	node HANDLE *解析器的地址(其属性值对获取是从其兄弟字串中获取的)
 *
 *@return JSON_NUMBER_VALUE数值,JSON_STRING_VALUE字符串,
 *	JSON_BOOL_VALUE布尔,JSON_NULL_VALUE字串为空
 *	JSON_OBJ_VALUE对象类型,JSON_ARR_VALUE数组类型,
 *	0时为空，-1报错
 */
int sw_json_get_attribute(HANDLE node, char *key, int keylen, bool bstrcase, uint8_t **attr, int attrlen)
{
	if ( key == NULL || attr == NULL || attrlen <= 0 ||  node == NULL )
		return -1;
	json_parser *root = (json_parser*)node;
	json_parser *p = NULL;
	keylen = ( keylen <= 0 ) ? (int)strlen(key) : keylen;
	(*attr)[0] = '\0';
	if ( keylen == 0 )
		return -1;
	if ( root->objtype == JSON_ARR_VALUE )
		return -1;
	if ( root->objtype == JSON_OBJ_VALUE )/* json对象从子节点获取 */
		p = root->children;
	else
		p = root;
	while ( p != NULL )
	{
		if ( p->keystartpos != NULL )
		{
			char tmpkey[512];
			int pkeylen = p->keylen;
			char *pkey = p->keystartpos+1;
			if (pkeylen < 0)
			{
				pkeylen = key_info_copy(p, tmpkey, sizeof(tmpkey));
				pkey = &tmpkey[0];
			}
			if ( keylen == pkeylen && 
			     ((!bstrcase && memcmp(key, pkey, keylen) == 0) || 
			     (bstrcase && strncasecmp(key, pkey, keylen) == 0)) )
			{/* 找到了 */
				if ( p->objtype == JSON_BOOL_VALUE )
				{
					snprintf((char*)*attr, attrlen, "%d", (int)(*p->valuestartpos));
					return JSON_BOOL_VALUE;
				}
				else if ( p->objtype == JSON_NULL_VALUE )
				{
					return JSON_NULL_VALUE;
				}
				if ( p->objtype == JSON_STRING_VALUE )
				{
					int vlen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) - 1;
					attrlen--;
					vlen = (attrlen < vlen ) ? attrlen : vlen;
					memmove(*attr, p->valuestartpos+1, vlen);
					(*attr)[vlen] = '\0';
					return p->objtype;					
				}
				else if ( p->objtype == JSON_STRING_ESC )
				{
					int vlen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) - 1;
					attrlen--;
					vlen = escape_mem_copy(*attr, attrlen, p->valuestartpos+1, vlen);
					(*attr)[vlen] = '\0';
					return JSON_STRING_VALUE;
				}
				else if ( p->objtype == JSON_NUMBER_VALUE )
				{
					int vlen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) + 1;
					attrlen--;
					vlen = (attrlen < vlen ) ? attrlen : vlen;
					memmove(*attr, p->valuestartpos, vlen);
					(*attr)[vlen] = '\0';
					return p->objtype;
				}
				else
				{
					*attr = (uint8_t*)node;
					return p->objtype;
				}
			}
		}
		p = p->brother;
	}
	return -1;	
}

/*
 *@brief 获取node的子节点
 */
HANDLE sw_json_get_children(HANDLE node)
{
	return  ( node != NULL ) ? (HANDLE)(((json_parser*)node)->children) : NULL;
}

/*
 *@brief 获取node的兄弟节点
 */
HANDLE sw_json_get_brother(HANDLE node)
{
	return (node != NULL ) ? (HANDLE)(((json_parser*)node)->brother) : NULL;
}

/*
 *@brief 获取node节点的属性值对
 *
 *@param:key char*关键字(为空不返回关键字)
 *	keylen int 保存关键字长度(要足够长才能保存完整个关键字)
 *	value uint8_t *属性值(为空时不返回属性字串)
 *	valuelen int 属性值长度
 *	node HANDLE 解析器的节点
 *
 *@return JSON_NUMBER_VALUE数值,JSON_STRING_VALUE字符串,
 *	JSON_BOOL_VALUE布尔,JSON_NULL_VALUE字串为空
 *	JSON_OBJ_VALUE对象类型,JSON_ARR_VALUE数组类型,
 *	0时为空，-1报错
 */
int sw_json_get_node_attribute(HANDLE node, char *key, int keylen, uint8_t *attr, int attrlen)
{
	if ( key != NULL && 1 <= keylen )
		*key = '\0';
	if ( attr != NULL && 1 <= attrlen )
		*attr = '\0';
	if ( node == NULL )
		return -1;
	json_parser *p = (json_parser*)node;
	attr_info_copy(p, key, keylen, (unsigned char*)attr, attrlen);
	if (p->objtype == JSON_STRING_ESC)
		return JSON_STRING_VALUE;
	else
		return p->objtype;
}

/*
 *@brief 获取node节点的属性及值长度
 *
 *@param:attrlen int *关键字长度
 *	valuelen int *属性值长度
 *	node HANDLE 解析器的节点
 *
 *@return JSON_NUMBER_VALUE数值,JSON_STRING_VALUE字符串,
 *	JSON_BOOL_VALUE布尔,JSON_NULL_VALUE字串为空
 *	JSON_OBJ_VALUE对象类型,JSON_ARR_VALUE数组类型,
 *	0时为空，-1报错
 */
int sw_json_get_node_attribute_info(HANDLE node, int *attrlen, int *valuelen)
{
	if ( node == NULL )
		return -1;
	json_parser *p = (json_parser*)node;
	int tmpattrlen = 0, tmpvaluelen = 0;
	if (attrlen == NULL)
		attrlen = &tmpattrlen;
	if (valuelen == NULL)
		valuelen = &tmpvaluelen;
	*attrlen = *valuelen = 0;
	int keylen = p->keylen;
	if (keylen < 0)
		keylen = 0-keylen;

	if (p->keystartpos != NULL)
		*attrlen = keylen;
	if ( p->objtype == JSON_BOOL_VALUE )
	{
		char buf[32];
		snprintf(buf, sizeof(buf), "%d", (int)(*p->valuestartpos));
		*attrlen = keylen;
		*valuelen = strlen(buf);
	}
	else if ( p->objtype == JSON_NULL_VALUE )
	{
		*attrlen = keylen;
		*valuelen = 0;
	}
	else
	{
		if (p->valuestartpos != NULL)
			*valuelen = (int)(*p->valueendpos) - (int)(*p->valuestartpos) + 1;
		if (p->objtype == JSON_STRING_ESC)
			return JSON_STRING_VALUE;
		else
			return p->objtype;
	}
	return -1;
}

