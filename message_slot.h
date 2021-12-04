#ifndef _MESSAGE_SLOT_H_
#define _MESSAGE_SLOT_H_

#include <linux/kernel.h>   
#include <linux/module.h>   
#include <linux/fs.h>       
#include <linux/uaccess.h>  
#include <linux/string.h>
#include <linux/slab.h>
#include <errno.h> 
#include <stdio.h>
#include <stdlib.h>
  

MODULE_LICENSE("GPL");

#define SUCCESS 0
#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128
#define DEVICE_FILE_NAME "message_slot_dev"

#endif 