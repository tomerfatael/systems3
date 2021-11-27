#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include "message_slot.h"

typedef strucrt FILEDATA {
    int minor, channelId;
} FILEDATA

int minorArr[256] = {0};

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
static int device_open(struct inode* inode, struct file* file) {
    int minor = iminor(inode);
    if(!minorArr[minor]) {
        minorArr[minor] = 1;
        FILEDATA* fileData;
        fileData = kmalloc(sizeof(FILEDATA), GFP_KERNEL);
        if(fileData == NULL) {
            printk("kmalloc function failed", file);
            return 1;
        }
        fileData->minor = minor;
        fileData->channelId = 0;
        file->private_data = (void*)fileData;
    }
    return SUCCESS;
}

static int device_release(struct inode* inode, struct file* file) {
    kfree(file->private_data);
    return SUCCESS;
}

static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    /*error cases*/
    if(file->private_data->channelId == 0) {
        errno = EINVAL;
        return -1;
    }
    if(length == 0 || length > BUF_LEN) {
        errno = EMSGSIZE;
        return -1;
    }
    if(buffer == NULL) { //checkk
        errno = EFAULT;
        return -1;
    }

    /*no errors*/
    for(int i = 0; i < length && i < BUF_LEN; i++) {
        get_user()
    }
}

static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset) {

}