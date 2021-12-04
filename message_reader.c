#include "message_slot.h"
#include <string.h>
#include <sys/ioctl.h>
#include<stdlib.h>
#include <stdio.h>
#include <fcntl.h> //check if need
#include <zconf.h>

int main(int argc, char** argv) {
    char* filePath;
    unsigned int channelId;
    long flag;
    int fd;
    char buffer[BUF_LEN];
    
    if(argc != 3){
        perror("invalid input");
        exit(1);
    }
    filePath = argv[1];
    channelId = argv[2];
    flag;
    fd = open(filePath, O_RDONLY);
    if(fd == -1) {
        perror("failed to open file");
        exit(1);
    }
    flag = ioctl(fd, MSG_SLOT_CHANNEL, channelId);
    if(flag != 0) {
        perror("ioctl failed");
        exit(1);
    }
    
    flag = read(fd, buffer, BUF_LEN);
    close(fd);
    if(flag < 0) {
        perror("read failed");
        exit(1);
    }
    return SUCCESS;

}