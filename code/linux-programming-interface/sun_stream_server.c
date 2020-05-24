#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define BACKLOG 5
#define BUF_SIZE 100
#define SOCK_PATH "/tmp/sun_stream"

int main(int argc, char* argv[]) {
  int listenfd;
  struct sockaddr_un addr;
  ssize_t numRead;
  char buf[BUF_SIZE];

  listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (listenfd == -1) {
    perror("socket error");
    exit(1);
  }

  if (remove(SOCK_PATH) == -1 && errno != ENOENT) {
    perror("remove error");
    exit(1);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (bind(listenfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("bind error");
    exit(1);
  }

  if (listen(listenfd, BACKLOG) == -1) {
    perror("listen error");
    exit(1);
  }

  for (;;) {
    int connfd = accept(listenfd, NULL, NULL);
    if (connfd == -1) {
      perror("accept error");
      exit(1);
    }

    while ((numRead = read(connfd, buf, BUF_SIZE)) > 0)
      if (write(STDOUT_FILENO, buf, numRead) != numRead) {
        perror("write error");
        exit(1);
      }

    if (numRead == -1) {
      perror("read error");
      exit(1);
    }

    if (close(connfd) == -1) {
      perror("close error");
      exit(1);
    }
  }

  return 0;
}