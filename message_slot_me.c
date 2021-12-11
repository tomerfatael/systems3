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
    unsigned long channelId;
    char* message;
    struct CHANNEL *next;
} CHANNEL;

typedef struct DEVICE {
    int minor;
    unsigned long curChannel;
} DEVICE;

CHANNEL* devicesMinorArr[256] = {NULL};

/*channels linked list functions*/

/*free all allocated memory for channels LL*/
void freeChannelsLL(CHANNEL* head) { 
    CHANNEL *node, *next;
    node = head;
    while(node != NULL) {
        next = node->next;
        if (node->message != NULL){
        kfree(node->message);}
        kfree(node);
        node = next;
    }
}

/*add new channel*/
CHANNEL* addChannelToLL(int minor, unsigned long channelId) {
    CHANNEL *channel, *head;
    channel = kmalloc(sizeof(CHANNEL), GFP_KERNEL);
    if(channel == NULL) {
        return NULL;
    }
    channel->channelId = channelId;
    channel->length = 0;

    head = devicesMinorArr[minor];

    while(head->next != NULL) {
        head = head->next;
    }
    head->next = channel;
    return channel;
}

/*find channel by id*/
CHANNEL* findChannel(int minor, unsigned long channelId) {
    CHANNEL *head;
    head = devicesMinorArr[minor];
    while(head != NULL && head->channelId != channelId) {
        head = head->next;
    }
    return head;
}

/**device functions**/
/*device open*/
static int device_open(struct inode* inode, struct file* file) { //todo and understand set curChannel to 0
    int minor;
    DEVICE* device;
    minor = iminor(inode);
    device = kmalloc(sizeof(DEVICE), GFP_KERNEL);
    /*allocation failed*/
    if(device == NULL) {
        return -EINVAL;
    }
    device->minor = minor;
    device->curChannel = 0;
    if(devicesMinorArr[minor] == NULL) {
        devicesMinorArr[minor] = kmalloc(sizeof(CHANNEL), GFP_KERNEL);
        if(devicesMinorArr[minor] == NULL) return 1;
        devicesMinorArr[minor]->length = 0;
        devicesMinorArr[minor]->channelId = 0;
        devicesMinorArr[minor]->message = NULL;
        devicesMinorArr[minor]->next = NULL;

    }
    file->private_data = (void*)device;
    return SUCCESS;
}

/*device release*/
static int device_release(struct inode* inode, struct file* file) { 
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
    if(device == NULL){
    return -EINVAL;
    }
    if(device->curChannel == 0 || buffer == NULL) {
    return -EINVAL;
    }
    if(length == 0 || length > BUF_LEN) {
            return -EMSGSIZE;
        }

    channel = findChannel(device->minor, device->curChannel);
    if(channel == NULL) {
        channel = addChannelToLL(device->minor, device->curChannel);
        if(channel == NULL) {
            return -EINVAL;
        }
    }
    channel->message = kmalloc(sizeof(char)*length, GFP_KERNEL);
    if(channel->message == NULL) {
        return -EINVAL;
    }

    /*no errors*/
    for(i = 0; i < length && i < BUF_LEN; i++) {
        get_user(channel->message[i], &buffer[i]);
    }

    if(length == i) {
        channel->length = length;
        return length;
    }
    else{ /*error has occured while trying to write data*/ 
        return 0; 
    }
}

/*device read*/
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {
    DEVICE* device;
    CHANNEL* channel;
    int i;
    device = (DEVICE*)file->private_data;
    if(device == NULL){
    return -EINVAL;
    }
    /*there is no channel in the provided device*/
    if(device->curChannel == 0 || buffer == NULL) {
        return -EINVAL;
    }
    channel = findChannel(device->minor, device->curChannel);
    if(channel == NULL) {
    return -EINVAL;
    }
    /*there is no message in the channel*/
    //TODO: make sure length is updated correctly!
    if(channel->message == NULL) {
        return -EWOULDBLOCK;
    }
    /*the provided buffer is too small*/
    if(length < channel->length) {
        return -ENOSPC;
    }

    for(i = 0; i < channel->length; i++) {
        put_user(channel->message[i], &buffer[i]);  
    }

    if(channel->length != i) {
        return -EINVAL; 
    }
    channel->length = i;
    return channel->length;
}

/*device ioctl*/
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
    ((DEVICE*)file->private_data)->curChannel = ioctl_param;
    return SUCCESS;
}

/**device setup**/
struct file_operations Fops = {
  .owner	  = THIS_MODULE, 
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
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
           freeChannelsLL(devicesMinorArr[i]);
       }
   }
    unregister_chrdev(240, DEVICE_RANGE_NAME);
}

module_init(simple_init);
module_exit(simple_cleanup);