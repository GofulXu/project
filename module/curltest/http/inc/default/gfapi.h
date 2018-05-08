#ifndef __GFAPI_H__
#define __GFAPI_H__


#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include <sys/stat.h>

#ifdef	WIN32
#include "gfapi_win32.h"
#endif

//preales define linux
#ifdef LINUX
#include "gfapi_linux.h"
#endif

#include "gftype.h"
#include "gfdefine.h"
#include "gfevent.h"


#endif /*__GFAPI_H__*/
