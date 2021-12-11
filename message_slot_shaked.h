
#include <linux/ioctl.h>

#define BUFFER_SIZE 128
#define SUCCESS 0
#define DEVICE_RANGE_NAME "message_slot"
#define DEVICE_FILE_NAME "message_slot"
#define MAJOR_NUM 240
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)


// -- STRUCT THAT DESCRIBE SPECIFIC CHANNEL
typedef struct channel {
    unsigned long channel_id;                        // channel id
    char* message;                      // channels message
    unsigned long length;
    struct channel* next;
} channel;

// -- STRUCT THAT DESCRIBE SPECIFIC MESSAGE SLOT DEVICE FILE --
typedef struct device {
    unsigned int minor;                             // minor number
    unsigned long curr_channel_id;
} device;
