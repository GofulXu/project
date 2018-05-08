#!/bin/sh

#kernel
KERNELDIR ?= /home/goeful/work/kernel
obj-m += ksa.o

VERSION = 1.0
PROJECT_PWD = $(shell pwd)
PROJECT_NAME = higo_test_dec
DIR_APP = /mnt/hgfs/goeful/tftp/hisipro
CROSS_COMPILE = arm-hisiv200-linux-

DIR_INC = ./inc -I./inc/common -I./inc/hi_common -I./inc/higo_common
DIR_BIN = ./bin
DIR_LIB = ./libs -L./libs/lib -L./libs/hilib
DIR_SRC = ./src/sample_dec.c
DIR_SRC += ./src/common/hi_adp_mpi.c ./src/common/hi_adp_data.c ./src/common/hi_adp_demux.c ./src/common/hi_adp_hdmi.c ./src/common/hi_adp_higo.c ./src/common/hi_adp_pvr.c ./src/common/hi_adp_search.c

CFLAGS = -g -Wall -I$(DIR_INC) 
LDFLAGS := -L$(DIR_LIB)
LDFLAGS += -lpthread -lm -lz -lMali -lhi_msp -lhi_vou -lhiflash  -lhi_ecs -lhi_common -ltde -lhipng -ljpeg -lhijpge -lfreetype -lhigoadp -lhigo -ldl

ifeq ($(DIR_SRC),./src)
OBJECTS := $(patsubst %.c,%.o,$(wildcard $(DIR_SRC)/*.c))
else
OBJECTS := $(patsubst %.c,%.o,$(DIR_SRC))
endif

MDEFINE := 

ifneq (${CFG_HI_DAC_CVBS},)
MDEFINE += -DHI_DAC_CVBS=${CFG_HI_DAC_CVBS}
else
MDEFINE += -DHI_DAC_CVBS=3
endif
ifneq (${CFG_HI_DAC_YPBPR_Y},)
MDEFINE += -DHI_DAC_YPBPR_Y=${CFG_HI_DAC_YPBPR_Y}
else
MDEFINE += -DHI_DAC_YPBPR_Y=1
endif
ifneq (${CFG_HI_DAC_YPBPR_PB},)
MDEFINE += -DHI_DAC_YPBPR_PB=${CFG_HI_DAC_YPBPR_PB}
else
MDEFINE += -DHI_DAC_YPBPR_PB=0
endif
ifneq (${CFG_HI_DAC_YPBPR_PR},)
MDEFINE += -DHI_DAC_YPBPR_PR=${CFG_HI_DAC_YPBPR_PR}
else
MDEFINE += -DHI_DAC_YPBPR_PR=2
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
