#ifndef __SWOS_PRIV_H__
#define __SWOS_PRIV_H__

#include "swlog.h"

#define OS_LOG_DEBUG( format, ...) 	sw_log( LOG_LEVEL_DEBUG, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_INFO( format, ... ) 	sw_log( LOG_LEVEL_INFO, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_WARN( format, ... ) 	sw_log( LOG_LEVEL_WARN, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_ERROR( format, ... ) sw_log( LOG_LEVEL_ERROR, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )
#define OS_LOG_FATAL( format, ... ) sw_log( LOG_LEVEL_FATAL, "OS", __FUNCTION__, __LINE__, format, ##__VA_ARGS__  )

#endif //__SWOS_PRIV_H__

