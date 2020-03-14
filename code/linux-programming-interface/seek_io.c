#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage(char *argv[]) {
    fprintf(stderr, "Usage: %s file {r<length>|R<length>|w<string>|s<offset>}...\n", argv[0]);
    exit(1);
}

void r(int fd, char *arg, int mode) {
    ssize_t numRead;
    size_t len = atol(arg);
    char *buf = malloc(len);
    if (buf == NULL) {
        perror("malloc buffer error");
        exit(1);
    }

    numRead = read(fd, buf, len);
    if (numRead == -1) {
        perror("read error");
        exit(1);
    }

    if (numRead == 0) {
        printf("%c%s: end-of-file\n", mode, arg);
    } else {
        printf("%c%s: ", mode, arg);
        for (int j = 0; j < numRead; j++)
            if (mode == 'r')
                printf("%c", isprint((unsigned char)buf[j]) ? buf[j] : '?');
            else
                printf("%02x ", (unsigned int)buf[j]);
        printf("\n");
    }

    free(buf);
}

void w(int fd, char *arg) {
    ssize_t numWritten = write(fd, arg, strlen(arg));

    if (numWritten == -1) {
        perror("write error");
        exit(1);
    }
    printf("w%s: wrote %ld bytes\n", arg, (long)numWritten);
}

void s(int fd, char *arg) {
    off_t offset = atol(arg);

    if (lseek(fd, offset, SEEK_SET) == -1) {
        perror("lseek error");
        exit(1);
    }
    printf("s%s: seek succeeded\n", arg);
}

int main(int argc, char *argv[]) {
    int fd, ap;

    if (argc < 3)
        usage(argv);

    fd = open(argv[1], O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    if (fd == -1) {
        perror("open file error");
        return 1;
    }

    for (ap = 2; ap < argc; ap++) {
        switch (argv[ap][0]) {
        case 'r':
        case 'R':
            r(fd, &argv[ap][1], argv[ap][0]);
            break;
        case 'w':
            w(fd, &argv[ap][1]);
            break;
        case 's':
            s(fd, &argv[ap][1]);
            break;
        default:
            usage(argv);
        }
    }

    // No need to close fd.
    return 0;
}