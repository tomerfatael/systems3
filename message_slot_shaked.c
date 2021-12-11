#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/slab.h>
#include "message_slot.h"

MODULE_LICENSE("GPL");

struct channel* open_devices[257] = {NULL}; //initial the array
//---------------------- UTILITIES FUNCTIONS -------------------------

void update_channels_list(channel* new_node, unsigned int minor) {
    if (open_devices[minor] == NULL) {              // need to initialize the list
        open_devices[minor] = new_node;
    }
    else{
        channel* head;
        head = open_devices[minor];
        while(head->next != NULL){
            head = head->next;
        }
        head->next = new_node;
    }
}

channel* extract_channel(unsigned int minor, unsigned int channel_id){
    channel* head = NULL;
    head = open_devices[minor];
    if (channel_id == -1) {
        return head;
    }
    while (head != NULL){
        if (head->channel_id == channel_id){
            return head;
        }
        head = head->next;
    }
    return head;
}

//---------------------- DEVICE OPEN -------------------------
static int device_open(struct inode* inode, struct file* file) {
	struct device *new_device;
	unsigned int minor;
    minor = iminor(inode);									    // getting the minor number of device
	new_device = kmalloc(sizeof(struct device), GFP_KERNEL);
	if (new_device == NULL) {
		printk("device file kmalloc failed inside device_open(%p)\n", file);
		return -EINVAL;
	}

    new_device->minor = minor; 								    // store minor number in device file
    new_device->curr_channel_id = 0;                            // initialize channel id
	file->private_data = (void*) new_device;					// store a pointer from the file struct to the device data
	return SUCCESS;
}
//---------------------- DEVICE IOCTL -------------------------
static long device_ioctl( struct   file* file, unsigned int ioctl_command_id, unsigned long ioctl_param){
    device* curr_device;
    channel* curr_channel;
    unsigned int minor;
    if (ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param <= 0) {
        return -1;
    }
    curr_device = (device*) file->private_data;
    minor = curr_device->minor;
    curr_device->curr_channel_id = ioctl_param;
    if (open_devices[minor] == NULL) {                           // if this is the first time we open that device
        open_devices[minor] = kmalloc(sizeof(channel), GFP_KERNEL);
        if (open_devices[minor] == NULL) {
            printk("device file (channel) kmalloc failed inside device_open(%p)\n", file);
            return -EINVAL; // WHY 1 ????
        }
        open_devices[minor]->channel_id = ioctl_param;
        open_devices[minor]->message = NULL;
        open_devices[minor]->length = 0;
        open_devices[minor]->next = NULL;
    }
    else{
        curr_channel = extract_channel(minor, ioctl_param);
        if (curr_channel == NULL){
            curr_channel = (channel*) kmalloc(sizeof(channel), GFP_KERNEL);
            if (curr_channel == NULL) {
                return -EINVAL;
            }
            // -- initialize new channel
            curr_channel->channel_id = ioctl_param;
            curr_channel->message = NULL;
            curr_channel->length = 0;
            curr_channel->next = NULL;
            update_channels_list(curr_channel, minor);
        }
    }
    return SUCCESS;
}
//---------------------- DEVICE READ -------------------------
static ssize_t device_read( struct file* file, char __user* buffer, size_t length, loff_t* offset ) {
  	unsigned int minor, channel_id;
    int response, i;
    channel* curr_channel;
    device* curr_device = (device*)file->private_data;                  // extract current device
    if (curr_device == NULL) {
        return -EINVAL;
    }
	if (curr_device->curr_channel_id == 0 || buffer == NULL) { 		// no channel has been set - impossible to read / invalid buffer
        return -EINVAL;
	}
	channel_id = curr_device->curr_channel_id;			                // extract the current channel id
    minor = curr_device->minor;                                         // extract minor number
    curr_channel = extract_channel(minor, channel_id);                  // extract current channel object
    if (curr_channel == NULL){
        return -EINVAL;
    }
    if (curr_channel->message == NULL){
        return -EWOULDBLOCK;
    }
    if(curr_channel->length > length){
        return -ENOSPC;
    }
    for(i=0; i<curr_channel->length; i++) {
        response = put_user(curr_channel->message[i], buffer+i);
        if (response != 0){
            break;
        }
    }
    if (i != curr_channel->length) {
        return -1; // WHY 0 ????????????????????????????????
    }
    curr_channel->length = i;
    return i;
}

//---------------------- DEVICE WRITE -------------------------
static ssize_t device_write( struct file* file, const char __user* buffer, size_t length, loff_t* offset) {
    unsigned int channel_id;
    unsigned int minor;
    channel* curr_channel;
    int i;
    device* curr_device = (device*) file->private_data;
    printk("INSIDE WRITE, CHANNLE ID = %lu", curr_device->curr_channel_id);
    if (buffer == NULL) {
        return -EINVAL;
    }
    if (length <= 0 || length > BUFFER_SIZE) {
        return -EMSGSIZE;
    }
    channel_id = curr_device->curr_channel_id;			// extract the current channel id
    if (channel_id == 0){
        return -1;
    }
    minor = curr_device->minor;                         // extract minor number
    curr_channel = extract_channel(minor, channel_id);                  // extract current channel object
    if (curr_channel == NULL){
        return -EINVAL;
    }
    curr_channel->message = (char*) kmalloc(sizeof(char)*length, GFP_KERNEL);
    curr_channel->channel_id = channel_id;
    if (curr_channel->message == NULL) {
        return -EINVAL;
    }
    for (i=0; i<length && i < BUFFER_SIZE; i++) {
        get_user(curr_channel->message[i], buffer+i);
    }
    if (i != length) {
        return -1; // WHY 0 ????????????????????????????????
    }
    curr_channel->length = length;
    return i;
}

//---------------------- DEVICE RELEASE -------------------------
static int device_release(struct inode* inode, struct file* file){
    kfree(file->private_data);
    return SUCCESS;
}

//----------------- DEVICE SETUP - File Operations ------------------------
struct file_operations Fops = {
		.owner = THIS_MODULE,
		.read = device_read,
		.write = device_write,
		.open = device_open,
		.release = device_release,
		.unlocked_ioctl = device_ioctl,
};
//----------------------------------------------------------


//----------------- INIT FUNCTION ----------------------------
// Initialize the module - Register the character device
static int __init init(void) {
    int major;
	major = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &Fops);
	if (major < 0) {
		printk(KERN_ERR "%s registration failed for %d\n", DEVICE_FILE_NAME, major);
		return major;
	}
	printk("%s registration succeed for %d\n", DEVICE_FILE_NAME, MAJOR_NUM);
	return SUCCESS;
}

//----------------- CLEANUP FUNCTION -------------------------
static void __exit cleanup(void){
	int i;
	channel* curr_channel;
    channel* tmp;
	for (i = 0; i < 257; i ++ ) {
		curr_channel = open_devices[i];
		while(curr_channel != NULL){
			tmp = curr_channel;
            curr_channel = curr_channel->next;
            kfree(tmp->message);
			kfree(tmp);
		}
		kfree(curr_channel);
	}
	// Unregistered the device, should always succeed
	unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);	
}
//----------------------------------------------------------
module_init(init);
module_exit(cleanup);