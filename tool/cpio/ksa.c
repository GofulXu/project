#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/gpio.h>
#include <mach/iomux-mx53.h>
#include <linux/uaccess.h>
#include "ksa.h"

MODULE_LICENSE("GPL v2");

#define BTN_MAJOR 100
#define BTN_MINOR 0
#define BTN_COUNT 1

#define UART3_TX    MX53_PAD_EIM_D24__GPIO3_24
#define UART3_RX    MX53_PAD_EIM_D25__GPIO3_25
#define SD2_DATA0   MX53_PAD_SD2_DATA0__GPIO1_15
#define SD2_DATA1   MX53_PAD_SD2_DATA1__GPIO1_14
#define SD2_DATA2   MX53_PAD_SD2_DATA2__GPIO1_13
#define SD2_DATA3   MX53_PAD_SD2_DATA3__GPIO1_12
#define SD2_CMD     MX53_PAD_SD2_CMD__GPIO1_11
#define SD2_CD      MX53_PAD_EIM_DA11__GPIO3_11

dev_t dev;
u32 btn_minor = 0;
//实例化cdev
struct cdev btn_cdev;

struct class *dev_class = NULL;
struct device *dev_device = NULL;

unsigned long long gsk_arry[8] = {UART3_TX,SD2_CD,SD2_DATA0,SD2_DATA2,UART3_RX,SD2_CMD,SD2_DATA1,SD2_DATA3};

int cdd_open(struct inode *inode, struct file *filp)
{
	if(0 > gpio_request(IOMUX_TO_GPIO(UART3_TX), "UART3_TX")){printk("UART3_TX\n");goto request_false;}
	if(0 > gpio_request(IOMUX_TO_GPIO(SD2_CD), "SD2_CD")){printk("SD2_CD\n");goto request_false;}
	if(0 > gpio_request(IOMUX_TO_GPIO(SD2_DATA0), "SD2_DATA0")){printk("SD2_DATA0\n");goto request_false;}
	if(0 > gpio_request(IOMUX_TO_GPIO(SD2_DATA2), "SD2_DATA2")){printk("SD2_DATA2\n");goto request_false;}
	if(0 > gpio_request(IOMUX_TO_GPIO(UART3_RX), "UART3_RX")){printk("UART_RX\n");goto request_false;}
	if(0 > gpio_request(IOMUX_TO_GPIO(SD2_CMD), "SD2_CMD")){printk("SD2_CMD\n");goto request_false;}
	if(0 > gpio_request(IOMUX_TO_GPIO(SD2_DATA1), "SD2_DATA1")){printk("SD2_DATA1\n");goto request_false;}
	if(0 > gpio_request(IOMUX_TO_GPIO(SD2_DATA3), "SD2_DATA3")){printk("SD2_DATA3\n");goto request_false;}
	
	printk("enter cdd_open!\n");
	return 0;

request_false:
	printk("error gpio request cdd_open!\n");
	return -1;
}



ssize_t cdd_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	unsigned char gsk_value = 0;
	if(0 > gpio_direction_input(IOMUX_TO_GPIO(UART3_TX))){printk("set gpio input error!\n");}
	if(0 > gpio_direction_input(IOMUX_TO_GPIO(SD2_CD))){printk("set gpio input error!\n");}
	gpio_direction_input(IOMUX_TO_GPIO(SD2_DATA0));
	gpio_direction_input(IOMUX_TO_GPIO(SD2_DATA2));
	gpio_direction_input(IOMUX_TO_GPIO(UART3_RX));
	gpio_direction_input(IOMUX_TO_GPIO(SD2_CMD));
	gpio_direction_input(IOMUX_TO_GPIO(SD2_DATA1));
	gpio_direction_input(IOMUX_TO_GPIO(SD2_DATA3));
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_EIM_D24__GPIO3_24)))gsk_value |= (0x01 << 0);
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_EIM_D25__GPIO3_25)))gsk_value |= (0x01 << 1);
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_SD2_DATA0__GPIO1_15)))gsk_value |= (0x01 << 2);
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_SD2_DATA1__GPIO1_14)))gsk_value |= (0x01 << 3);
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_SD2_DATA2__GPIO1_13)))gsk_value |= (0x01 << 4);
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_SD2_DATA3__GPIO1_12)))gsk_value |= (0x01 << 5);
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_SD2_CMD__GPIO1_11)))gsk_value |= (0x01 << 6);
	if(gpio_get_value(IOMUX_TO_GPIO(MX53_PAD_EIM_DA11__GPIO3_11)))gsk_value |= (0x01 << 7);
	printk("gsk_value:%x\n",gsk_value);
	copy_to_user(buf, &gsk_value, 1);
	printk("enter cdd_read!\n");
	return 0;
}

