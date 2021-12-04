#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include "message_slot.h" 
#include <linux/kernel.h>   
#include <linux/module.h>   
#include <linux/fs.h>       
#include <linux/uaccess.h>  
#include <linux/string.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

typedef struct CHANNEL {
    int length;
    unsigned int channelId;
    char message[BUF_LEN];
    struct CHANNEL *next;
} CHANNEL;

typedef struct DEVICE {
    int minor, numOfChannels;
    unsigned int curChannel;
    CHANNEL *head;
} DEVICE;


DEVICE* devicesMinorArr[256] = {NULL};

/*channels linked list functions*/

/*free all allocated memory for channels LL*/
void freeChannelsLL(CHANNEL* head) { //toCheck
    CHANNEL *node, *next;
    node = head;
    while(node != NULL) {
        next = node->next;
        kfree(node);
        node = next;
    }
}

/*add new channel*/
int addChannelToLL(DEVICE* device, unsigned int channelId) {
    CHANNEL *channel, *head;
    channel = kmalloc(sizeof(CHANNEL), GFP_KERNEL);
    if(channel == NULL) {
        return 1;
    }
    channel->channelId = channelId;
    channel->length = 0;

    head = device->head;

    if(head == NULL) {
        device->head = channel;
        return SUCCESS;
    }

    while(head->next != NULL) {
        head = head->next;
    }
    head->next = channel;
    return SUCCESS;
}

/*find channel by id*/
CHANNEL* findChannel(DEVICE* device, unsigned int channelId) {
    CHANNEL* channel;
    channel = device->head;
    while(channel != NULL && channel->channelId != channelId) {
        channel = channel->next;
    }
    return channel;
}

/**device functions**/
/*device open*/
static int device_open(struct inode* inode, struct file* file) { //todo and understand set curChannel to 0
    int minor;
    DEVICE* device;
    minor = iminor(inode);
    if(devicesMinorArr[minor] == NULL) {
        device = (DEVICE*) kmalloc(sizeof(DEVICE), GFP_KERNEL);
        /*allocation fail*/
        if(device == NULL) { 
            return 1; //check if needed to return errno
        }
    }
}

/*device release*/
static int device_release(struct inode* inode, struct file* file) { //dont need to free all allocated memory for device and channels?
    kfree(file->private_data);
    return SUCCESS;
}

/*device write*/
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    DEVICE* device;
    CHANNEL* channel;
    int i;
    device = (DEVICE*)file->private_data;
    /*error cases*/
    if(device->curChannel == 0) {
        return -EINVAL;
    }
    if(length == 0 || length > BUF_LEN) {
        return -EMSGSIZE;
    }
    if(buffer == NULL) { //checkk
        return -EFAULT;
    }

    channel = findChannel(device, device->curChannel);
    /*no errors*/
    for(i = 0; i < length && i < BUF_LEN; i++) {
        get_user(channel->message[i], &buffer[i]);
    }

    if(length == i) {
        channel->length = length;
        return length;
    }
    else{ /*error has occured while trying to write data*/ //cehckkkkkkkkkkkkkkkkkkkkkkkkkkk
        return -ENOMEM; //checlllllllll
    }
}

/*device read*/
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {
    DEVICE* device;
    CHANNEL* channel;
    int i;
    device = (DEVICE*)file->private_data;
    /*there is no channel in the provided device*/
    if(device->curChannel == 0) {
        return -EINVAL;
    }
    channel = findChannel(device, device->curChannel);
    /*there is no message in the channel*/
    if(channel->length == 0) {
        return -EWOULDBLOCK;
    }
    /*the provided buffer is too small*/
    if(length < channel->length) {
        return -ENOSPC;
    }

    for(i = 0; i < channel->length && i < BUF_LEN; i++) {
        put_user(channel->message[i], &buffer[i]);  
    }

    if(length != i) {
        return -ENOMEM; //alsoo check
    }
    return length;
}

/*device ioctl*/
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
    DEVICE* device;
    CHANNEL* channel;
    int flag;
    if(ioctl_param == 0 || ioctl_command_id != MSG_SLOT_CHANNEL){
        return -EINVAL;
    }
    
    device = (DEVICE*)file->private_data;
    channel = findChannel(device, ioctl_param);
    if(channel == NULL) { /*channel not exist, need to add*/
        flag = addChannelToLL(device, ioctl_param);
        if(flag == 1) {
            return -ENOMEM; //memory allocation in addChannelToLL faild
        }
    }
    device->curChannel = ioctl_param;
    return SUCCESS;
}

/**device setup**/
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .release        = device_release,
};

/*device init*/
static int __init simple_init(void) {

    int registerSuccess;
    //Register driver capabilities. Obtain major num
    registerSuccess = register_chrdev(240, DEVICE_RANGE_NAME, &Fops);
    // Negative values signify an error
    if(registerSuccess < 0) {
    printk(KERN_ALERT "%s registraion failed for  %d\n",DEVICE_FILE_NAME, 240);
    return registerSuccess;
    }

    printk( "Registeration is successful. The major device number is %d.\n", 240);
    return 0;
}

/*device release*/
static void __exit simple_cleanup(void) { //need to clean LL or in device release?
    int i;
    for(i = 0; i < 256; i++) {
        if(devicesMinorArr[i] != NULL) {
            freeChannelsLL(devicesMinorArr[i]->head);
            //kfree(devicesMinorArr[i]); ///checkkkkkkkkkkkk if needed
        }
    }
    unregister_chrdev(240, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);