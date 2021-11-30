#include "message_slot.h"

int main(int argc, char** argv) {
    if(argc != 4) {
        perror("invalid input");
        exit(1);
    }
    char* filePath = argv[1];
    unsigned int channelId = atoi(argv[2]);
    char* message = argv[3];
    long flag;

    int fd = open(filePath, O_WRONLY);
    if(fd == -1) {
        perror("failed to open file");
        exit(1);
    }
    flag = ioctl(fd, MSG_SLOT_CHANNEL, channelId);
    if(flag != 0){
        perror("ioctl failed");
        exit(1);
    }
    flag = write(fd, message, strlen(message));
    if(flag != strlen(message)){
        perror("write failed");
        exit(1);
    }
    return SUCCESS;
}
