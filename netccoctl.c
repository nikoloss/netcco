#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "nc_def.h"

static char proc_file[256];
static struct nc_distort d;

int main(int argc, char** argv){
    sprintf(proc_file, "/proc/%s", procfs_name);
    int fd = open(proc_file, O_WRONLY);
    if (fd<0) {
        perror("open");
        return EXIT_FAILURE;
    }
    if (argc<5) {
        printf("%s [old_ip] [old_port] [new_ip] [new_port]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    d.old_addr = inet_addr(argv[1]);
    d.old_port = htons(atoi(argv[2]));
    d.new_addr = inet_addr(argv[3]);
    d.new_port = htons(atoi(argv[4]));
    write(fd, &d, sizeof(struct nc_distort));
    close(fd);
    return EXIT_SUCCESS;
}