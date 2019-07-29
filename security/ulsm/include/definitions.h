/**
 * @file
 *
 * Incorporates the module definitions
 */
#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <linux/module.h>

#define MODULE_NAME "ulsm"
		MODULE_AUTHOR("Argus");
		MODULE_DESCRIPTION("Userspace Linux Security Module");
		MODULE_LICENSE("GPL");
		MODULE_VERSION("1.0");

#define NETLINK_USER 31
#define SUCCESS 0
#define print(format, args...) printk( KERN_INFO MODULE_NAME ": " format "\n", ##args)

extern wait_queue_head_t wait_queue_123;



#endif /* DEFINITIONS_H */
