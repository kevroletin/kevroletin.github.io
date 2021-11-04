#include <pty.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CHK(a, msg) if (a < 0) { fprintf(stderr, "%s failed\n", msg); exit(-1); };
#define BUF_SIZE 1024*4
#define STDIN 0
#define STDOUT 1

int max(int a, int b) {
    return a > b ? a : b;
}

struct termios termios_cfg;
struct winsize winsize_cfg;
char slave_name[1024];
char buf[BUF_SIZE];

int copy_loop(int from_1, int to_1, int from_2, int to_2) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    while (1) {
        FD_SET(from_1, &read_fds);
        FD_SET(from_2, &read_fds);

        int ready = select(max(from_1, from_2) + 1, &read_fds, NULL, NULL, NULL);

        if (ready == 0)
            continue;

        if (ready == -1) {
            perror("select()");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(from_1, &read_fds)) {
            int n = read(from_1, buf, BUF_SIZE);
            if (n < 1) {
                fprintf(stderr, "error while reading");
                exit(-1);
            }
            CHK( write(to_1, buf, n), "write" );
            FD_CLR(from_1, &read_fds);
        }
        if (FD_ISSET(from_2, &read_fds)) {
            int n = read(from_2, buf, BUF_SIZE);
            if (n < 1) {
                fprintf(stderr, "error while reading");
                exit(-1);
            }
            CHK( write(to_2, buf, n), "write" );
            FD_CLR(from_2, &read_fds);
        }
    }
}

int main(int argc,  char* argv[]) {
    if (argc < 2) {
        printf("usage: ...\n");
        exit(-1);
    }

    int master, slave;
    CHK( openpty(&master, &slave, slave_name, &termios_cfg, &winsize_cfg), "openpty" );
    fprintf(stderr, "Opened a new tty: %s\n", slave_name);

    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork error\n");
        exit(-1);
    }
    if (pid == 0) {
        CHK( dup2(slave, 0), "dup2" );
        CHK( dup2(slave, 1), "dup2" );
        CHK( dup2(slave, 2), "dup2" );
        CHK( execvp(argv[1], (argv + 1)), "execvp" );
    }

    return copy_loop(STDIN, master,
                     master, STDOUT);
}
