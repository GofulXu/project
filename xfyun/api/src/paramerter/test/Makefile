BUILD_TARGET_TYPE = exe
BINDIR = .
NAME = showpara
SRC += \
		show_para.c

CFLAGS += \
			-I$(PROJECT_PATH)/module/include					\
			-I$(PROJECT_PATH)/api/include/sqlite3				\
			-I$(PROJECT_PATH)/api/include

DYN_LDS_WITH += \
				-L$(PROJECT_PATH)/api/lib						\
				-L$(PROJECT_PATH)/lib

LINKFLAGS += \
			 -lmodule											\
			 -loperate										\
			 -lapi											\
			 -lpthread											\
			 -lm												\
			 -ldl

-include $(PROJECT_PATH)/build/core.Makefile
