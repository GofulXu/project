#ifndef __GFOS_PRIV_H__
#define __GFOS_PRIV_H__

#include "gflog.h"

#define OS_LOG_DEBUG( format, ...) 	gf_log( LOG_LEVEL_DEBUG, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_INFO( format, ... ) 	gf_log( LOG_LEVEL_INFO, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_WARN( format, ... ) 	gf_log( LOG_LEVEL_WARN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_ERROR( format, ... ) gf_log( LOG_LEVEL_ERROR, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_FATAL( format, ... ) gf_log( LOG_LEVEL_FATAL, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )

#endif //__GFOS_PRIV_H__

