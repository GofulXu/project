BUILD_TARGET_TYPE = exe
BINDIR = $(PROJECT_PATH)/app/bin
NAME = app
SRC += \
		src/app_shell.c						\
		src/app.c

CFLAGS += \
			-I$(PROJECT_PATH)/module/include					\
			-I$(PROJECT_PATH)/api/include/sqlite3				\
			-I$(PROJECT_PATH)/api/include

DYN_LDS_WITH += \
				-L$(PROJECT_PATH)/api/lib						\
				-L$(PROJECT_PATH)/lib

LINKFLAGS += \
			 -lmodule											\
			 -lapi											\
			 -loperate										\
			 -lpthread											\
			 -lm												\
			 -luuid												\
			 -ldl

-include $(PROJECT_PATH)/build/core.Makefile
