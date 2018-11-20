BUILD_TARGET_TYPE = static
BINDIR = $(PROJECT_PATH)/api/lib
NAME = api
SRC += \
		base/gffifo.c									\
		base/gficonv.c									\
		base/gflog.c									\
		base/gfmutex.c									\
		base/gfsignal.c									\
		base/gfthrd.c									\
		base/string.c									\
		base/sysclient/sysclient.c						\
		base/syscmd/syscmd.c							\
		parse/json/cJSON.c								\
		network/gftcp.c									\
		network/gfudp.c									\
		network/gftcpclient.c							\
		network/gftcpserver.c							\
		network/gfudpclient.c							\
		network/gfmulticast.c							\
		network/gfftpclientcurl.c						\
		network/gftftpclientcurl.c						\
		shell/gfshell.c									\
		paramerter/gfparamerter.c						\
		uart/gfcom.c									\
		uart/gfserial.c					

CFLAGS += \
			-I$(PROJECT_PATH)/module/include			\
			-I$(PROJECT_PATH)/api/include				

DYN_LDS_WITH += \
				-L$(PROJECT_PATH)/api/lib				\
				-L$(PROJECT_PATH)/lib

LINKFLAGS += 

-include $(PROJECT_PATH)/build/core.Makefile
