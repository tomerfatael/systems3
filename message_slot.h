#ifndef _MESSAGE_SLOT_H_
#define _MESSAGE_SLOT_H_

#include <linux/ioctl.h>

#define MSG_SLOT_CHANNEL _IOW(240,0,unsigned int)
#define SUCCESS 0
#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128
#define MAJOR_NUM 240
#define DEVICE_FILE_NAME "message_slot_dev"

#endif 