#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUF_SIZE 1024*4

#ifndef ARRAY_SIZE
# define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define CHK(a) if (a < 0) { fprintf(stderr, "%s:%d failed\n", __FILE__, __LINE__); exit(-1); };

struct termios termios_cfg;
struct winsize winsize_cfg;

char cmd[PATH_MAX + 100];
char slave_name[PATH_MAX];
char buf[BUF_SIZE];

void exit_on_sigchld(int sig) {
    system("stty sane");
    exit(0);
}

/* A loop to copy data between filehandles:
 * from_1 -> to_1
 * from_2 -> to_2
 */
int copy_loop(int from_1, int to_1, int from_2, int to_2) {
    enum {
        POLLFD_1,
        POLLFD_2
    };

    struct pollfd pfd[] = {
        [POLLFD_1] = { .fd = from_1,  .events = POLLIN | POLLERR | POLLHUP },
        [POLLFD_2] = { .fd = from_2,  .events = POLLIN | POLLERR | POLLHUP },
    };

    int to_fd[] = {
        [POLLFD_1] = to_1,
        [POLLFD_2] = to_2,
    };

    while (1) {
        size_t i, ret;

        ret = poll(pfd, ARRAY_SIZE(pfd), -1);

        if (ret < 0) {
            if (errno == EAGAIN)
                continue;
            exit( -errno );
        }

        /* timeout */
        if (ret == 0)
            exit( -1 );

        for (i = 0; i < ARRAY_SIZE(pfd); i++) {
            if (pfd[i].revents == 0)
                continue;

            if (pfd[i].revents & POLLIN) {
                int n = read(pfd[i].fd, buf, sizeof(buf));
                if (n < 1) {
                    fprintf(stderr, "error while reading");
                    exit(-1);
                }
                CHK( write(to_fd[i], buf, n) );
            }

            if (pfd[i].revents & POLLHUP || (pfd[i].revents & POLLNVAL)) {
                fprintf(stderr, "eof");
                exit(-1);
            }
        }
    }
}

int main(int argc,  char* argv[]) {
    int master, slave, pid;

    if (argc < 2) {
        printf("usage: ...\n");
        exit(-1);
    }

    CHK( openpty(&master, &slave, slave_name, &termios_cfg, &winsize_cfg) );
    fprintf(stderr, "Opened a new tty: %s\r\n", slave_name);

    sprintf(cmd, "stty sane -F %s", slave_name);
    system(cmd);
    system("stty raw -isig -echo drain");

    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork error\n");
        exit(-1);
    }

    if (pid == 0) {
        CHK( dup2(slave, STDIN_FILENO) );
        CHK( dup2(slave, STDOUT_FILENO) );
        CHK( dup2(slave, STDERR_FILENO) );
        CHK( close(slave) );
        CHK( close(master) );

        CHK( execvp(argv[1], (argv + 1)) );
    }

    signal(SIGCHLD, exit_on_sigchld);

    return copy_loop(STDIN_FILENO, master,
                     master, STDOUT_FILENO);
}
