

#include "message_slot.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

int main(int argc, char *argv[]){

    int fd, ioctl_response, read_response;
    char buffer[BUFFER_SIZE];
    unsigned int channel_id;
    char* response;

    if (argc < 3) {
        perror("not enough args supplied \n");
        exit(1);
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("error during open file \n");
        exit(1);
    }
    channel_id = strtol(argv[2], &response, strlen(argv[2]));

    ioctl_response = ioctl(fd, 1, channel_id);
    if (ioctl_response != 0){
        perror("error during ioctl");
        exit(1);
    }
    read_response = read(fd, buffer, BUFFER_SIZE); /* Read a message from message slot file to the buffer. */
    if (read_response < 0){
        perror("error during read function");
        exit(1);
    } 
    else {
        if (write(STDOUT_FILENO, buffer, read_response) == -1){
            perror("error during write to console");
        }
    }
    close(fd);
    return SUCCESS;
}