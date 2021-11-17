---
layout: post
title:  "How terminal works. Part 1: Xterm, user input"
categories: terminal
---

* TOC
{:toc}

## Motivation

This blog series explains how modern terminals and command-line tools work. The
main goal here is to learn by **experimenting**. I'll provide Linux tools to
debug every component mentioned in the discussion. Our main focus is to discover
**how** things work. For the explanation of **why** things work in a certain
way, I encourage the reader to read excellent articles:

* [The TTY demystified](https://www.linusakesson.net/programming/tty/)
* [A Brief Introduction to termios](https://blog.nelhage.com/2009/12/a-brief-introduction-to-termios/)

and to visit a computer history museum:

* [Teletype ASR 33 Part 10: ASR 33 demo](https://www.youtube.com/watch?v=S81GyMKH7zw)
* [The IBM 1401 compiles and runs FORTRAN II](https://www.youtube.com/watch?v=uFQ3sajIdaM).

Please note that I talk solely about Linux (because that is what I use), but many
discussed concepts should apply to other Unix-like systems.

I've chosen the "learn by experimenting" approach because that's how I've
learned about command-line tools. In my case, there was no single "click" moment
after which I've understood all the things. Instead, I've learned through a
never-ending process of building mental models, proving them to be wrong, and
then adjusting those models to reflect new knowledge.

Target audience are people who wants to start working on command line tools.

The series consists of 3 parts. The first part discusses how xterm work. Parts 2
and 3 talk about different features of tty:

* Part 1: Xterm, user input;
* Part 2: Xterm, CLI tool output;
* Part 3: pty, stty;
* Part 4: pty, sessions.

## Introduction

Let's start the discussion with an **inaccurate** diagram that demonstrates a
general use case for working with a command-line shell:

```
           (1)   (2)   (3)
user <---> xterm <---> bash 
                  
```

The user interacts with bash using a terminal emulator xterm. xterm is a GUI app
that receives "key pressed" events and writes corresponding characters into a
bidirectional filehandle (2). Bash reads those characters from (2) does
something and sends the output back to xterm, using the same filehandle (2).
Xterm reads bash outputs from (2) and renders them on the screen. (2) is "just a
file" and this communication scheme looks pretty simple.

If the user asks bash to execute a command, let's say `cat log.txt` then bash
spawns `cat` which uses the same filehandle to send its output to xterm:

```
                       bash
           (1)   (2)     (4)
user <---> xterm <--->   cat
                  
```

Again, pretty simple. In this unrealistic model (2) is "just a file" xterm and
cat exchange plain text.

In reality, things are slightly more complicated. The simple scheme of "using a
bidirectional filehandle to exchange plain text" is extended with more features
to provide a better user experience. Additional features are:
1. TUI interfaces. The terminal can draw characters at an arbitrary part of
   the screen; command-line tools can ask capabilities of the terminal and can
   handle window resize;
2. job control. Shell organizes processes into logical groups which can be
   paused/resumed or stopped altogether;
3. access control for the filehandle (2). Bash has a feature to spawn background
   processes, this might lead to a situation when two processes are writing
   their output into the same filehandle (2) at the same time; there should be 
   some access control mechanism;
4. "fixing" stupid tools which believe that the terminal is just a file with
   plain text; so that those tools look and feel better.

## User input

Requirement above and 50 years of history led us to this scheme:

```
           (1)         (2)       (3)
user <---> xterm <---> tty <---> bash 

```

The first thing to notice is a "middle man" **tty** between xterm and bash. We
will discuss tty in detail in parts 3 and 4. For now, we will just say that:
* tty sits between xterm and bash and passes data from one to the other in both
  directions;
* tty can be configured and depending on its configuration, it will "slightly"
  alter data it receives from one side before passing to the other.
* there is command `stty raw -echo -isig` which configures tty to pass data "as
  is without modification".
  
Using `stty raw -echo -isig` to disable most effects of tty is our main strategy
to explore how xterm works. Until the part 3, we will ignore the existence of tty
and will concentrate on exploring xterm's behavior.

Let's start by discussing a bi-directional link between a user and xterm.
Converting scancodes that come from a keyboard into GUI events happens in two
steps. First, Linux handles hardware events and turns them into keycodes that
can be read by userland (using device descriptors like
`/dev/input/by-id/usb-2.4G_2.4G_Wireless_Device-event-kbd`). Second, Windows
system (X or Wayland) reads Linux keycodes and converts them into its own
keycodes, and also assigns a keysym (i.e. a Unicode character). To check how it
works, one can use:

* `sudo showkey` to explore Linux keycodes *(visit [this page](https://tldp.org/HOWTO/Keyboard-and-Console-HOWTO-14.html) for more info)*;
* `xev` (or [`wev`](https://git.sr.ht/~sircmpwn/wev) for Wayland users) to explore GUI events.

For example, when I press the `q` button on my keyboard, depending on my keyboard
layout I see:
* showkey: keycode 16
* xev: keycode 24 (keysym 0x71, q)
* xev: keycode 24 (keysym 0x6ca, Cyrillic_shorti) 

The reason why I am mentioning this here is because xterm modifies its input
according to certain conventions before sending into tty(2). Hence converting
keycodes from a keyboard into characters written into (2) happens in 3 steps,
which is not trivial. It is beneficial to be able to trace all these steps.

Let's figure out what xterm sends into tty. There are two strategies we can use
to accomplish this task:

* `strace`: trace system calls *(we will trace `write` and `read` calls, but be aware that there are also
  [aio](https://man7.org/linux/man-pages/man7/aio.7.html) API)*;
* run a command-line tool that will
  * disable tty's input/output processing using `stty raw -echo -isig`
  * log its inputs.

### strace

Let's start with `strace` because, in my opinion, it's quite a practical
approach. In your daily life, if you'll get stuck with misbehaving command-line
tools, you can attach to a running process and observe what your terminal is
writing into filehandles and what your shell reads. You don't need to restart
running programs to figure out what is going on.

First, a little helper to find out PID of a terminal by clicking it with a
computer mouse (for users of XWindows system):

```
xprop | grep '_NET_WM_PID(CARDINAL)' | awk '{print $3}'
```

Then let's observe what xterm writes and reads into/from filehandles (please replace
`-p 22853` with an appropriate PID):

```
sudo strace -f -e 'trace=write,read' -e write=all -e read=all -p 22853 2>&1 | grep -v EAGAIN
```

For testing, I've entered `qwe` sequence and strace gave me:

```
write(4, "q", 1)                        = 1
 | 00000  71                                                q                |
read(4, "q", 4096)                      = 1
 | 00000  71                                                q                |
write(4, "w", 1)                        = 1
 | 00000  77                                                w                |
read(4, "w", 4096)                      = 1
 | 00000  77                                                w                |
write(4, "e", 1)                        = 1
 | 00000  65                                                e                |
read(4, "e", 4096)                      = 1
 | 00000  65 
```

That makes sense. xterm sends (writes) `q`. tty+bash echoes back `q` to display
it so that the user can see what he/she entered. Then a sequence `we` follows the
same pattern. Now, I'll try arrow keys: the left arrow and then the right arrow:

```
write(4, "\33[D", 3)                    = 3
 | 00000  1b 5b 44                                          .[D              |
read(4, "\10", 4096)                    = 1
 | 00000  08                                                .                |
write(4, "\33[C", 3)                    = 3
 | 00000  1b 5b 43                                          .[C              |
read(4, "\33[C", 4096)                  = 3
 | 00000  1b 5b 43                                          .[C              |
```

For left arrow key, xterm sends `\33[D` and receives back `\10`. `man ascii`
tells us that `33` Oct is the same `1b` Hex and it's a `\ESC` (escape)
ASCII control character. `10` Oct is `08` Hex and its `BS` backspace control
character (commonly abbreviated as `\b` thanks to C programming language). We
will discuss ANSI escape sequences and ASCII control characters soon, for now, we
can confirm that using strace helps to observe what xterm is actually doing: it
sends `qwe\ESC[D\ESC[C` and receives `qwe\b\ESC[C`.

Let's use strace to observe what bash is doing.

```
sh-4.4$ echo $$
5944
```

Entering the sequence `qwe<left><right>` gives me symmetrical result from the bash side:
It receives `qwe\ESC[D\ESC[C` and sends `qwe\b\ESC[C` back.

```
read(0, "\33", 1)                       = 1
 | 00000  1b                                                .                |
read(0, "[", 1)                         = 1
 | 00000  5b                                                [                |
read(0, "D", 1)                         = 1
 | 00000  44                                                D                |
write(2, "\10", 1)                      = 1
 | 00000  08  
```

I've promised to ignore tty for a while, but just to demonstrate why it might be
useful to strace both a terminal and bash, let's experiment. Let's execute
`cat -` command and observe in real-time what xterm is sending to tty and what
`cat` receives.

First, let's get the PID of a shell and then execute `cat`

```
echo $$

10519
sh-4.4$ cat -
```

Then in the other terminal window, let's find out the PID of `cat` using "parent PID"
option of ps:

```
ps --ppid 10519

  PID TTY          TIME CMD
10560 pts/5    00:00:00 cat
```

On my system, I am observing that xterm writes characters one by one immediately
after I've pressed a keyboard button. Yet `cat` receives the whole line only
after I've pressed Enter. Moreover, I can use the Backspace key to erase
previously entered characters, which is relatively complicated logic. This logic
is part of what tty is capable of.

```
read(0, "qwe\33[D\33[C\n", 131072)      = 10
 | 00000  71 77 65 1b 5b 44 1b 5b  43 0a                    qwe.[D.[C.       |
```

We will discuss tty in detail in the 2nd part. For now, let's just enjoy success
of our debugging approach: we've just observed what *exactly* xterm and bash
send to each other and how tty (which sits in the middle) can alter data before
sending it to a consumer. The big limitation of such an approach is that reading
sequences like `\33[D\33[C\n` requires a certain patience and might be quite hard
if applications output a lot of data  ¯\_(ツ)_/¯.

### Printing non-printable

While playing with strace we've encountered sequences like this `\33[D\33[C`
which I've later written like this: `\ESC[D\ESC[C`. In my daily life, I sometimes
encounter different notations, for example `\u001b[D\u001b[C`, `\x1b[D\x1b[C`,
or something else. Different software uses different conventions for visualizing
non-printable characters. Also, many programming languages have a way to embed
non-printable characters into string literals using a sequence of printable
characters. But again, conventions for representing non-printable characters
using printable ones differ between programming languages.

Let's discover how different software visualizes the ESC (escape) ASCII
character:

```
printf "\x1b" > data.txt
```

* vi, emacs: `^[`
* less: `ESC`
* code, gedit : on my systems render some nonsense
* hexdump: `1b` (hexdump supports many output formats)
* od -a : `esc` (od supports many output formats)
* strace: `\33` and `1b`
* python: `\x1b`
  ```
  open("/tmp/data.txt", "r").read()
  ```
* Haskell: `\ESC`
  ```
  import qualified Data.ByteString as BS
  BS.readFile "/tmp/data.txt" >>= print
  ```
* nodejs: `\u001b`
  ```
  const fs = require('fs')
  console.dir( fs.readFileSync('/tmp/data.txt', 'utf8') )
  ```

To make things more confusing, some popular programming languages support syntax
for embedding non-printable characters into string literals, but doesn't provide
easily accessible function to convert a string into the same notation. For
example, using the C programming language, I can easily make a string containing
ESC character:

```
char* str = "\x1b";
```

But the easiest way I know to visualize it using printable characters is to
write code like this:

```
#include <stdio.h>
#include <ctype.h>

int main() {
    FILE* f = fopen("/tmp/data.txt", "r");
    int c = fgetc(f);
    while (!feof(f)) {
        if (isprint(c))
            printf("%c", c);
        else
            printf("\\x%x", c);
        c = fgetc(f);
    }
    return 0;
}
```

The moral here is that different tools visualize non-printable characters
differently. To make things less confusing it's helpful to train your eye to
recognize magic strings `^[`, `\ESC`, `ESC`, `esc`, `1b`, `\x1b`, `0x1b`,
`\u001b`, `33`, `27`. Also, it's helpful to choose tools that you can understand
even under stress.

### stty raw -echo -isig

We've traced xterm using `strace` to check what it sends to bash. We can
accomplish a similar task without using a tracing tool. The most fool-proof way
to do so is to disable the effects of tty and to dump binary data which comes from
tty into a file. Then we can explore the content of a file using our favorite tool
of choice:

```
stty raw -echo -isig; dd bs=1 of=/tmp/data.txt
```

I prefer to use vi or od:

```
vi /tmp/data.txt
od -ac /tmp/data.txt
```

It might be cool to visualize the same data in real-time. One can use this bash one-liner:

```
stty sane -isig -echo -icanon; while true; do od -N 1 -ax -; done
```

Or convert `man ascii` into a [small c program](/assets/how-terminal-works/display_ascii.c).
It executes `stty raw -echo`
on startup, so that tty doesn't modify terminal output and hence the tool
demonstrates what terminal sends into tty.

Pressing a sequence of `a`, `1`, `Ctrl+d`, `Ctrl+l` gives:

```
a
1
EOT (end of transmission)
FF  '\f' (form feed)
```

`Alt+d` gives 2 characters:

```
ESC (escape)
d
```

`Ctrl+Alt+Shift+d` gives:

```
ESC (escape)
EOT (end of transmission)
```

which is the same as `Ctrl+Alt+d`, Shift is just ignored.

That behavior of xterm is **not** set in stone and can be configured in a
terminal-dependent way. Depending on its configuration, xterm would send
different characters or escape sequences in response to `Ctrl+Alt+Shift`
combination. Here is the discussion about xterm [modified
keys](https://invisible-island.net/xterm/modified-keys.html).

### UTF-8

Utf8 has a few nice features which I didn't appreciate enough until
recently:

1. Utf8 is a self-synchronizing code. If you take any Utf8-encoded string and
   randomly chop off the beginning so that you end up in the middle of multi-byte
   character, then:
   * you'll be able to detect an error: an attempt to decode an invalid
     character;
   * you'll be able to recover from the error by discarding bytes of a broken
     character and figure out the beginning of the next valid character.
2. ASCII characters (including control character) are valid one-byte Utf8 encoded
   characters.

Combined (1) and (2) give us a nice property that control characters will
never appear as part of multi-byte characters. I.e. python code below is correct
for any string consisting of multi-byte Utf8 characters:

```
"编程很有趣".find("\n") == -1
```

Let's understand why this is the case. `编程很有趣` is encoded using 15 bytes;
below I've represented bytes using decimal numbers:

```
编            程             很            有            趣
231 188 150 | 231 168 139 | 229 190 136 | 230 156 137 | 232 182 163
```

All these numbers are greater than 127 Dec. But all ASCII characters (including
control characters) are lesser or equal to 127 Dec. So it's safe to search for
single-byte ASCII characters in Utf8 strings without decoding them because all
bytes of every valid multi-byte character is guaranteed to be greater than 127
Dec.

Error recovery is possible and it works surprisingly simple. In binary notation
all ASCII characters start with leading `0`, all bytes of multi-byte characters
start with `1`. In addition, for multi-byte characters:
* only the first byte of a character can start with `11`;
* all continuation bytes (bytes 2, 3, 4) start with `10`.

You can easily observe these properties in action:

```
编
11100111
10111100
10010110
程
11100111
10101000
10001011
很
11100101
10111110
10001000
有
11100110
10011100
10001001
趣
11101000
10110110
10100011
```

Each byte starts with `1` indicating that it's a part of a multi-byte character.
Also, each byte contains an indication if it's the first byte or a continuation
byte.

## Conclusion

Using strace and by disabling tty features, we've explored how keyboard input
from users reaches command-line tools. We also saw that xterm might send
non-printable characters and different tools visualize non-printable characters
differently. We've improved our mental resilience by getting accustomed to
different notations and by trying different tools for visualizing control
characters. We also said a few words about Utf8 encoding which is the most
widely used Unicode encoding nowadays.

In this blog post we've discussed how xterm handles user input. In the next post
will discuss how xterm visualizes output of CLI tools.

Stay tuned :) 

