KERNELDIR ?= /home/json/work/goeful/kernel
obj-m += ksa.o
TARGET = test
CC = arm-fsl-linux-gnueabi-gcc
#OBJECTS := $(patsubst %.c,%.o,$(wildcard *.c))
OBJS = test.c
dirver:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=/home/json/Goeful/application modules_install
test:
	$(CC) -o $(TARGET) $(OBJS)
cpy:
	@cp $(TARGET) ksa.ko /mnt/hgfs/vmSource/Goeful/tftp -f
clean:
	@rm -rf $(TARGET) *.o *.ko *.mod *.sy *.ord *.cmd .tmp*
	
