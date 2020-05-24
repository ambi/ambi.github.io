#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int become_daemon(void) {
  int maxfd, fd;

  /* 親プロセスを終了して、子プロセスは実行を継続する。
   * その結果、デーモンは init の子プロセスになる。 */
  switch (fork()) {
    case -1:
      return -1;
    case 0:
      break;
    default:
      _exit(0);
  }

  /* 新規セッションを開始する。 */
  if (setsid() == -1) return -1;

  /* 再度 fork させて、端末デバイスをオープンさせられないようにする。 */
  switch (fork()) {
    case -1:
      return -1;
    case 0:
      break;
    default:
      exit(0);
  }

  /* umask をクリアする。 */
  umask(0);
  /* ルートディレクトリに移動する。 */
  chdir("/");

  /* 親プロセスから受け継いだファイルディスクリプタをすべてクローズする。 */
  maxfd = sysconf(_SC_OPEN_MAX);
  if (maxfd == -1) maxfd = 8192;
  for (fd = 0; fd < maxfd; fd++) close(fd);

  /* 標準入力・出力・エラーを /dev/null に対応させる。 */
  fd = open("/dev/null", O_RDWR);
  if (fd != STDIN_FILENO) return -1;
  if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO) return -1;
  if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO) return -1;
}
