#include "message_slot.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

int main(int argc, char** argv) {
    char *filePath, *strt;
    unsigned long channelId;
    long flag;
    int fd;
    char buffer[BUF_LEN];
    
    if(argc < 3){
        perror("invalid input");
        exit(1);
    }
    filePath = argv[1];
    channelId = strtol(argv[2], &strt, 10);

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
    if(flag < 0) {
        perror("read failed");
        exit(1);
    }
    else {
        if (write(STDOUT_FILENO, buffer, flag) == -1){
        perror("writing failed");
        }
    }
    close(fd);
    return SUCCESS;

}