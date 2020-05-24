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
  int sockfd;
  struct sockaddr_un addr;
  ssize_t numRead;
  char buf[BUF_SIZE];

  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket error");
    exit(1);
  }

  memset(&addr, 0, sizeof(struct sockaddr_un));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

  if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) ==
      -1) {
    perror("connect error");
    exit(1);
  }

  while ((numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0)
    if (write(sockfd, buf, numRead) != numRead) {
      perror("write error");
      exit(1);
    }

  if (numRead == -1) {
    perror("read error");
    exit(1);
  }

  return 0;
}