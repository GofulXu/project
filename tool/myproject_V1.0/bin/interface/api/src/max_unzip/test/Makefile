BUILD_TARGET_TYPE = exe
BINDIR = .
NAME = test_zip
SRC += \
		test_zip.c

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
			 -lm

CFLAGS += -DZTASK

-include $(PROJECT_PATH)/build/core.Makefile
