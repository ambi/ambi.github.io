#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage(char *argv[]) {
  fprintf(stderr,
          "Usage: %s file {r<length>|R<length>|w<string>|s<offset>}...\n",
          argv[0]);
  exit(1);
}

int main(int argc, char *argv[]) {
  int pfd[2];
  char buf[1000];
  int dummy;

  if (argc < 2) usage(argv);

  setbuf(stdout, NULL);

  printf("Parent started\n");

  if (pipe(pfd) == -1) {
    perror("lseek error");
    exit(1);
  }

  for (int j = 1; j < argc; j++) {
    switch (fork()) {
      case -1:
        snprintf(buf, 1000, "fork %d error", j);
        perror(buf);
        exit(1);

      case 0:
        if (close(pfd[0]) == -1) {
          perror("close error");
          exit(1);
        }

        sleep(atoi(argv[j]));
        printf("Child %d (PID=%ld) closing pipe\n", j, (long)getpid());

        if (close(pfd[1] == -1)) {
          perror("close error");
          exit(1);
        }
        _exit(0);

      default:
        break;
    }
  }

  if (close(pfd[1]) == -1) {
    perror("close error");
    exit(1);
  }

  if (read(pfd[0], &dummy, 1) != 0) {
    fprintf(stderr, "parent didn't get EOF");
    exit(1);
  }
  printf("Parent ready to go\n");

  return 0;
}