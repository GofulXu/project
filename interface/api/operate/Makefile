BUILD_TARGET_TYPE = static
BINDIR = $(PROJECT_PATH)/api/lib
NAME = operate
SRC += \
		sqlite3.c			\
		GfOperate.c



CFLAGS += \
			-I$(PROJECT_PATH)/module/include	\
			-I$(PROJECT_PATH)/api/include/sqlite3		\
			-I$(PROJECT_PATH)/api/include

DYN_LDS_WITH += \
				-L$(PROJECT_PATH)/api/lib \
				-L$(PROJECT_PATH)/lib


LINKFLAGS += \
			 -lapi 						

-include $(PROJECT_PATH)/build/core.Makefile
