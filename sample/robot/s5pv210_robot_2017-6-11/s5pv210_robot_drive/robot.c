#include <linux/init.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/uaccess.h>
#include <asm/gpio.h>
#include <plat/gpio-cfg.h>

#define gec210io_MAJOR 100
#define gec210io_MINOR 0
#define gec210io_COUNT 1

//功能选择
#define LN298_ON_FUNCTION			1400
#define LN298_OFF_FUNCTION			1500

//控制动作
#define GO_RUN		100
#define GO_BACK		110
#define TURN_LEFT	120
#define TURN_RIGHT	130
#define GO_STOP		140

enum infrared_gpio_type{
	INFRARED_GPIO_X0,
	INFRARED_GPIO_X1,
	INFRARED_GPIO_X2,
	INFRARED_GPIO_B0,
	INFRARED_GPIO_B1,
}GPIO_HAND;

enum ln298_gpio_type{
	LN298_GPIO_OUT0,
	LN298_GPIO_OUT1,
	LN298_GPIO_OUT2,
	LN298_GPIO_OUT3,
	LN298_GPIO_POWER
}GPIO_LN298_TYPE;


MODULE_LICENSE("GPL v2");

dev_t dev;
u32 gec210io_minor = 0;
//实例化cdev
struct cdev gec210io_cdev;

struct class *dev_class = NULL;
struct device *dev_device = NULL;


unsigned int send_buf[6] = {0};
unsigned long gec210_request_gpio_table[13]=
{
	S5PV210_GPH2(0),
	S5PV210_GPH2(1),
	S5PV210_GPH2(2),
	S5PV210_GPH2(3),
	S5PV210_GPH3(0),
	S5PV210_GPH3(1),
	S5PV210_GPH3(2),
	S5PV210_GPH3(3),
	S5PV210_GPH0(0),
	S5PV210_GPH0(1),
	S5PV210_GPH0(3),
	S5PV210_GPH0(4),
	S5PV210_GPH0(5)
};


/*红外GPIO端口*/
unsigned long gec210_infrared_gpio_table[5]=
{
	S5PV210_GPH2(0),
	S5PV210_GPH2(1),
	S5PV210_GPH2(2),
	S5PV210_GPH2(3),
	S5PV210_GPH3(0),
};

/*电机驱动LN298端口*/
unsigned long gec210_ln298_gpio_table[5]=
{
	S5PV210_GPH0(0),
	S5PV210_GPH0(1),
	S5PV210_GPH0(3),
	S5PV210_GPH0(4),
	S5PV210_GPH0(5)
};



void get_infrared_data(void)
{
	int i = 0;
	//send_buf = {0};
	//配置为输入功能
	gpio_direction_input(gec210_infrared_gpio_table[INFRARED_GPIO_X0]);
	gpio_direction_input(gec210_infrared_gpio_table[INFRARED_GPIO_X1]);
	gpio_direction_input(gec210_infrared_gpio_table[INFRARED_GPIO_X2]);
	gpio_direction_input(gec210_infrared_gpio_table[INFRARED_GPIO_B0]);
	gpio_direction_input(gec210_infrared_gpio_table[INFRARED_GPIO_B1]);
	//获取GPIO的状态
	
	for(i = 1; i < 6;i++)
	{
		send_buf[i] = (gpio_get_value(gec210_infrared_gpio_table[i-1]));
	}
	//上报键值
	send_buf[0] = jiffies_to_usecs(jiffies);
	//copy_to_user(buf, &send_buf, sizeof(send_buf));
	
}


