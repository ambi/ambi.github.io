#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int inputFd, outputFd;
    ssize_t numRead, numWritten;
    char buf[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s old-file new-file\n", argv[0]);
        return 1;
    }

    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1) {
        perror("open reading file error");
        return 1;
    }
    outputFd = open(argv[2], O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (outputFd == -1) {
        perror("open writing file error");
        return 1;
    }

    while ((numRead = read(inputFd, buf, BUF_SIZE)) > 0) {
        numWritten = write(outputFd, buf, numRead);
        if (numWritten == -1) {
            perror("open error");
            return 1;
        }
        if (numWritten != numRead) {
            fprintf(stderr, "coudn't write whole buffer\n");
            return 1;
        }
    }
    if (numRead == -1) {
        perror("read error");
        return 1;
    }

    if (close(inputFd) == -1) {
        perror("close reading file error");
        return 1;
    }

    if (close(outputFd) == -1) {
        perror("close writing file error");
        return 1;
    }

    return 0;
}