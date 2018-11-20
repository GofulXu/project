BUILD_TARGET_TYPE = exe
BINDIR = $(PROJECT_PATH)/api/src/xfyun/bin
NAME = iat_record_sample
SRC += \
	    linuxrec.c												\
		speech_recognizer.c										\
		iat_record_sample.c

CFLAGS += \
			-I$(PROJECT_PATH)/module/include					\
			-I$(PROJECT_PATH)/api/include/sqlite3				\
			-I$(PROJECT_PATH)/api/include/xfyun					\
			-I$(PROJECT_PATH)/api/include

DYN_LDS_WITH += \
				-L$(PROJECT_PATH)/api/lib						\
				-L$(PROJECT_PATH)/api/lib/xfyun/x64					\
				-L$(PROJECT_PATH)/lib

LINKFLAGS += \
			 -lmodule											\
			 -lapi												\
			 -loperate											\
			 -lmsc												\
			 -lasound											\
			 -lpthread											\
			 -lm												\
			 -ldl												\
			 -lrt

-include $(PROJECT_PATH)/build/core.Makefile
