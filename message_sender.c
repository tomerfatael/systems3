#include "message_slot.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

int main(int argc, char** argv) {
    char *filePath, *message, *strt;
    unsigned long channelId;
    long flag;
    int fd;
    
    if(argc < 4) {
        perror("invalid input");
        exit(1);
    }
    filePath = argv[1];
    channelId = strtol(argv[2], &strt, 10);
    message = argv[3];

    fd = open(filePath, O_WRONLY);
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
    close(fd);
    return SUCCESS;
}