int gec210io_open(struct inode *inode, struct file *filp)
{
	printk("enter gec210io_open!\n");

	gpio_direction_input(gec210_request_gpio_table[0]);
	gpio_direction_input(gec210_request_gpio_table[1]);
	gpio_direction_input(gec210_request_gpio_table[2]);
	gpio_direction_input(gec210_request_gpio_table[3]);
	gpio_direction_input(gec210_request_gpio_table[4]);
	gpio_direction_input(gec210_request_gpio_table[5]);
	gpio_direction_input(gec210_request_gpio_table[6]);
	gpio_direction_input(gec210_request_gpio_table[7]);
	gpio_direction_input(gec210_request_gpio_table[8]);
	gpio_direction_input(gec210_request_gpio_table[9]);
	gpio_direction_input(gec210_request_gpio_table[10]);
	gpio_direction_input(gec210_request_gpio_table[11]);
	gpio_direction_input(gec210_request_gpio_table[12]);
	//初始化ln298
	//将GPIO设置为输出功能,默认输出高电平(灭)
	gpio_direction_output(gec210_ln298_gpio_table[LN298_GPIO_OUT0], 1);
	gpio_direction_output(gec210_ln298_gpio_table[LN298_GPIO_OUT1], 1);
	gpio_direction_output(gec210_ln298_gpio_table[LN298_GPIO_OUT2], 1);
	gpio_direction_output(gec210_ln298_gpio_table[LN298_GPIO_OUT3], 1);
	gpio_direction_output(gec210_ln298_gpio_table[LN298_GPIO_POWER], 1);
	//禁止上下拉
	s3c_gpio_setpull(gec210_ln298_gpio_table[LN298_GPIO_OUT0], S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(gec210_ln298_gpio_table[LN298_GPIO_OUT1], S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(gec210_ln298_gpio_table[LN298_GPIO_OUT2], S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(gec210_ln298_gpio_table[LN298_GPIO_OUT3], S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(gec210_ln298_gpio_table[LN298_GPIO_POWER], S3C_GPIO_PULL_NONE);
	//设置输出值,输出高电平()
	gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],1);
	gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],1);
	gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],1);
	gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],1);
	gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],0);
	return 0;
}

ssize_t gec210io_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
	int ret = 0;
	printk("enter gec210io_read!\n");
	get_infrared_data();
	ret = copy_to_user(buf, &send_buf, sizeof(send_buf));
	return count;
}

ssize_t gec210io_write(struct file *filp, const char __user *buf, size_t count, loff_t *offset)
{
	printk("enter gec210io_write!\n");
	return 0;
}

int gec210io_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long data)
{
	printk("enter gec210io_ioctl!\n");
	switch(cmd&~(1))
	{
		case LN298_ON_FUNCTION:
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],1);
			break;
		case LN298_OFF_FUNCTION:
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],0);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],1);
			break;
			
		case GO_RUN:
			printk("GO_RUN!\n");
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],0);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],0);
			break;
			
		case GO_BACK:
			printk("GO_BACK!\n");
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],0);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],0);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],1);
			break;
		
		case TURN_LEFT:
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],0);
			break;
			
		case TURN_RIGHT:
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],0);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],1);
			break;
			
		case GO_STOP:
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_POWER],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT0],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT1],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT2],1);
			gpio_set_value(gec210_ln298_gpio_table[LN298_GPIO_OUT3],1);
			break;
		
		default:
			return -EINVAL;
	}
	
	return 0;
}

int gec210io_release(struct inode *inode, struct file *filp)
{
	printk("enter gec210io_release!\n");
	gpio_direction_input(gec210_request_gpio_table[0]);
	gpio_direction_input(gec210_request_gpio_table[1]);
	gpio_direction_input(gec210_request_gpio_table[2]);
	gpio_direction_input(gec210_request_gpio_table[3]);
	gpio_direction_input(gec210_request_gpio_table[4]);
	gpio_direction_input(gec210_request_gpio_table[5]);
	gpio_direction_input(gec210_request_gpio_table[6]);
	gpio_direction_input(gec210_request_gpio_table[7]);
	gpio_direction_input(gec210_request_gpio_table[8]);
	gpio_direction_input(gec210_request_gpio_table[9]);
	gpio_direction_input(gec210_request_gpio_table[10]);
	gpio_direction_input(gec210_request_gpio_table[11]);
	gpio_direction_input(gec210_request_gpio_table[12]);
	return 0;
}

struct file_operations gec210io_fops = 
{
	.owner = THIS_MODULE,
	.open = gec210io_open,
	.read = gec210io_read,
	.write = gec210io_write,
	.ioctl = gec210io_ioctl,
	.release = gec210io_release,
};

