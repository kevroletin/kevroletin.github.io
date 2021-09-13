---
layout: post
title:  "How terminal works. Part 2: pty"
categories: terminal
---

* TOC
{:toc}

## Terminology

### tty, pts, serial port, line discipline

1. word **tty** originated as an abbreviation of teletypewriter;
2. in Linux **tty device** originally meant a connection to a terminal;
3. and later it was extended to refer to any serial-port style device
   [\[ref\]](https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch18.html).

"serial-port style device" means that the device driver provides an interface to
a userland that looks like a serial-port device: a filehandle together with
certain supported `ioctl` calls. Such a device doesn't necessarily use a physical
serial port, instead it might use the network stack or something else
[\[ref\]](https://en.wikipedia.org/wiki/Line_discipline).

Linux kernel provides building blocks for creating new tty drivers: tty core and
several existing tty **line disciplines**. Line discipline is a piece of logic
that **transforms** sequence of bytes into some other structured
representation. For example, it might accumulate input into PPP packets (which is
useful for dial-up Internet connection). Or it might transform a sequence of ASCII
characters into a sequence of lines of ASCII characters.

Here we aren't interested in any serial-port style devices; instead, we are
discussing connections to a terminal. In case of a connection to a physical
terminal, tty driver would expose to a userland a filehandle which `bash` would
use to receive user input and to send its output. For consistency with the next
diagram let's call that filehandle a slave filehandle *(note, this is a
simplification, one terminal might be used to represent several logical "seats",
see [multiseat](https://www.freedesktop.org/wiki/Software/systemd/multiseat/))*:

TODO: draw a diagram
```
¯\_(ツ)_/¯ <---> ... <---> bash
                     slave
```

To support software terminal emulators similarly to hardware terminals
Linux implements pseudo terminals. In the previous diagram hardware terminal was
receiving bash output via a physical connection. In the case of a pseudo-terminal,
Linux creates one more filehandle to send data to xterm. That way xterm runs on
the same machine and reads bash output via a filehandle; the whole scheme looks
like that:

```
(1)           (2)         (3)
xterm <-----> tty <-----> bash 
      master      slave
```

Master filehandle is referred to as `ptm`, slave filehandle is `pts`. The
pseudo-terminal is abbreviated as `pty` (see `man pty`). A mnemonics to remember
that bash uses a slave filehandle is that bash receives commands from a user,
hence it's being controlled and it's a slave. Xterm sends user commands hence
it's a master.

Back to line disciplines. Line discipline used for pseudoterminals is
[drivers/tty/n_tty.c](https://github.com/torvalds/linux/blob/f40ddce88593482919761f74910f42f4b84c004b/drivers/tty/n_tty.c).
It has many settings which we are going to discuss in detail soon.
(TODO: check).

### Session, background/foreground process groups

Bash organizes newly spawned processes into process groups. Normally, bash and
all its children (the processes it spawned) are part of the same session
associated with the same `tty`. While executing commands bash marks a single
process group as a foreground process group of the session.

session, process group, foreground process group
* are configured using system calls (so both bash and tty know about them);
* are used to logically organize processes so that
* with help of tty bash can implement job control, keyboard shortcuts to
  interrupt/suspend a running command, and it terminates children when a terminal
  gets closed (with some nuances).

Here is a simple test to demonstrate how bash organizes processes into process
groups. We create a background job `yes | sleep 1h` which consists of two
processes `yes` and `sleep`. Then we execute `ps` which runs in the foreground:

```
yes | sleep 1h & 
[1] 6741

ps -ostat,tty,sess,pgid,pid,comm -H

STAT TT        SESS  PGID   PID COMMAND
S    pts/4     6545  6545  6545 bash
S    pts/4     6545  6740  6740   yes
S    pts/4     6545  6740  6741   sleep
R+   pts/4     6545  6769  6769   ps
```

Notice that all processes run in the same session and share the same tty. `yes`
and `sleep` share the same process group. `ps` is marked with `+` which means
"is in the foreground process group".

Note that the term "job" is a bash-specific term; tty doesn't know about
it.

We will discuss sessions and process groups in more detail later. That brief
introduction should be enough however to understand tty settings that we will
discuss soon.

## Features configurable by stty

In Part 1 we've already discovered the existence of tty and observed some of it's
features. Now let's recall what we've seen and let's build on top of that
knowledge:

1. tty passes data from xterm to bash in both directions; tty can be configured
   and depending on its configuration it will "slightly" alter data it receives
   from one side before passing to the other side;
2. in addition to passing data through filehandles tty also generates signals;
   again, delivering signals is configurable logic; it might work differently
   depending on tty's configuration and depending on how processes are organized
   into logical groups.

I'll split tty's features into two parts depending on whether we can configure
them using the `stty` tool. To explore features that cannot be configured using
`stty` we will write C code.

`man stty` gives us 7 sections

1. Special characters
1. Special settings
1. Control settings
1. Input settings
1. Output settings
1. Local settings
1. Combination settings

Skipping "Combination settings" gives us 89 settings. 11 of them are synonyms,
leaving us 78 settings. I've counted 34 settings that are either "not relevant
to the modern world" or seem to be not relevant to software terminal emulators.
This leaves us 44 options to discuss. I've organized these 44 options into 7
groups based on features they control.

### Outdated features

* flow control and parity check

  These features make sense only for physical serial transmission lines. Serial
  ports are still in use, for example in embedded software engineering and
  embedded devices might implement flow control. But it's not relevant for
  pseudo terminals. I didn't check all the features, but many of them simply do
  nothing in the case of a pseudo-terminal.

  The only relevant feature from "Control settings" is the `hup` setting
  (send a hangup signal when the last process closes the tty).

* delays (bsN, crN, ffN, nlN)

  Delays just don't work with pseudo terminals.

  [drivers/tty/n_tty.c#L417-L419](https://github.com/torvalds/linux/blob/master/drivers/tty/n_tty.c#L417-L419)

* swtch

  swtch also seems to do nothing on Linux.

* line N - use line discipline N

  pseudo terminals always (TODO: check) use `n_tty` line discipline.

More details are in the table (TODO) add link.

### Relevant features

We can further split the remaining 46 settings into several groups. It's not
necessary to study all the available options, but skimming through all the
groups are useful to understand that tty has a finite amount of features.

The whole list can be found in the last section of this post. Here I'll just
discuss the groups that I've chosen to organize all available options.

* input processing 

  tty might ignore some characters which come from the master filehandle (from
  xterm), or convert them into other characters;

* output processing

  tty can convert certain characters which come from slave filehandle (from bash)
  into some other character sequences (`"\n" -> "\n\r"`, etc.);

* echo

  tty sends data that came from xterm back to xterm so that the user can see
  what he/she is typing; note that echoing control characters in case of enabled
  the line editor is more complicated than just sending the same characters back (TODO: rephrase);

* line editor feature

  the user can enter text while using Backspace, Ctrl+w, Ctrl+u to remove last
  character, word, or the whole line; to implement the feature tty buffers one
  line of input and sends it to bash only after receiving a return character
  "\r"; if echo and line editing are enabled together tty hides most of
  interaction with a user from bash but at the same time it actively interacts
  with xterm to implement line editing;

* generating signals

  common (but configurable) behavior is that pressing Ctrl+C will cause tty to
  send SIGINT, closing xterm will eventually cause certain programs to receive
  SIGHUP; programs receive sigWINCH signal after tty got resized;

* size

  there are several commands to get tty size and to resize it.

### Combination setting

Useful combinations are:

* `stty sane` - combines enabled echo, line editor, output processing to make
  text tools look nice and common key bindings;

* `stty raw -echo -isig` - make pty pass characters "as is".

Be aware that bash changes tty setting before executing commands and restores
the previous setting after the command has finished. We can easily observe that by
using `watch stty ...`. Open two xterm windows and get the tty path in the first
window. Start watching settings of the first tty in another window using `watch
stty -a -F <pts>` command. Execute something like `cat -` in the first terminal.
Check new tty settings. Close `cat` with Ctrl+d and note that the settings have
changed back.

```
xterm1               | xterm2
---------------------|-------------------------------
~$ tty               |
/dev/pts/1           |
                     | ~$ watch stty -a -F /dev/pts/1
~$ cat -             |
...                  |
```

## Practice

Equipped with the knowledge of tty options let's observe

1. how tty's line editing feature works;
1. how output processing makes text files dumped into terminal look nicer;
1. what signals does pty generate.

### Line editor

We've already observed that tty can allow you to edit the current input line and
to delete a previous character, a word of the whole line. Let's quickly repeat
the experiment from Part 1, but instead of `strace` let's play with bash
scripts. We'll use one xterm window to receive input and the other to visualize
what tty is sending to our bash script. Open two xterm windows, and in the first
xterm:

```
mkfifo /tmp/out.pipe
dd bs=1 of=/tmp/out.pipe
```

In the second xterm execute the following line. The while loop with `dd` is my
attempt to make `od` read characters one by one (asking it directly using `od
-w1` option gives an error):

```
(while true; do dd count=1 bs=1 2>/dev/null | od -ax; done) < /tmp/out.pipe
```

Now you can enter something in the xterm1. In the other xterm window, you can
observe in real-time what tty is sending to a script.

Execute `stty raw` before calling `dd` in the xterm1 and observe that now
tty sends data to bash character by character:

```
stty raw; dd bs=1 of=/tmp/out.pipe
```

### Making plain text output look good

A few lines of text might look either like this:

```
stty sane; printf "\nline1\nline2\nline3\n"
line1
line2
line3
```

or like that:

```
stty raw; printf "\nline1\nline2\nline3\n"

line1
     line2
          line3
               sh-4.4$ 
```

Using tools from the Part 1 we can easily discover that in the first test case
xterm receives `\r\nline1\r\nline2\r\nline3\r\n` and in the second test case it
receives `\nline1\nline2\nline3\n`. The difference is that `\n` were replaced by
'\r\n' in the first test case. This behavior is configured by `onlcr` setting
(translate newline to carriage return-newline).

### Observing signals

We are going to execute two bash commands: one in the background, one in the foreground.
And then we will use `strace` to observe which signals reach both processes:

```
echo $$
18552
sleep 1h &
sleep 1h
```

Let's find out pids of each `sleep` process:

```
ps --ppid 18552
  PID TTY          TIME CMD
19055 pts/8    00:00:00 sleep
19062 pts/8    00:00:00 sleep
```

Then attach with `strace` from other xterm windows:

```
strace -f -e 'trace=!all' -p 19055
```

and:

```
strace -f -e 'trace=!all' -p 19062
```

1. resizing xterm windows

   delivers `SIGWINCH` to a foreground process;
   
2. common key-bindings to stop the execution of a command

   * suspend/resume foreground process

     Ctrl+z will cause a foreground process group receiving `SIGTSTP`.

     ```
     --- SIGTSTP {si_signo=SIGTSTP, si_code=SI_KERNEL} ---
     --- stopped by SIGTSTP ---
     ```

     After that bash appends the process to a list of `jobs`. `%2` resumes a
     stopped the background process by sending the `SIGCONT` signal.

   * Ctrl+c 

     delivers `SIGINT` to a foreground process group:

     ```
     --- SIGINT {si_signo=SIGINT, si_code=SI_KERNEL} ---
     +++ killed by SIGINT +++
     ```

   * Ctrl+\
   
     delivers `SIGQUIT` to a foreground process group.

4. Tty access control

   When a process from a background process group attempts to read from tty the
   whole process group receives `SIGTTIN`:

   ```
    strace -e 'trace=!all' dd bs=1 &
   [2] 1739
   --- SIGTTIN {si_signo=SIGTTIN, si_code=SI_KERNEL} ---                                                                                                                                          
   --- stopped by SIGTTIN ---
   ```

   Similarly with configured `stty tostop` an attempt to output from a
   background process would cause the process group to receive `SIGTTOU`
   signal:

   ```
   stty tostop; echo 123 &
   stty -tostop; echo 123 &
   ```

5. Closing terminal windows 

   This topic has one nuance. Looking at these stty settings:
   * hup   - send a hangup signal when the last process closes the tty
   * hupcl - same as hup
   
   and skimming through [drivers/tty/tty_jobctrl.c](https://github.com/torvalds/linux/blob/f40ddce8/drivers/tty/tty_jobctrl.c#L269)
   gives an idea that pty sends `SIGHUP` to all processes within the session when either:
   * bash closes master filehandle;
   * bash disassociates itself from a session by starting a new session.

   However, on my system, I observe that bash disables `-hupcl` tty option and
   hence it doesn't rely on this feature:
   
   ```
   stty -a
   ...
   -hupcl
   ...
   ```

   However `strace` shows that when I close xterm, all processes withing corresponding
   session receive `SIGHUP`. Tracing xterm, bash, and all background/foreground
   jobs reveal that:
   * when xterm exits it sends `SIGHUP` to bash;
   * when bash receives `SIGHUP` it terminates its children by using the same signal.
   
   Hence in practice delivery of the `SIGHUP` signal depends on the behavior of a
   command-line shell such as bash. Some experimentation reveals that, for
   example, `zsh` has a slightly different behavior and it doesn't send `SIGHUP`
   to running background jobs. For example, in zsh

   ```
   sleep 1h &
   ```
   
   would stay alive after closing xterm.

## Conclusion

In this post, we've explained the origins of the term `tty` and relationships between
`tty`, `pty`, and a serial port. We've talked a little about
foreground/background process groups. And then we've experimented with tty's
features. We also discovered that many stty's options don't apply to pty and
hence can be safely ignored while working with terminal emulators.

In the last Part 3 of this post series, we will continue exploring tty features
by writing C code. We will develop a simple analog of a `script` utility with the
goal of understanding sessions and process groups better. Stay tuned :)

## Appendix: relevant stty options by category

* input processing 

  tty replaces some input characters that come from the master filehandle with
  some other characters:

  * eof CHAR   - CHAR will send an end of file (terminate the input)
  * eol CHAR   - CHAR will end the line
  * eol2 CHAR  - alternate CHAR for ending the line
  * lnext CHAR - CHAR will enter the next character quoted
  * icrnl      - translate carriage return to newline
  * ignbrk     - ignore break characters
  * igncr      - ignore carriage return
  * inlcr      - translate newline to carriage return
  * iuclc      - translate uppercase characters to lowercase

* output processing

  first, an option to enable/disables output processing:

  * opost  - postprocess output

  rest of the options control whether tty should replace certain characters
  which come from slave filehandle with some other characters:

  * ocrnl    - translate carriage return to newline
  * olcuc    - translate lowercase characters to uppercase
  * onlcr    - translate newline to carriage return-newline
  * onlret   - newline performs a carriage return
  * onocr    - do not print carriage returns in the first column
  * crtkill  - kill all line by obeying the echoprt and echoe settings
  * iexten   - enable non-POSIX special characters
  * noflsh   - disable flushing after interrupt and quit special characters 
  * xcase    - with icanon, escape with '\' for uppercase characters

* echo

  * echo     - echo input characters
  * crterase - echo erase characters as backspace-space-backspace
  * ctlecho  - echo control characters in hat notation ('^c')
  * echok    - echo a newline after a kill character
  * echonl   - echo newline even if not echoing other characters
  * echoprt  - echo erased characters backward, between '\' and '/'

* line editor

  enable/disable a line editor:

  * icanon - enable special characters: erase, kill, werase, rprnt

  keybindings:

  * erase CHAR  - CHAR will erase the last character typed
  * kill CHAR   - CHAR will erase the current line
  * rprnt CHAR  - CHAR will redraw the current line
  * werase CHAR - CHAR will erase the last word typed

  and also:

  * iutf8 - assume input characters are UTF-8 encoded
  
  `iutf8` instructs tty's line editor to erase characters correctly. So that it
  removes several bytes from its internal buffer when needed. This logic is
  [very simple](https://github.com/torvalds/linux/blob/f40ddce8/drivers/tty/n_tty.c#L393).
  More complicated features such as `iuclc` (translate uppercase characters to
  lowercase) don't work correctly with `iutf8`.

* access control

  * tostop     - stop background jobs that try to write to the terminal

  and rudimentary flow control:

  * start CHAR - CHAR will restart the output after stopping it
  * stop CHAR  - CHAR will stop the output
  * ixany      - let any character restart output, not only start character
  * ixon       - enable XON/XOFF flow control

* generating signals

  * hup    - send a hangup signal when the last process closes the tty
  * isig   - enable interrupt, quit, and suspend special characters
  * noflsh - disable flushing after interrupt and quit special characters
 
  keybindings:
 
  * intr CHAR - CHAR will send an interrupt signal
  * susp CHAR - CHAR will send a terminal stop signal
  * quit CHAR - CHAR will send a quit signal

* size

  * cols N - tell the kernel that the terminal has N columns
  * rows N - tell the kernel that the terminal has N rows
  * size   - print the number of rows and columns according to the kernel
