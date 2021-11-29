#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include "message_slot.h" //check if this is really the only include neededs

typedef struct DEVICE {
    int minor, numOfChannels;
    CHANNEL *head;
} DEVICE;

typedef struct CHANNEL {
    int channelId, length;
    char message[BUF_LEN];
    CHANNEL *next;
} CHANNEL;

DEVICE* devicesMinorArr[256] = {NULL};

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
static void __exit simple_cleanup(void) {
  unregister_chrdev(240, DEVICE_RANGE_NAME);
}

/**device functions**/
/*device open*/
static int device_open(struct inode* inode, struct file* file) { //todo and understand
    int minor = iminor(inode);
    if(devicesMinorArr[minor] == NULL) {
        DEVICE* device = (DEVICE*) kmalloc(sizeof(DEVICE), GFP_KERNEL);
        /*allocation fail*/
        if(device == NULL) { 
            return 1; //check if needed to return errno
        }
        
    }
    
}

/*device release*/
static int device_release(struct inode* inode, struct file* file) {
    kfree(file->private_data);
    return SUCCESS;
}

/*device write*/
static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    /*error cases*/
    if(file->private_data->channelId == 0) {
        return -EINVAL;
    }
    if(length == 0 || length > BUF_LEN) {
        return -EMSGSIZE;
    }
    if(buffer == NULL) { //checkk
        return -EFAULT;
    }

    /*no errors*/


    for(int i = 0; i < length && i < BUF_LEN; i++) {
        get_user()
    }
}

/*device read*/
static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {

}

/*device ioctl*/
static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
    if(ioctl_param == 0 || ioctl_command_id != MSG_SLOT_CHANNEL){
        return -EINVAL;
    }
    file->private_data->channelId = ioctl_param;
    return 0;
}