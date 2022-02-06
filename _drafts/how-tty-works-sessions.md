---
layout: post
title:  "How terminal works. Part 4: pty, sessions"
categories: terminal
---

![]("/assets/how-terminal-works/img/terminal.jpg")

* TOC
{:toc}

We will finish discussion of tty features by writing a simplified version of
[script](https://man7.org/linux/man-pages/man1/script.1.html) utility. A few
simplifications will help us fit most of the code in this post. The task will
help us to:
* create and configure a pseudo-terminal;
* discuss process groups and sessions in more detail.

We will use the C programming language because it's a widely understood language
and also because it's a primary system programming language on Linux.

## script utility

We've already used the `script` tool in the previous post to capture the output
of the `vim` command. Its main purpose is to intercept communication between
`xterm` and a requested command (in our case `vim`) and to dump the captured
output into a file. Let's discuss how `script` captures data. Recall the usual
scheme of communication between `xterm` and `vim`:

```
(1)           (2)         (3)
xterm <-----> tty <-----> vim
      m           s
```

`script` creates one more pseudo-terminal (4), configures tty (2) into raw mode,
and then enters a loop of continuously sending data and signals back and forth
between tty(2) and tty(4). That way:
* `vim` can configure tty(4) to use tty's input/output processing capabilities;
* `script` can capture the output of tty(4) and send it into tty(2) so that
  xterm displays it;
* tty(2) doesn't alter the output of `script` because it's in raw mode; hence,
  data captured by `script` from tty(4) is exactly the same data that `xterm`
  receives and visualizes.

```
(1)           (2)         (3)            (4)         (5)
xterm <-----> tty <-----> script <-----> tty <-----> vim
      m           s              m           s
```

## Creating pty

`man ptmx` tells how to create a new pty: it's a sequence of `open`, `grantpt`,
`unlockpt` library calls. To make life a few lines shorter, we'll use a helper
function `openpty` from `glibc`.

Our current task is to:

1. create a new `pty`;
2. configure the current `pty` to raw mode, configure the newly created `pty` to
   a sensible default configuration;
3. fork a process to run a requested command; make it use newly created `pty`;
4. make sure we terminate when a child process terminates;
5. enter a loop to copy data between a new pty(4) and a pty(2) associated with stdin.

For simplicity we ignore signal handling, we use assertions for errors handling
and we give no [special handling for
eof](https://github.com/karelzak/util-linux/blob/master/lib/pty-session.c#L299).

```c

// ...

#define CHK(a) if (a < 0) { fprintf(stderr, "%s:%d failed\n", __FILE__, __LINE__); exit(-1); };

int main(int argc,  char* argv[]) {
    int master, slave, pid;

    if (argc < 2) {
        printf("usage: ...\n");
        exit(-1);
    }

    // step 1
    CHK( openpty(&master, &slave, slave_name, &termios_cfg, &winsize_cfg) );
    fprintf(stderr, "Opened a new tty: %s\r\n", slave_name);

    // step 2
    sprintf(cmd, "stty sane -F %s", slave_name);
    system(cmd);
    system("stty raw -isig -echo drain");

    // step 3
    pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork error\n");
        exit(-1);
    }

    if (pid == 0) {
        // child
        CHK( dup2(slave, STDIN_FILENO) );
        CHK( dup2(slave, STDOUT_FILENO) );
        CHK( dup2(slave, STDERR_FILENO) );
        CHK( close(slave) );
        CHK( close(master) );

        /* a missing piece of code causing
         * "sh: no job control in this shell" error
         */

        CHK( execvp(argv[1], (argv + 1)) );
    }

    // step 4
    signal(SIGCHLD, exit_on_sigchld);

    // step 5
    return copy_loop(STDIN_FILENO, master,
                     master, STDOUT_FILENO);
}
```

Good programmers borrow, so let's borrow the `copy_loop` routine to copy data
between tty(2) and tty(4) from the source code of `script` utility
([util-linux/lib/pty-session.c:ul_pty_proxy_master](https://github.com/karelzak/util-linux/blob/master/lib/pty-session.c#L520)
-> [/assets/how-tty-works/create_tty_1.c:35](/assets/how-tty-works/create_tty_1.c)).

Running our newly created program reveals `sh: no job control in this shell`
error message. Despite the "no job control" problem, `sh` seem to work and it
evaluates commands. Here is an example of running `date`:

```
$  ~  gcc --pedantic -g create_tty_1.c -l util && ./a.out sh
Opened a new tty: /dev/pts/24
sh: cannot set terminal process group (4181): Inappropriate ioctl for device
sh: no job control in this shell
sh-4.4$ date
Tue Oct 19 13:07:50 PST 2021
```

An easy workaround for `sh: no job control in this shell` message is using `setsid` with `-c` flag:

```
$  ~  ./a.out setsid -c sh
```

No more errors, moreover we can press `ctrl+z` to stop the execution of a running command. Nice!

```
sh-4.4$ sleep 1h
^Z
[1]+  Stopped(SIGTSTP)        sleep 1h
sh-4.4$ 
```

## Sessions

### Theory

Recall that using `setsid` fixed the problem from the previous section. Let's
check what `setsid` does. `man setsid`: `setsid` - "run a program in a new
session". That means creating a new session is a missing part of the previous
code sample.

A few notes on terminology. The term "session" appears in many areas of computer
science and corresponds to a continuous process of doing something while
maintaining a relevant state. A
[shell](https://en.wikipedia.org/wiki/Shell_(computing)) is a "program which
exposes an operating system's services to a human user". There are GUI and CLI
shells. For both GUI and CLI shells, session is roughly a set of running
programs that were started by a user (or started automatically when the user
logged in) and will terminate when the user terminates the session.

GUI and CLI shells use different mechanisms to manage their sessions:
* GUI shells: use X Windows (or Wayland) protocol (see `man xsm`, [XSMP protocol](https://en.wikipedia.org/wiki/X_session_manager#XSMP_Protocol));
* CLI shells: use system calls (`setsid`, `tcsetpgrp`, `setpgid`).

The kernel provides the following system calls for session management:
- create a new session (`setsid`);
- associate a tty device with a session (aka set a controlling terminal `ioctl(0, TIOCSCTTY, 0)`);
- organize processes into process groups (`setpgid`); set foreground process group (`tcsetpgrp`);
- deliver signals to each process in a given process group.


"Process group ID and session ID" section of the `man credentials` page gives a
great explanation of how processes are organized into sessions and process
groups. I'll borrow an explanation of why do we need these abstractions:
"Sessions and process groups are abstractions devised to support shell job
control". Important things are:

* each process belongs to exactly one session;
* each process belongs to exactly one process group;
* each session might have many process groups, but most one process group might
  be marked as a foreground process group (other process groups are considered
  being background process groups);
* newly created processes inherit session and process group from a parent.

Here is a visualization of process groups configured by sh to execute `sleep 1h
& ps | less`:

```

session
+---------------------------+
|  foreground process group |
|  +---------+-----------+  |
|  |ps       |  less     |  |
|  +---------+-----------+  |
|  background process group |
|  +---------------------+  |
|  |sleep 1h             |  |
|  +---------------------+  |
|  background process group |
|  +---------------------+  |
|  |sh (session leader)  |  |
|  +---------------------+  |
|                           |
+---------------------------+

```

How is that way of organizing processes useful:
* in most cases `tty` (which we've discussed in the previous post) delivers
  signals to a foreground process group (i.e. to each process in a process
  group);
* for `tty` access control: normally only a foreground process group is allowed
  to read/write from/to a `tty`, attempts to read/write to/from tty by a member
  of the background process group will generate a `SIGTTOU` or `SIGTTIN` signal
  for an entire process group;
* there is an option to make `tty` send `SIGHUP` signal to the entire session
  when `tty` closes.

### Job control in action

`bash` organizes processes into process groups and sets a currently running
command as a foreground process group. Because of that, pressing `ctrl+c`
(assuming enabled `stty isig`) will cause `tty` to deliver `SIGINT` to a
foreground process group which is, thanks to bash, is a currently running
command.

An easy way to find out a controlling terminal, a session, and a process group
of all processes in the system is to use `ps`. Note that `+` in the status
column means a foreground process group. A snippet below shows that to execute
`ps | less` command, my shell puts both `ps` and `less` in the same foreground
process group:

```
$  ~  ps -ostat,tty,sess,pgid,pid,comm -He | less
STAT TT        SESS  PGID   PID COMMAND
...
Ss   pts/1     5031  5031  5031                 zsh
R+   pts/1     5031  5836  5836                   ps
S+   pts/1     5031  5836  5837                   less
```

Now let's return to our buggy program and use `ps` to explore the problem with
job control. Remember that we've created a new `tty`, we've started a new `sh`
process and we want `sh` to use newly created `tty`. The reason we want `sh` to
use a newly created `tty` is because that way our tool can pretend to be an
`xterm` and read the output of a new `tty(4)`.

```
(1)           (2)         (3)            (4)         (5)
xterm <-----> tty <-----> script <-----> tty <-----> sh
      m           s              m           s
```

In our previous code sample we've made `sh` to use a slave filehandle of
`tty(4)`, but we didn't start a new session. As a result, `sh` cannot use
`setpgid` and `tcsetpgrp` to organize processes into process groups:

`./a.out sh`

```
STAT TT        SESS  PGID   PID COMMAND
S    ?        15557 15557 15582     xterm
Ss   pts/6    15583 15583 15583       bash
S+   pts/6    15583 15668 15668         a.out
S+   pts/6    15583 15668 15671           sh
```

A fix is to use setsid utility:

`./a.out setsid -c sh`

```
STAT TT        SESS  PGID   PID COMMAND
S    ?        15557 15557 15582     xterm
Ss   pts/6    15583 15583 15583       bash
S+   pts/6    15583 17175 17175         a.out
Ss+  pts/9    17178 17178 17178           sh
```

Look at that! Now `sh` is associated with a new `tty`, it's a part of a new
session and it's also a session leader. Now `sh` can use system calls to set a
foreground process group and to organize processes into groups.

Instead of using the setsid tool, we could've written code like this:

```
if (pid == 0) {
    // Use a tty's slave filehandle as stdin, stdout and setderr of a new process
    // ...

    // start a new session
    if (setsid() == -1) {
        fprintf(stderr, "Failed to start a new session\n");
        exit(-1);
    }

    // acquire stdin as a controlling terminal
    if (ioctl(0, TIOCSCTTY, 0) == -1) {
        fprintf(stderr, "Failed to acquire a terminal\n");
        exit(-1);
    }

    CHK( execvp(argv[1], (argv + 1)) );
}
```

### Missing parts of a.out

With recent changes, our utility works, but it has a few drawbacks:
* it does nothing useful: it intercepts data, but doesn't process it in any
  meaningful way;
* it doesn't propagate a window size change signal from `tty(2)` to `tty(4)`.

Exercise: fix the problems above.

## Conclusion

In this section, we've written a tool similar to the `script` utility. It misses
a few parts to be useful, but it helped us to discuss sessions, process groups,
and to understand how CLI shells work.

In this 3-post serious we've explored how terminal emulators, tty, and CLI tools
work. CLI tools aren't the most in-demand skills of 2021, but it's an
interesting area to explore in-depth. I had a lot of satisfaction from digging
through old technologies and understanding how things are. I hope that while
reading the text, you’ve also come up with questions and could answer them.
