BUILD_TARGET_TYPE = static
BINDIR = $(PROJECT_PATH)/lib
NAME = module
SRC += \
		src/gftime/gftime.c 						\
		src/gftime/gfadjsystime.c					\
		src/gfservice/gfservice.c					\
		src/gfservice/gfservice_send.c					\
		src/gfservice/gfhashfunc.c					\
		src/gfreportlog/gfreportlog.c 				\
		src/GfP2P/GfP2PService.c 				\
		src/gfmodule.c 	

CFLAGS += \
			-I$(PROJECT_PATH)/module/include							\
			-I$(PROJECT_PATH)/api/include								\
			-I$(PROJECT_PATH)/api/include/sqlite3

DYN_LDS_WITH += \
				-L$(PROJECT_PATH)/api/lib								\
				-L$(PROJECT_PATH)/lib

LINKFLAGS += \
			 -loperate												\
			 -lapi

-include $(PROJECT_PATH)/build/core.Makefile
