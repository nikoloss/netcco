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
    //inet_addr把xxx.xxx.xxx.xx的字符串直接变成整形（网络字节序）
    d.old_addr = inet_addr(argv[1]); 
    d.old_port = htons(atoi(argv[2]));
    d.new_addr = inet_addr(argv[3]);
    d.new_port = htons(atoi(argv[4]));
    printf("old ip:%d\n", d.old_addr);
    printf("old port:%d\n", d.old_port);
    printf("new ip:%d\n", d.new_addr);
    printf("new port:%d\n", d.new_port);
    write(fd, &d, sizeof(struct nc_distort));
    close(fd);
    return EXIT_SUCCESS;
}