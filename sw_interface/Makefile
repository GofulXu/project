#!/bin/sh

#kernel
KERNELDIR ?= /home/goeful/work/kernel
obj-m += ksa.o

VERSION = 1.0
PROJECT_NAME = goeful
DIR_INC = ./inc
DIR_BIN = ./bin
DIR_LIB = ./lib
DIR_APP = /home/goeful/share/tftpboot
DIR_SRC = ./src/httpclient_test.c
DIR_SRC += ./src/swthrd.c ./src/swmutex.c ./src/swtcp.c ./src/swsignal.c ./src/swudp.c ./src/string.c ./src/swlog.c
DIR_SRC += ./src/swhttp/swhttpclient.c ./src/swhttp/swurl.c ./src/swhttp/http_authentication.c ./src/swhttp/md5.c ./src/swhttp/string_ext.c ./src/swhttp/txtparser.c ./src/swhttp/swhttpserver.c ./src/swhttp/swmem.c ./src/swhttp/swhttpfile.c
TIME = Build on $(shell date)
ifeq ($(GMAKETIME),y)
TARGET = $(DIR_BIN)/$(PROJECT_NAME)$(shell TZ=CST date -u "+%Y%m%d_%H%M%S")__$(VERSION)
else
TARGET = $(DIR_BIN)/$(PROJECT_NAME)_$(VERSION)
endif
#CROSS_COMPILE = arm-hisiv200-linux-
CFLAGS = -g -Wall -I$(DIR_INC) 


LDFLAGS := -L$(DIR_LIB)

LDFLAGS += -lpthread 

OBJECTS := $(patsubst %.c,%.o,$(DIR_SRC))

$(TARGET) : $(OBJECTS)
	$(CROSS_COMPILE)gcc  $^ -o $@ $(CFLAGS) $(LDFLAGS)
	@echo "$(TIME)"

%.o : %.c
	@echo "$(TIME)"
	$(CROSS_COMPILE)gcc -c  $< -o $@ $(CFLAGS)
clean:
	@rm -f $(OBJECTS) $(DIR_BIN)/$(PROJECT_NAME)*$(VERSION)

cp:
	@cp -f $(DIR_BIN)/$(PROJECT_NAME)*$(VERSION) $(DIR_APP)

driver:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

play:
	$(DIR_BIN)/$(PROJECT_NAME)*$(VERSION)

remove:
	@rm -rf $(DIR_INC) $(DIR_BIN) $(DIR_SRC) $(DIR_LIB) $(pwd)doc $(pwd)readme.txt $(pwd)Makefile

.PHONY:clean

#common makefile foot
