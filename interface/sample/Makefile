BUILD_TARGET_TYPE = exe
BINDIR = $(PROJECT_PATH)/sample
NAME = gfbroadjniservice
SRC += \
	   gfbroadjniservice.c
#		test_service_send.c
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
			 -ldl

-include $(PROJECT_PATH)/build/core.Makefile
