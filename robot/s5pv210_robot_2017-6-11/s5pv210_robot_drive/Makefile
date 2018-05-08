KERNELDIR ?= /home/transplant/gec210-kernel-2.6.35.7

obj-m		+=		robot.o

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) INSTALL_MOD_PATH=/home/student/workdir/rootfs modules_install

clean:
	rm -rf *.o *.ko *.mod* *.sy* *.ord* .*cmd .tmp*

