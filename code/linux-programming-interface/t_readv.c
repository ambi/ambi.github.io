#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define STR_SIZE 100

void usage(char *argv[]) {
    fprintf(stderr, "Usage: %s file \n", argv[0]);
    exit(1);
}

int main(int argc, char *argv[]) {
    int fd;
    struct iovec iov[3];
    ssize_t numRead, size = 0;
    char buf1[STR_SIZE], buf2[STR_SIZE], buf3[STR_SIZE];

    if (argc < 2)
        usage(argv);

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open file error");
        return 1;
    }

    size = 0;

    iov[0].iov_base = buf1;
    iov[0].iov_len = STR_SIZE;
    size += iov[0].iov_len;

    iov[1].iov_base = buf2;
    iov[1].iov_len = STR_SIZE;
    size += iov[1].iov_len;

    iov[2].iov_base = buf3;
    iov[2].iov_len = STR_SIZE;
    size += iov[2].iov_len;

    numRead = readv(fd, iov, 3);
    if (numRead == -1) {
        perror("readv file error");
    }

    if (numRead < size) {
        printf("Read fewer bytes than requested\n");
        return 0;
    }

    printf("%.*s%.*s%.*s", STR_SIZE, buf1, STR_SIZE, buf2, STR_SIZE, buf3);
    return 0;
}
