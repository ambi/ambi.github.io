#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <syslog.h>

#include "daemonize.h"

#define BUF_SIZE 500

int inet_bind(const char *service, int type, socklen_t *addrlen) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sfd, optval, s;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;
  hints.ai_socktype = type;
  hints.ai_family = AF_UNSPEC; /* IPv4, IPv6 に対応 */
  hints.ai_flags = AI_PASSIVE; /* ワイルドカードアドレス */

  s = getaddrinfo(NULL, service, &hints, &result);
  if (s != 0) return -1;

  /* ソケットにバインド可能なアドレス構造を検索する */
  optval = 1;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1) continue;

    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) break; /* 成功 */
    close(sfd);                                             /* 失敗 */
  }

  if (rp != NULL && addrlen != NULL) *addrlen = rp->ai_addrlen;
  freeaddrinfo(result);

  return (rp == NULL) ? -1 : sfd;
}

int main(int argc, char *argv[]) {
  int sfd;
  socklen_t addrlen;
  struct sockaddr_storage claddr;
  char buf[BUF_SIZE];

  if (become_daemon() == -1) {
    perror("become_daemon error");
    exit(1);
  }

  sfd = inet_bind("echo", SOCK_DGRAM, &addrlen);
  if (sfd == -1) {
    syslog(LOG_ERR, "Could not create server socket (%s)", strerror(errno));
    exit(1);
  }

  for (;;) {
    socklen_t len = sizeof(struct sockaddr_storage);
    ssize_t numread = recvfrom(sfd, buf, BUF_SIZE, 0, &claddr, &len);
    if (numread == -1) {
      syslog(LOG_ERR, "recvfrom error");
      exit(1);
    }
    if (sendto(sfd, buf, numread, 0, &claddr, len) != numread) {
      syslog(LOG_WARNING, "error echoing response (%s)", strerror(errno));
    }
  }
}
