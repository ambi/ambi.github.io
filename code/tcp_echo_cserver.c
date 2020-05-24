#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <syslog.h>

#include "daemonize.h"

#define BUF_SIZE 4096

int inet_listen(const char *service, int backlog) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, opt, s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_UNSPEC; /* IPv4, IPv6 に対応 */
  hints.ai_flags = AI_PASSIVE; /* ワイルドカードアドレス */

  s = getaddrinfo(NULL, service, &hints, &result);
  if (s != 0) return -1;

  /* ソケットにバインド可能なアドレス構造を検索する */
  opt = 1;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1) continue;

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
      close(sfd);
      freeaddrinfo(result);
      return -1;
    }

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) break; /* 成功 */
    close(sfd);                                             /* 失敗 */
  }

  if (rp != NULL && listen(sfd, backlog) == -1) {
    freeaddrinfo(result);
    return -1;
  }

  freeaddrinfo(result);
  return (rp == NULL) ? -1 : sfd;
}

static void handle_request(int cfd) {
  char buf[BUF_SIZE];
  ssize_t numread;

  while ((numread = read(cfd, buf, BUF_SIZE)) > 0) {
    if (write(cfd, buf, numread) != numread) {
      syslog(LOG_ERR, "failure in write (%s)", strerror(errno));
      exit(1);
    }

    if (numread == -1) {
      syslog(LOG_ERR, "failure in read (%s)", strerror(errno));
      exit(1);
    }
  }
}

int main(int argc, char *argv[]) {
  int lfd;

  if (become_daemon() == -1) {
    perror("become_daemon error");
    exit(1);
  }

  lfd = inet_listen("echo", 10);
  if (lfd == -1) {
    syslog(LOG_ERR, "COuld not create server socket (%s)", strerror(errno));
    exit(1);
  }

  for (;;) {
    int cfd = accept(lfd, NULL, NULL);
    if (cfd == -1) {
      syslog(LOG_ERR, "failure in accpet (%s)", strerror(errno));
      exit(1);
    }

    switch (fork()) {
      case -1:
        syslog(LOG_ERR, "Could not create child (%s)", strerror(errno));
        close(cfd);
        break;
      case 0:
        close(lfd);
        handle_request(cfd);
        _exit(0);
      default:
        close(cfd);
        break;
    }
  }
}
