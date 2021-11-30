#include "message_slot.h"

int main(int argc, char** argv) {
    if(argc != 3){
        perror("invalid input");
        exit(1);
    }
    char* filePath = argv[1];
    unsigned int channelId = argv[2];
    long flag;
    int fd = open(filePath, O_RDONLY);
    if(fd == -1) {
        perror("failed to open file");
        exit(1)''
    }
    flag = ioctl(fd, MSG_SLOT_CHANNEL, channelId);
    if(flag != 0) {
        perror("ioctl failed");
        exit(1);
    }
    

}