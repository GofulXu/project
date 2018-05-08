#!/bin/sh

#kernel
KERNELDIR ?= /home/goeful/work/kernel
obj-m += ksa.o

VERSION = 1.0
PROJECT_PWD = $(shell pwd)
ifeq ($(DBGSER),y)
PROJECT_NAME = gf_server
DIR_SRC = ./src/gf_servermain.c
else 
PROJECT_NAME = gf_client
DIR_SRC = ./src/gf_clientmain.c
endif
DIR_INC = ./inc
DIR_BIN = ./bin
DIR_LIB = ./lib
DIR_APP = /home/goeful/app
CROSS_COMPILE = 
CFLAGS = -g -Wall -I$(DIR_INC) 
LDFLAGS := -L$(DIR_LIB)
LDFLAGS += -lpthread -lm

ifeq ($(DIR_SRC),./src)
OBJECTS := $(patsubst %.c,%.o,$(wildcard $(DIR_SRC)/*.c))
else
OBJECTS := $(patsubst %.c,%.o,$(DIR_SRC))
endif

TIME = Build on $(shell date)
ifeq ($(GMAKETIME),y)
TARGET = $(DIR_BIN)/$(PROJECT_NAME)$(shell TZ=CST date -u "+%Y%m%d_%H%M%S")__$(VERSION)
else
TARGET = $(DIR_BIN)/$(PROJECT_NAME)_$(VERSION)
endif

$(TARGET) : $(OBJECTS)
	$(CROSS_COMPILE)gcc  $^ -o $@ $(CFLAGS) $(LDFLAGS)
	@echo "$(TIME)"

%.o : %.c
	@echo "$(TIME)"
	$(CROSS_COMPILE)gcc -c  $< -o $@ $(CFLAGS)

app:
	$(CROSS_COMPILE)gcc  $(OBJECTS) -o $(DIR_APP)/$(PROJECT_NAME) $(CFLAGS) $(LDFLAGS)
	@echo "$(TIME)"
	

clean:
	@rm -f $(OBJECTS) $(DIR_BIN)/$(PROJECT_NAME)*$(VERSION)

testremove:
	@rm -f $(DIR_SRC)/../src/test.*

cp:
	@cp -f $(DIR_BIN)/$(PROJECT_NAME)*$(VERSION) $(DIR_APP)

driver:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

play:
	$(DIR_BIN)/$(PROJECT_NAME)*$(VERSION)

remove:
	@rm -rf $(PROJECT_PWD)/$(DIR_SRC) $(PROJECT_PWD)/$(DIR_INC) $(PROJECT_PWD)/$(DIR_LIB) $(PROJECT_PWD)/$(DIR_BIN) $(PROJECT_PWD)/doc $(PROJECT_PWD)/readme.txt $(PROJECT_PWD)/Makefile

.PHONY:clean

#common makefile foot
