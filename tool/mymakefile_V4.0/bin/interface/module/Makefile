BUILD_TARGET_TYPE = static
BINDIR = $(PROJECT_PATH)/lib
NAME = module
SRC += \
		src/gftime/gftime.c 						\
		src/gfreportlog/gfreportlog.c 				\
		src/gfcom/gfcom_sample.c 					\
		src/gfcom/gfnetwork_sample.c 				\
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
