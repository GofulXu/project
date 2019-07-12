#!/bin/sh

#kernel
KERNELDIR ?= /home/goeful/work/kernel
obj-m += ksa.o

VERSION = 1.0
PROJECT_PWD = $(shell pwd)
PROJECT_NAME = myproject
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

install:
	@sudo rm -f /bin/$(PROJECT_NAME)
	@sudo ln -s $(PROJECT_PWD)/$(DIR_BIN)/$(PROJECT_NAME)_$(VERSION) /bin/$(PROJECT_NAME)

clean:
	@rm -f $(OBJECTS) $(DIR_BIN)/$(PROJECT_NAME)*$(VERSION)

cp:
	@cp -f $(DIR_BIN)/$(PROJECT_NAME)_$(VERSION) $(DIR_APP)

driver:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

play:
	$(DIR_BIN)/$(PROJECT_NAME)_$(VERSION)

.PHONY:clean

#common makefile foot
