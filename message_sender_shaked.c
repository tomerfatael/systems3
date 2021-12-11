#include "message_slot.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
int main(int argc, char** argv){
    // validation of given arguments
    if (argc < 4){
        perror("not enough args supplied \n");
        exit(1);
    }
    int fd, ioctl_response, write_response;
    char* response;
    char* file_path = argv[1];
    unsigned int channel_id = strtol(argv[2], &response, 10);
    char* buffer = argv[3];
    // first we need to open the desire file
    fd = open(file_path, O_WRONLY);
    if (fd < 0) {
        perror("error during open file \n");
        exit(1);
    }
    // set desire channel id
    ioctl_response = ioctl(fd, MSG_SLOT_CHANNEL, channel_id);
    if (ioctl_response != 0) {
        perror("error during ioctl");
        exit(1);
    }
    write_response = write(fd, buffer, strlen(buffer));
    if (write_response != strlen(buffer)){
        perror("error during write function");
        exit(1);
    }
    close(fd);
    return SUCCESS;
}
