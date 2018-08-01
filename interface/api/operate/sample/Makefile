BUILD_TARGET_TYPE = exe
BINDIR = $(PROJECT_PATH)/api/operate/sample
NAME = paramctldb
SRC +=	paramctl_db.c

LINKFLAGS += \
			 -lapi 						\
			 -loperate					\
			 -lpthread						\
			 -lm							\
			 -ldl

CFLAGS += \
			-I$(PROJECT_PATH)/module/include	\
			-I$(PROJECT_PATH)/api/include/sqlite3		\
			-I$(PROJECT_PATH)/api/include

DYN_LDS_WITH += \
				-L$(PROJECT_PATH)/api/lib \
				-L$(PROJECT_PATH)/lib

-include $(PROJECT_PATH)/build/core.Makefile
