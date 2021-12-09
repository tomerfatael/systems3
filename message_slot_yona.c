#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/radix-tree.h>
#include "message_slot.h"
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
MODULE_LICENSE("GPL");


typedef struct message{
    int size;
    char *buffer;
} my_message;

typedef struct file_struct{
    int minor;
    unsigned long channel_ID;
} my_file;

struct radix_tree_root *minors_arr[256];

//getting the message struct from the private_date field in the
// file structure if the channel contains a message,
//or else returning an empty message
my_message *get_message(struct file* file, size_t length){
    unsigned long channel_ID = ((my_file *)file->private_data)->channel_ID;
    int minor = ((my_file *)file->private_data)->minor;
    my_message *message;
    struct radix_tree_root *root = minors_arr[minor];

    if (channel_ID == 0){
        return NULL;
    }
    //checking if the channel already contains a message
    //allocating memory for the message if not
    message = (my_message*)radix_tree_lookup(root , channel_ID);
    if (message == NULL){
        message = kcalloc(1, sizeof(struct message), GFP_KERNEL);
        if (message==NULL) {
            return NULL;
        }
        message->size = 0;
        radix_tree_insert(root , channel_ID , message);
    }


    return message;
}



static int device_open(struct inode* inode, struct file* file){
    my_file *file_ptr;
    int minor;
    file_ptr = kmalloc(sizeof(my_file), GFP_KERNEL);
    if (file_ptr ==NULL){
        return 1;
    }
    file_ptr->minor = iminor(inode); file_ptr->channel_ID = 0; // initializing file fields
    minor = file_ptr->minor;

    //using the private data field to store the file
    file->private_data = (void *)file_ptr;

    //if its a new file- allocating a new tree root for it
    if (minors_arr[minor] == NULL){
        minors_arr[minor] = kcalloc(1,sizeof(struct radix_tree_root), GFP_KERNEL);
        if (minors_arr[minor] ==NULL){
            return 1;
        }
        INIT_RADIX_TREE(minors_arr[minor], GFP_KERNEL);
    }
    return 0;
}

//connecting between the channel id and the file
static long device_ioctl(struct file* file , unsigned int command , unsigned long channel_id){
    if ((command != MSG_SLOT_CHANNEL) || (channel_id == 0) ) {
        return -EINVAL;
    }
    ((my_file *)file->private_data)->channel_ID = channel_id;
    return 0;
}

//reading the message that was found in get message
static ssize_t device_read(struct file* file, char *buffer, size_t length , loff_t* offset){
    my_message *message = get_message(file,length);
    int i;
    //error checks
    if ((message == NULL) || (file == NULL) || (buffer == NULL)){
        return -EINVAL;
    }
    if (length < message->size){
        return -ENOSPC;
    }
    if (message->size == 0){
        return -EWOULDBLOCK;
    }
    // writing the message to the given buffer
    for (i = 0;( i < message->size) && (i < BUF_LEN); ++i){
        put_user(message->buffer[i], &buffer[i]);
    }
    return (i == message->size)? i : -EINVAL;
}


static ssize_t device_write(struct file* file, const char *buffer, size_t length, loff_t* offset){
    int i;
    my_message *message = get_message(file,length);

    //setting the length needed for the buffer
    message->buffer = kmalloc(length* sizeof(char), GFP_KERNEL);


    if ((length > BUF_LEN) || (length == 0)){

        return -EMSGSIZE;
    }

    if ((buffer == NULL) || (file == NULL) || (message == NULL) || (message->buffer == NULL)){

        return -EINVAL;
    }

    for (i = 0; i < length; ++i){
        get_user(message->buffer[i], &buffer[i]);
    }

    message->size = (i == length)? i: 0;
    return i;
}
void free_radix_tree(struct radix_tree_root* root){
    struct radix_tree_iter it;
    void **node;
    radix_tree_for_each_slot(node, root, &it, 0){
        kfree(*node);
    }
}

static int device_release(struct inode* inode, struct file* file){
    my_file *file_ptr = (my_file *)file->private_data;
    kfree(file_ptr);
    return 0;
}

struct file_operations Fops =
        {
                .owner	  = THIS_MODULE,
                .read           = device_read,
                .write          = device_write,
                .open           = device_open,
                .unlocked_ioctl = device_ioctl,
                .release        = device_release,

                };

static int __init init_p(void){
    int reg_char_device = register_chrdev(MAJOR_NUM, MESSAGE_SLOT, &Fops);
    if (reg_char_device < 0){
        printk(KERN_ERR "module initialization failed\n");
        return -1;
    }
    return 0;
}

static void __exit cleanup_p(void){
    int i=0;
    unregister_chrdev(MAJOR_NUM, MESSAGE_SLOT);
    for (i = 0; i < 256; i++){
        if (minors_arr[i] != NULL){
            free_radix_tree(minors_arr[i]);
        }
    }
}

module_init(init_p);
module_exit(cleanup_p);