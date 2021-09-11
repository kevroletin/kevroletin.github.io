---
layout: post
title:  "How terminal works. Part 2"
categories: debug
---

## Preliminaries

### tty, pts, serial port, line discipline

1. word **tty** originated as an abbreviation of teletypewriter;
2. in Linux **tty device** originally meant a connection to a terminal;
3. and later it was extended to refer to any serial-port style device
   [^ref](https://www.oreilly.com/library/view/linux-device-drivers/0596005903/ch18.html).

"serial-port style device" means that the device driver provides an interface to
a userland that looks like a serial-port device: a filehande together with
certain supported `ioctl` calls. Such a device doesn't necessary use a physical
serial port, instead it might use the network stack or something else
[^ref](https://en.wikipedia.org/wiki/Line_discipline).

Linux kernel provides existing building blocks for creating new tty drivers: tty
core and several existing tty **line disciplines**. Line discipline is a piece
of logic which **transforms** sequence of bytes into some other structured
representation. For example it might accumulate input into PPP packets (which is
useful for dial-up Internet connection). Or it might transform sequence of ASCII
characters into a sequence of lines of ASCII characters.

"Any serial-port style devices" is quite a broad definition. In this post we are
discussing specifically connections to a terminal. In case of a physical
terminal tty driver would expose to a userland one filehandle which `bash` will
use to receive user input and to send its output. For consistency with the next
diagram let's call that filehandle a slave filehandle *(note, this is
simplification, one terminal might be used to represent several logical "seats",
see [multiseat](https://www.freedesktop.org/wiki/Software/systemd/multiseat/))*:

TODO: draw a diagram
```
¯\_(ツ)_/¯ <---> ... <---> bash
                     slave
```

To support software terminal emulators in a similar manner as hardware terminal
emulators Linux implements pseudo terminals. In the previous diagram hardware
terminal was receiving bash output via a physical connection. In case of
software terminal emulator, Linux creates one more filehandle to send data to
xterm. So xterm runs on the same machine and reads bash output via a filehandle,
and the whole scheme looks like that:

```
(1)           (2)         (3)
xterm <-----> tty <-----> bash 
      master      slave
```

Master filehandle is referred as `ptm`, slave filehandle is `pts`. A mnemonics
to remember that bash uses a slave filehandle is that bash receives commands
from a user, hence it's being controlled and it's a slave. Xterm sends user
commands hence it's a master.

Back to line disciplines. Line discipline used for pseudoterminals is
[drivers/tty/n_tty.c](https://github.com/torvalds/linux/blob/f40ddce88593482919761f74910f42f4b84c004b/drivers/tty/n_tty.c).
It has many settings which we are going to discuss in detail soon.

### Session, background/foreground process groups

Bash organizes newly spawned processes into process groups. Normally, bash and
all it's children (the processes it spawned) are part of the same session
associated with the same `tty`. While executing commands bash marks a single
process group as a foreground process group of the session.

session, process group, foreground process group
* are configured using system calls (so both bash and tty know about them);
* are used to logically organize processes so that
* bash using tty behavior can implement job control, keyboard shortcuts to
  interrupt/suspend a running command, and terminating children when terminal
  got closed (with some nuances).

A simple experiment to demonstrate how bash organizes processes into process
groups. We create a background job `yes | sleep 1h` which consists of two
processes `yes` and `sleep`. Then we execute `ps` which runs in foreground:

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

Note that the term "job" is bash-specific; tty doesn't doesn't know about it.

We will discuss sessions and process groups in more detail later. This brief
introduction should be enough however to understand tty settings that we will
discuss soon.

## Features we can control using stty

In Part 1 we've already discovered existence of tty and observed some of it's
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
them using `stty` tool. To explore features which cannot be configured using
`stty` we will write C code.

`man` stty gives us 7 sections

1. Special characters
1. Special settings
1. Control settings
1. Input settings
1. Output settings
1. Local settings
1. Combination settings

Skipping "Combination settings" gives us 89 settings. 12 of them are synonyms,
leaving us 77 settings. I've counted 31 settings that are either "not relevant
to the modern world" or seem to be not relevant to software terminal emulators.

### Outdated features

* flow control and parity check

  These features make sense only for physical serial transmission lines. Serial
  ports are still in use, for example in embedded software engineering and
  embedded devices might implement flow control. But it's not relevant for
  pseudo terminals. I didn't check all the features, but many of them simply do
  nothing in case of a pseudo terminal.

  The only relevant feature from "Control settings" is the `hup` setting
  (send a hangup signal when the last process closes the tty).

* delays (bsN, crN, ffN, nlN)

  Delays just doesn't work with pseudo terminals.

  [drivers/tty/n_tty.c#L417-L419](https://github.com/torvalds/linux/blob/master/drivers/tty/n_tty.c#L417-L419)
  > Note that Linux currently ignores TABDLY, CRDLY, VTDLY, FFDLY
  > and NLDLY.  They simply aren't relevant in the world today.
  > If you ever need them, add them here.

* swtch

  swtch also seems to do nothing on Linux.

* line N - use line discipline N

  pseudo terminals always (TODO: check) use `n_tty` line discipline.

### Relevant features

We can further split remaining 46 settings into several groups (with some
intersections between groups). It's not necessary to study all the available
options, but skimming through all the groups is useful to understand that tty
has a finite amount of features.

The whole list can be found in the last section of this post. Here I'll just
discuss the groups I've chosen to organize options.

* input processing 

  tty might ignore some characters, convert them into other characters, or send
  signals once certain characters come from a master filehandle;

* output processing

  tty can convert certain characters into some other character sequences (`"\n"
  -> "\n\r"`, etc.);

* echo

  TODO

* line editor feature

  the user can enter text while using Backspace, Ctrl+w, Ctrl+u to remove last
  character, word or the whole line; to implement the feature tty buffers one
  line of input and sends it to bash only after receiving a return character
  "\r"; if echo and line editing are enabled together tty hides most of
  interaction with a user from bash but in the same time it actively interacts
  with xterm TODO

* generating signals

  common (but configurable) behavior is that pressing Ctrl+C will cause tty to
  send sigINT, closing xterm will eventually cause certain programs to receive
  sigHUP; programs receive sigWINCH signal after tty got resized;

* size

  there are several commands to get tty size and to resize it.

### stty sane

In the case of trouble use `stty sane`.

### How bash configures tty

Be aware that bash changes tty setting before executing commands and restores
previous setting after the command has finished. We can easily observe that by
using `watch stty ...`. Open two xterm windows and get tty path in the first
window. Start watching settings of the first tty in an other window using `watch
stty -a -F <pts>` command. Execute something like `cat -` in the first terminal.
Check new tty settings. Close `cat` with Ctrl+d and note that the settings has
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

### Experimanting

Equipped with the knowledge of tty options let's observe

1. how tty's line editing feature works;
1. how output processing make text files dumped into terminal look nicer;
1. how tty generates signals.

#### Line editor

We've already observed that tty can allow you to edit the current input line and
to delete a previous character, a word of the whole line. Let's quickly repeat
the experiment from the Part 1, but instead of `strace` let's play with bash
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
(while true; do dd count=1 bs=1 2>/dev/null | od -ax; done) < /data/out.pipe
```

Now you can enter something in the xterm1. In the other xterm window you can
observe in real time what tty is sending to a script.

Execute `stty raw` before calling `dd` in the xterm1 and observe that now
tty sends data to bash character by character:

```
stty raw; dd bs=1 of=/tmp/out.pipe
```

#### Making plain text output look good

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

#### Observing signals

We are going to execute two bash commands: one in background, one in foreground.
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
   
2. common key-bindings to stop execution of a command

   * suspend/resume foreground process

     Ctrl+z results in a foreground process receiving.

     ```
     --- SIGTSTP {si_signo=SIGTSTP, si_code=SI_KERNEL} ---
     --- stopped by SIGTSTP ---
     ```

     After that bash append the process to a list of `jobs`. `%2` resumes a
     stopped background process resulting in a `SIGCONT` signal.

   * Ctrl+c 

     delivers `SIGINT` to a foreground process group (TODO: check):

     ```
     --- SIGINT {si_signo=SIGINT, si_code=SI_KERNEL} ---
     +++ killed by SIGINT +++
     ```
   * Ctrl+\
   
     delivers `SIGQUIT` to a foreground process group.

4. Tty access control

   TODO:

5. Closing a terminal windows 

   TODO: fix this, add info about uart driver vs pseudo tty
   causes a foreground process to receive `SIGHUP` signal to background and foreground processes (TODO: but with several conditions)

   ```
   --- SIGHUP {si_signo=SIGHUP, si_code=SI_KERNEL} ---
   ```


[The TTY demystified](https://www.linusakesson.net/programming/tty/)

## Appendix: stty options by category

* input processing 

  The first group consists on self-explanatory options. The logic configured by
  these settings is quite simple: tty just replaces some characters with some
  other characters.

  * eol CHAR   - CHAR will end the line
  * eol2 CHAR  - alternate CHAR for ending the line
  * lnext CHAR - CHAR will enter the next character quoted
  * icrnl      - translate carriage return to newline
  * ignbrk     - ignore break characters
  * igncr      - ignore carriage return
  * inlcr      - translate newline to carriage return
  * iuclc      - translate uppercase characters to lowercase

  The second group is similar, except instead of sending characters tty will
  send signals:

  * eof CHAR  - CHAR will send an end of file (terminate the input)
  * quit CHAR - CHAR will send a quit signal
  * intr CHAR - CHAR will send an interrupt signal
  * susp CHAR - CHAR will send a terminal stop signal
  * brkint    - breaks cause an interrupt signal

  These settings affect line-editing features of tty:

  * erase CHAR  - CHAR will erase the last character typed
  * kill CHAR   - CHAR will erase the current line
  * rprnt CHAR  - CHAR will redraw the current line
  * werase CHAR - CHAR will erase the last word typed

  And finally:

  * iutf8 - assume input characters are UTF -8 encoded
  
  A note about `iutf8` flag. It's instructs tty's line editor to erase
  characters correctly. So that it removes several bytes from it's internal
  buffer when needed. This logic is [very
  simple](https://github.com/torvalds/linux/blob/f40ddce8/drivers/tty/n_tty.c#L393).
  More complicated features such as `iuclc` (translate uppercase characters to
  lowercase) doesn't work correctly with `iutf8`.

* output processing

  First I'll highlight an option which enables/disables output processing:

  * opost  - postprocess output

  Rest of the options more or less understandable:

  * ocrnl    - translate carriage return to newline
  * olcuc    - translate lowercase characters to uppercase
  * onlcr    - translate newline to carriage return-newline
  * onlret   - newline performs a carriage return
  * onocr    - do not print carriage returns in the first column
  * crtkill  - kill all line by obeying the echoprt and echoe settings
  * extproc  - enable "LINEMODE"; useful with high latency links
  * flusho   - discard output
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

  Enable/disable line editing:

  * icanon - enable special characters: erase, kill, werase, rprnt

  A few options:

  * min N  - with -icanon, set N characters minimum for a completed read
  * time N - with -icanon, set read timeout of N tenths of a second

  Note: (TODO: explain better) line discipline is a predefined tty configuration to provide line-editing feature.

  And then something we've already seen in input-related and output-related sections:

  * erase CHAR  - CHAR will erase the last character typed
  * kill CHAR   - CHAR will erase the current line
  * rprnt CHAR  - CHAR will redraw the current line
  * werase CHAR - CHAR will erase the last word typed

* generating signals

  Fist, two super important options:

  * hup    - send a hangup signal when the last process closes the tty
  * tostop - stop background jobs that try to write to the terminal
  
  Then a few more new options:
  
  * isig   - enable interrupt, quit, and suspend special characters
  * noflsh - disable flushing after interrupt and quit special characters
 
  And then something we've already seen: 
 
  * intr CHAR  - CHAR will send an interrupt signal
  * lnext CHAR - CHAR will enter the next character quoted
  * susp CHAR  - CHAR will send a terminal stop signal
  * brkint     - breaks cause an interrupt signal

* size

  * cols N - tell the kernel that the terminal has N columns
  * rows N - tell the kernel that the terminal has N rows
  * size   - print the number of rows and columns according to the kernel
