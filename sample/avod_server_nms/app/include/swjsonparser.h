#ifndef __SW_JSON_PARSER_H__
#define __SW_JSON_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* 定义json属性值的类型 */
#define JSON_NUMBER_VALUE	1	/* 数值型,只有在对象里才有关键字,在数组属性中没关键字 */
#define JSON_STRING_VALUE	2	/* 字符串类型(要判断转义符),只有在对象里才有关键字,在数组属性中没关键字 */
#define JSON_BOOL_VALUE		3	/* bool变量,只有在对象里才有关键字,在数组属性中没关键字 */
#define JSON_NULL_VALUE		4	/* null变量,只有在对象里才有关键字,在数组属性中没关键字 */
#define JSON_OBJ_VALUE		5	/* 对象类型{} */
#define JSON_ARR_VALUE		6	/* 数组类型[] */
#define JSON_STRING_ESC		7	/* 需要进行转义的JSON_STRING_VALUE类型 */
/**************************************************************************************
对于{\"audio_track_list\":[{\"PID\" : \"33\", \"language_code\":\"chi\",,,},{\"PID\":\"34\",\"language_code\":\"eng\"},,]}的json字串
节点树为              
    root({[]})
    |子节点
    node1(audio_track_lis)
    |子节点
    node21({})				兄弟					node22({})---xxx
    |子节点										|子节点
    node311(PID)--兄弟--node312(language_code)--兄弟--node313,314,315(空)             		node321(PID)---xxx
要提取language_code为eng的对象先解析sw_json_init
1：后获取sw_json_get_attribute获取audio_track_list的json解析器节点修改后的attr地址，用此获取sw_json_get_children第一个孩子的下一个兄弟sw_json_get_brother
的json解析器节点,用此解析器作为sw_json_get_value的解析器获取到language_code的属性值
2：后获取sw_json_get_attribute获取audio_track_list的json解析器节点地址，
**************************************************************************************/

/*
 *@brief json解析器初始化接口,初始化解析器,不进行内存分配,使用原始字串,所以其json字串在整个获取属性值对前不能被修改
*
*@param:json const char*原始需要解析的json字串
*	jsonlen int 需要解析的json字串长度,小于等于0时解析整个长度
*	parser HANDLE 解析结果空间起始地址
*	parserlen int 解析结果空间长度,如果地址非空其长度不能小于等于0, 32xN+56
*
*@return 解析器的第一个节点位置
*/
HANDLE sw_json_init(const char *json, HANDLE parser, int parserlen);

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
 *	node HANDLE *解析器的地址(其属性值对获取是从其子节点中获取的)
 *
 *@return -1报错，>= 0 拷贝到目标value的数据长度
 */
int sw_json_get_value(HANDLE node, const char *key, int keylen, bool bstrcase, char *value, int valuelen);

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
HANDLE sw_json_get_expression_node(HANDLE node, const char *expression, bool bstrcase);
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
int sw_json_get_expression_attribute(HANDLE node, const char *expression, bool bstrcase, char *key, int keysize, char *attr, int attrsize);

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
 *		如果其属性值为数组,对象,修改attr地址为parser解析器地址(本解析器的root节点)
 *	valuelen int 属性字串长度
 *	node HANDLE *解析器的地址(其属性值对获取是从其兄弟字串中获取的)
 *
 *@return JSON_NUMBER_VALUE数值,JSON_STRING_VALUE字符串,
 *	JSON_BOOL_VALUE布尔,JSON_NULL_VALUE字串为空
 *	JSON_OBJ_VALUE对象类型,JSON_ARR_VALUE数组类型,
 *	0时为空，-1报错
 */
int sw_json_get_attribute(HANDLE node, char *key, int keylen, bool bstrcase, uint8_t **attr, int attrlen);

/*
 *@brief 获取parser的子节点
 */
HANDLE sw_json_get_children(HANDLE node);

/*
 *@brief 获取parser的兄弟节点
 */
HANDLE sw_json_get_brother(HANDLE node);

/*
 *@brief 获取parser节点的属性值对
 *
 *@param:key char*关键字(为空不返回关键字)
 *	keylen int 保存关键字长度(要足够长才能保存完整个关键字)
 *	value uint8_t *属性值(为空时不返回属性字串)
 *	valuelen int 属性值长度
 *	parser HANDLE 解析器的节点
 *
 *@return JSON_NUMBER_VALUE数值,JSON_STRING_VALUE字符串,
 *	JSON_BOOL_VALUE布尔,JSON_NULL_VALUE字串为空
 *	JSON_OBJ_VALUE对象类型,JSON_ARR_VALUE数组类型,
 *	0时为空，-1报错
 */
int sw_json_get_node_attribute(HANDLE node, char *key, int keylen, uint8_t *attr, int attrlen);

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
int sw_json_get_node_attribute_info(HANDLE node, int *attrlen, int *valuelen);

#ifdef __cplusplus
}
#endif

#endif /* __SW_JSON_PARSER_H__ */
