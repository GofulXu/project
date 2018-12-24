#!/bin/sh

#kernel
KERNELDIR ?= /home/goeful/work/kernel
obj-m += ksa.o

VERSION = 3.0
PROJECT_PWD = $(shell pwd)
PROJECT_NAME = mymake
DIR_APP = /home/goeful/app
CROSS_COMPILE = 
DIR_BIN = ./bin

CFLAGS += -Wall -I./inc/default \
		  -I./inc
LDFLAGS += -L./lib
LDFLAGS += -lpthread -lm -ldl
MDEFINE += -DLINUX

OBJECTS += $(patsubst %.c,%.o,$(wildcard ./src/gfapi/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ./src/*.c))

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
	$(CROSS_COMPILE)gcc -c  $< -o $@ $(CFLAGS) $(MDEFINE)

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