int __init gec210io_init(void)
{
	int ret = 0;
	if(gec210io_minor)//静态分配
	{
		dev = MKDEV(gec210io_MAJOR,gec210io_MINOR);

		//向内核申请
		ret = register_chrdev_region(dev, gec210io_COUNT, "robot");
	}
	else//动态分配
	{
		ret = alloc_chrdev_region(&dev, gec210io_minor, gec210io_COUNT, "robot");
	}
	
	if(ret<0)
	{
		printk("register_chrdev faigec210io_!\n");
		goto faiure_register_chrdev;
	}

	//初始化cdev
	cdev_init(&gec210io_cdev, &gec210io_fops);
	//向内核注册cdev
	ret = cdev_add(&gec210io_cdev, dev, gec210io_COUNT);
	if(ret<0)
	{
		printk("cdev_add faigec210io_!\n");
		goto failure_cdev_add;
	}

	/*动态创建设备文件*/
	// 1.创建设备类
	//会在/sys/class目录下创建"gec210io_class"为名的文件夹
	dev_class = class_create(THIS_MODULE, "robot_class");
	if(IS_ERR(dev_class))
	{
		ret = PTR_ERR(dev_class);
		goto failure_class_create;
	}
	// 2.创建设备文件
	//会在/dev目录下创建对应的设备文件
	dev_device = device_create(dev_class, NULL, dev, NULL, "robot");
	if(IS_ERR(dev_device))
	{
		ret = PTR_ERR(dev_device);
		goto failure_device_create;
	}

	//申请GPIO资源
	gpio_request(gec210_request_gpio_table[0], "GPH2_0");
	gpio_request(gec210_request_gpio_table[1], "GPH2_1");
	gpio_request(gec210_request_gpio_table[2], "GPH2_2");
	gpio_request(gec210_request_gpio_table[3], "GPH2_3");
	gpio_request(gec210_request_gpio_table[4], "GPH3_0");
	gpio_request(gec210_request_gpio_table[5], "GPH3_1");
	gpio_request(gec210_request_gpio_table[6], "GPH3_2");
	gpio_request(gec210_request_gpio_table[7], "GPH3_3");
	gpio_request(gec210_request_gpio_table[8], "GPH0_0");
	gpio_request(gec210_request_gpio_table[9], "GPH0_1");
	gpio_request(gec210_request_gpio_table[10], "GPH0_3");
	gpio_request(gec210_request_gpio_table[11], "GPH0_4");
	gpio_request(gec210_request_gpio_table[12], "GPH0_5");
	return 0;

	failure_device_create:
		class_destroy(dev_class);
	failure_class_create:
		cdev_del(&gec210io_cdev);
	failure_cdev_add:
		unregister_chrdev_region(dev, gec210io_COUNT);
	faiure_register_chrdev:
		return ret;
}

void __exit gec210io_exit(void)
{
	gpio_direction_input(gec210_request_gpio_table[0]);
	gpio_direction_input(gec210_request_gpio_table[1]);
	gpio_direction_input(gec210_request_gpio_table[2]);
	gpio_direction_input(gec210_request_gpio_table[3]);
	gpio_direction_input(gec210_request_gpio_table[4]);
	gpio_direction_input(gec210_request_gpio_table[5]);
	gpio_direction_input(gec210_request_gpio_table[6]);
	gpio_direction_input(gec210_request_gpio_table[7]);
	gpio_direction_input(gec210_request_gpio_table[8]);
	gpio_direction_input(gec210_request_gpio_table[9]);
	gpio_direction_input(gec210_request_gpio_table[10]);
	gpio_direction_input(gec210_request_gpio_table[11]);
	gpio_direction_input(gec210_request_gpio_table[12]);
	printk("enter gec210io_exit!\n");
	//释放GPIO资源
	gpio_free(gec210_request_gpio_table[0]);
	gpio_free(gec210_request_gpio_table[1]);
	gpio_free(gec210_request_gpio_table[2]);
	gpio_free(gec210_request_gpio_table[3]);
	gpio_free(gec210_request_gpio_table[4]);
	gpio_free(gec210_request_gpio_table[5]);
	gpio_free(gec210_request_gpio_table[6]);
	gpio_free(gec210_request_gpio_table[7]);
	gpio_free(gec210_request_gpio_table[8]);
	gpio_free(gec210_request_gpio_table[9]);
	gpio_free(gec210_request_gpio_table[10]);
	gpio_free(gec210_request_gpio_table[11]);
	gpio_free(gec210_request_gpio_table[12]);
	//注销设备
	device_destroy(dev_class, dev);
	//注销设备类
	class_destroy(dev_class);
	//注销cdev
	cdev_del(&gec210io_cdev);
	//注销设备号
	unregister_chrdev_region(dev, gec210io_COUNT);
}

module_init(gec210io_init);
module_exit(gec210io_exit);
