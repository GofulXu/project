#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8775a687, "module_layout" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x98082893, "__copy_to_user" },
	{ 0x6c8d5ae8, "__gpio_get_value" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0xfe990052, "gpio_free" },
	{ 0xbdfd54f9, "device_create" },
	{ 0xa9c31eca, "__class_create" },
	{ 0xa4436d95, "cdev_add" },
	{ 0xc9f03065, "cdev_init" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0xea147363, "printk" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xc1c1a973, "cdev_del" },
	{ 0x310839ba, "class_destroy" },
	{ 0x9750f693, "device_destroy" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