ssize_t cdd_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	printk("enter cdd_write!\n");
	return 0;
}

int cdd_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long data)
{

	switch(cmd)
	{
		case GPIO_ON:
		//将GPIO设置为输出功能,默认输出高电平(灭)
			gpio_direction_output(IOMUX_TO_GPIO(gsk_arry[data]), 1);
			//禁止上下拉
			//s3c_gpio_setpull(gsk_arry[data], S3C_GPIO_PULL_NONE);
			//设置输出值,输出低电平(亮)
			gpio_set_value(IOMUX_TO_GPIO(gsk_arry[data]),0);break;
		case GPIO_OFF:
			gpio_direction_output(IOMUX_TO_GPIO(gsk_arry[data]),1);
			gpio_set_value(IOMUX_TO_GPIO(gsk_arry[data]),1);break;

		//default:break;
	}
				
	printk("enter cdd_ioctl!\n");
	return 0;
}

int cdd_release(struct inode *inode, struct file *filp)
{
	gpio_free(IOMUX_TO_GPIO(UART3_TX));
	gpio_free(IOMUX_TO_GPIO(UART3_RX));
	gpio_free(IOMUX_TO_GPIO(SD2_DATA0));
	gpio_free(IOMUX_TO_GPIO(SD2_DATA1));
	gpio_free(IOMUX_TO_GPIO(SD2_DATA2));
	gpio_free(IOMUX_TO_GPIO(SD2_DATA3));
	gpio_free(IOMUX_TO_GPIO(SD2_CD));
	gpio_free(IOMUX_TO_GPIO(SD2_CMD));
	printk("enter cdd_release!\n");
	return 0;
}

struct file_operations btn_fops = 
{
	.owner = THIS_MODULE,
	.open = cdd_open,
	.read = cdd_read,
	.write = cdd_write,
	.ioctl = cdd_ioctl,
	.release = cdd_release,
};

int __init btn_init(void)
{
	int ret = 0;
		if(btn_minor)//静态分配
	{
		dev = MKDEV(BTN_MAJOR,BTN_MINOR);

		//向内核申请
		ret = register_chrdev_region(dev, BTN_COUNT, "ksa");
	}
	else//动态分配
	{
		ret = alloc_chrdev_region(&dev, btn_minor, BTN_COUNT, "ksa");
	}
	
	if(ret<0)
	{
		printk("register_chrdev failed!\n");
		goto faiure_register_chrdev;
	}

	//初始化cdev
	cdev_init(&btn_cdev, &btn_fops);
	//向内核注册cdev
	ret = cdev_add(&btn_cdev, dev, BTN_COUNT);
	if(ret<0)
	{
		printk("cdev_add failed!\n");
		goto failure_cdev_add;
	}

	/*动态创建设备文件*/
	// 1.创建设备类
	//会在/sys/class目录下创建"btn_class"为名的文件夹
	dev_class = class_create(THIS_MODULE, "ksa_class");
	if(IS_ERR(dev_class))
	{
		ret = PTR_ERR(dev_class);
		goto failure_class_create;
	}
	// 2.创建设备文件
	//会在/dev目录下创建对应的设备文件
	dev_device = device_create(dev_class, NULL, dev, NULL, "ksa_btn");
	if(IS_ERR(dev_device))
	{
		ret = PTR_ERR(dev_device);
		goto failure_device_create;
	}
		
	return 0;

	failure_device_create:
		class_destroy(dev_class);
	failure_class_create:
		cdev_del(&btn_cdev);
	failure_cdev_add:
		unregister_chrdev_region(dev, BTN_COUNT);
	faiure_register_chrdev:
		return ret;
}

void __exit btn_exit(void)
{
	//注销设备
	device_destroy(dev_class, dev);
	//注销设备类
	class_destroy(dev_class);
	//注销cdev
	cdev_del(&btn_cdev);
	//注销设备号
	unregister_chrdev_region(dev, BTN_COUNT);
}

module_init(btn_init);
module_exit(btn_exit);
