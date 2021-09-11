---
layout: post
title:  "How terminal works. Part 1"
categories: debug
---

This article is an explanation of how modern terminals and command-line tools
work together. The main goal here is to learn by experimenting. I'll provide
Linux tools to debug every component mentioned in the discussion. Our main focus
is to discover **how** things work. To find the explanation **why** things work
in a certain way, I encourage the reader to visit excellent articles:

* [The TTY demystified](https://www.linusakesson.net/programming/tty/)
* [A Brief Introduction to termios](https://blog.nelhage.com/2009/12/a-brief-introduction-to-termios/)

Please note that I talk solely about Linux (because that what I use), but many
presented concepts should apply to other Unix-like systems.

Let's start the discussion with an **inaccurate** diagram that demonstrates a
general use case for working with a command-line shell:

```
           (1)   (2)   (3)
user <---> xterm <---> bash 
                  
```

The user interacts with bash using terminal emulator xterm. xterm is a GUI app that
receives "key pressed" events and writes corresponding characters into
a bidirectional filehandle (2). Bash reads those characters from (2) does something
and sends the output back to xterm, using the same filehandle (2). Xterm reads bash
output from (2) and renders them on the screen. (2) is "just a file" and this
communication scheme looks pretty simple.

If the user asks bash to execute a command, let's say `cat log.txt` then bash
spawns `cat` which uses the same filehandle to send its output to xterm:

```
                       bash
           (1)   (2)     (4)
user <---> xterm <--->   cat
                  
```

Again, pretty simple. In this unrealistic model (2) is "just a file" xterm and
cat exchange pain text.

In reality, things are slightly more complicated. The simple scheme of "using a
bidirectional filehandle to exchange plain text" is extended with more features
to provide a better user experience. Additional features are:
1. TUI interfaces. The terminal is drawing characters at an arbitrary part of
   the screen; command-line tools can ask capabilities of the terminal and can
   handle window resize;
2. job control. Shell organizes processes into logical groups which can be
   paused/resumed or stopped together;
3. access control for the filehandle (2). Bash has a feature to spawn background
   processes, this might lead to a situation when two processes are writing
   their output into the same filehandle (2) at the same time; there should be 
   some access control mechanism;
4. "fixing" stupid tools which believe that the terminal is just a file with
   plain text; so that those tools look and feel better.

## Part 1: Terminal emulator

Requirement above and 50 years of history led us to this scheme:

```
           (1)         (2)       (3)
user <---> xterm <---> tty <---> bash 

```

The first thing to notice is a "middle man" **tty** between xtem and bash. We
will discuss tty in detail in the part 2. For now, we will just say that:
* tty sits between xterm and bash and passes data from one to the other in both
  directions;
* tty can be configured and depending on its configuration it will "slightly"
  alter data it receives from one side before passing to the other side.
* there is command `stty raw -echo -isig` which configures tty to pass data "as
  is without modification".
  
Using `stty raw -echo -isig` to disable most effects of tty is our main strategy
to explore how xterm works. Until the part 2 we will ignore the existence of tty
and will concentrate on exploring xterm's behavior.

Let's start by discussing a bi-directional link between a user and xterm.
Converting keycodes that come from a keyboard into GUI events happens in two
steps. First, Linux handles hardware events and turns them into keycodes that
can be read by userland (using device descriptors like
`/dev/input/by-id/usb-2.4G_2.4G_Wireless_Device-event-kbd`). Second, Windows
system (X or Wayland) reads Linux keycodes and converts them into its own
keycodes, and also assigns a keysym. To check how it works one can use:

TODO: https://tldp.org/HOWTO/Keyboard-and-Console-HOWTO-14.html
+ scancodes not keycodes
+ mention dumpkeys
+ mention xkb

* `sudo showkey` to explore Linux keycodes;
* `xev` (or [`wev`](https://git.sr.ht/~sircmpwn/wev) for Wayland users) to explore GUI events.

For example, when I press the `q` button on my keyboard, depending on my keyboard
layout I see:
* showkey: keycode 16
* xev: keycode 24 (keysym 0x71, q)
* xev: keycode 24 (keysym 0x6ca, Cyrillic_shorti) 

The reason why I am mentioning this here is because xterm modifies its input
according to certain conventions before sending into tty(2). Hence converting
keycodes from a keyboard into characters written into (2) happens in 3 steps
which is not trivial. It is beneficial to be able to trace all these steps.

Let's figure out what xterm sends into tty. There are two strategies we can use
to accomplish this task:

* `strace`: trace system calls (most likely `write` and `read`, but there are also
  [aio](https://man7.org/linux/man-pages/man7/aio.7.html) API);
* run a command-line tool that will
  * disable tty's input/output processing using `stty raw -echo -isig`
  * logs its inputs

### strace

Let's start with `strace` because, in my opinion, it's quite a practical
approach. If in your daily life you'll get stuck with misbehaving command-line
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
same pattern. Now I'll try arrow keys: the left arrow and then the right arrow:

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

For left arrow key xterm sends `\33[D` and receives back `\10`. `man ascii`
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
sh-4.4$ echo $$
10519
sh-4.4$ cat -
```

Then in the other terminal window let's find out the PID of `cat` using "parent PID"
option of ps:

```
sh-4.4$ ps --ppid 10519
  PID TTY          TIME CMD
10560 pts/5    00:00:00 cat
```

On my system, I am observing that xterm writes characters one by one immediately
after I've pressed a keyboard button. Yet `cat` receives the whole line only
after I've pressed Enter. Moreover, I can use the Backspace key to erase
previously entered characters which is relatively complicated logic. This logic
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

#### Printing non-printable

While playing with strace we've encountered sequences like this `\33[D\33[C`
which I've later written like this: `\ESC[D\ESC[C`. In my daily life I sometimes
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

To make things more confusing some popular programming languages support syntax
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
on startup so that tty doesn't modify terminal output and hence the tool
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

This behavior of xterm is **not** set in stone and can be configured in a
terminal-dependent way. Depending on its configuration xterm would send
different characters or escape sequences in response to `Ctrl+Alt+Shift`
combination. Here is the discussion about xterm [modified
keys](https://invisible-island.net/xterm/modified-keys.html).

### Utf8

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

Let's understand why this is the case. `编程很有趣` is encoded using 15 bytes
I've represented bytes with decimal numbers:

```
编            程             很            有            趣
231 188 150 | 231 168 139 | 229 190 136 | 230 156 137 | 232 182 163
```

All these numbers are greater than 127 Dec. But all ASCII characters (including
control characters) are lesser or equal to 127 Dec. So it's safe to search for
single-byte ASCII characters in Utf8 strings without decoding it because all
bytes of every valid multi-byte character are guaranteed to be greater than 127
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

### Rendering graphics

Finally, we are equipped with tools and knowledge to discuss what terminals
emulators emulate. Terminals (either hardware devices or software programs)
perform two main tasks:
* visualize the output of tools like bash, cat, top, vi, etc. to the user;
* pass user input to command-line tools.

We've already discussed how the terminal handles user input. Now let's discuss how
it visualizes bash output. xterm maintains state of a "scene" (a buffer) and
exposes API to modify that state. API is centered around a concept of a cursor
and instructions which modify the buffer. For example:
* insert a character in the current position and move the cursor to the right (I
  am sorry for [RTL
  scripts](https://en.wikipedia.org/wiki/Right-to-left_script));
* move the cursor in different directions;
* change color and other properties of newly rendered characters;
* erase characters, clear a line or clear the whole buffer; 
* etc. ...

I guess the phrase "xterm exposes API" sounds too modern for how API is actually
organized. In reality, xterm just parses text which comes from bash and
separates printable characters from commands. Commands are either
* single-byte ASCII control characters;
* escape sequences: a sequence of several bytes starting with an ESC character
  and several printable ASCII characters.

In a nutshell, the behavior of xterm is quite simple. It reads bash ~~(well,
tty)~~ output character by character and decides:
* this is printable, let's print it in the current cursor position;
* this is a command, let's execute the command.

Additional difficulties which make this scheme a little more complicated are:
* errors handling: both for ill-formed Unicode characters and ill-formed escape
  sequences;
* emulation: terminal emulators emulate the behavior of previously existed
  hardware terminals; for that reason they provide instructions (escape
  sequences) to choose which terminal model to emulate; this alters emulator's
  behavior;
* there is no single standard that covers the behavior of all terminals and
  terminal emulators.

The last point is, probably the biggest difficulty associated with terminals.
There is a variety of different hardware terminals and terminal emulators which
support different features and behave slightly differently. Digging through
standards is hard and doesn't pay well: terminals often doesn't follow standards
by implementing only a subset of features or they implement non-specified
behavior differently or they introduce their own new features.

#### History

My current understanding is that `longlong` ago terminal vendors where
implementing terminals with similar features, but they implemented different
APIs. This led to the creation of terminfo - a database of terminal features
which led to the creation of software terminal compatibility libraries such as
ncurses. At some point industry matured and produced an ANSI standard that
standardized terminal API and gave us words ANSI-compatible terminal and
ANSI-escape sequences. The first popular terminal that supported the ANSI
standard was VT100. It was so successful that it made ANSI not only a standard
on a paper but also a de-facto standard. That's why nowadays by saying
"VT100-compatible terminal" and "ANSI-compatible terminal" some people mean the
same thing. Modern command line tools sometimes aren't concerned with
compatibility and they might not use terminfo. Instead, they expect an
ANSI-compatible terminal emulator and assume support for certain features.

The evolution of terminals didn't stop after the release of the ANSI standard. Modern
terminal emulators continue to invent new features which have a different level of
adoption. To give just a few examples:

* a widely supported non-standard feature: enable/disable alternative screen buffer
  `\ESC[?1049h` and `\ESC[?1049l`; tui programs use it to restore terminal content
  after they finish;
* a rare feature: urxvt's "set background image" `\ESC]20` doesn't seem to have
  support in other terminals;
* somewhere in between: [truecolor escape
  codes](https://github.com/termstandard/colors) are supported by many popular
  terminals but not by all.

The moral here is that:

* terminals are difficult to understand because of an uncountable amount of small details;

  history led us to a situation when there are many terminals (both hardware
  and software emulators); they have slightly different behavior and there is no
  single document describing a "reference terminal";

* focusing on modern software terminal emulators make it easier;

  modern major terminal emulators are ANSI compatible and hence:
  * they support similar syntax for ANSI escape sequences (with differences in how
    errors are handled and how arguments for non-standard commands are parsed,
    a good example is [truecolor escape
    codes](https://github.com/termstandard/colors));
  * they support many similar features, so by using widely supported features
    and writing enough "if" conditions one can achieve terminal-independent code
    for modern terminals.

#### Practice

##### Ask xterm to execute commands

Let's ask xterm to execute something interesting for us. First, let's consult with
the Internet wisdom to find commonly used escape sequences:

* [a nice gist](https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797)
* [man console_codes](https://man7.org/linux/man-pages/man4/console_codes.4.html)
* [xterm manual](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Functions-using-CSI-_-ordered-by-the-final-character_s)
* [Build your own Command Line with ANSI escape codes](https://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html)

Then open two terminals. Get `tty` from one terminal and try to write something
into this tty using the other terminal window:

```
xterm1               | xterm2
---------------------|-------------------------------
~$ tty               |
/dev/pts/1           |
                     | ~$ echo "\n123\n" > /dev/pts1
123                  |
                     |
```

It seems to work. Let's try to send an escape sequence to clear the screen:

```
printf "\x1b[2J" > /dev/pts/1
```

That also works. Let's extend this idea to implement a simple [interactive
drawing program](/assets/how-terminal-works/wasd_paint.sh) which handles the
following keybindings:
* `wasd` or `hjkl` - move cursor;
* `space` - insert `x`;
* `.` - erase current character by inserting a space;
* `ESC` - exit.

The main idea of the script is to send ANSI escape sequences into a given tty,
so that xterm reads the sequences and executes requested commands:

```
  case $char in
      [wk] ) cmd="A" ;;                  # move cursor up
      [sj] ) cmd="B" ;;                  # move cursor down
      [ah] ) cmd="D" ;;                  # move cursor left
      [dl] ) cmd="C" ;;                  # move cursor right
      ...
  esac
  printf "\x1b[$cmd" > $pts
```

Here is my self-portrait which I've created using our new tool:

```
$  /tmp      xx  xxxxxxx xxxx           x
$  /tmp     xx  x            x           x
$  /tmp     x  x               x          x
$  /tmp     x x                 xx         x
$  /tmp     xx                    x        x
$  /tmp     x   xxxxxxxx   xxxxxxx x       x
$  /tmp      x    xxxx   x     xxx  x      x
$  /tmp      x   x xx x  x    x x x  x x x x
$  /tmp      x    xxxx   x     xxx   x x x x
$  /tmp      x          x            x x x x
$  /tmp      x          x            x x x x
$  /tmp      x         x             x x x x
$  /tmp      x x      x    x       x x x x x
$  /tmp       x x     xxxxx      xxx x x x x
$  /tmp        x xx  xxxxxxxxx  xxx  x x x x
$  /tmp         xxxx  x     x  xxx x x x x x
$  /tmp           xxx  xxxxx   xx x  x x x x
$  /tmp             xx        xxx    x x x x
$  /tmp              xxxxxxxx xx     x x x x
$  /tmp              x x x  x        x x x
$  /tmp              xxxxxxxxx       x x x
$  /tmp              x x x x x       x x
$  /tmp              x x x x x       x x
$  /tmp              x x x x x       x
$  /tmp              x x x x x       x
$  /tmp              x x x x x
                       x x x
                       x x x
                         x
```

##### Explore vim output

In the last experiment, we were sending ANSI escape sequences to xterm. Now
let's go in the opposite direction. Let's see what existing TUI programs send to
xterm. One way to capture the output of command-line tools is to use utilities
like [script](https://man7.org/linux/man-pages/man1/script.1.html),
[autoexpect](https://linux.die.net/man/1/autoexpect) or
[aciinema](https://asciinema.org/) (we will discuss how these tools work in the
part 2). They all capture the output of command-line programs in different
formats. `script` captures only the output and stores it as is. `autoexpect`
captures both inputs and outputs but it uses a backslash to quote certain
characters.

```
autoexpect -f vi.exp vi
```

Opening `vi.exp` in emacs shows many `expect` and `send` commands with very long
lines full of ANSI escape sequences. `expect` statement correspond to `vi`
output. `send` statements correspond to data that comes from tty (which is
either user input or xterm responses to commands which came from vi).

```
expect -exact "^[\[?2004h^[\[?1049h^[\[?1h ...
send -- "^[\[2;2R^[\[>85;95;0c^[\]10;rgb:5 ...
expect -exact "^[\[?25l^[\[2;1H^[\[38;5;12m~ ...
^[\[3;1H~ ...
^[\[4;1H~ ...
```

My first guess is that the first line is a terminal-independent part of vim's
initialization. Then there should be an ANSI escape sequence that asks xterm to
give its current configuration or its capabilities. Then xterm responds with
`^[\[2;2R^[\[>85;95;0c^[\]10;rgb:5 ...`. Initialization continues, and then vi
renders its interface with commands which jump to the beginning of each line
and then output its famous `~` symbol indicating a non-existing line.

```
^[\[3;1H~ ...
^[\[4;1H~ ...
```

Let's consult [xterm's user manual](https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h3-Functions-using-CSI-_-ordered-by-the-final-character_s)
to find out the meaning of ANSI escape sequences emitted by vim:

| Sequence      | Meaning                                                                        | Source of info |
|---------------|--------------------------------------------------------------------------------|----------------|
| `^[[?2004h`   | Set bracketed paste mode                                                       |                |
| `^[[?1049h`   | Save cursor as in DECSC and use Alternate Screen Buffer, clearing it first ... |                |
| `^[[?1h`      | Application Cursor Keys (DECCKM)                                               |                |
| `^[=`         | Application Keypad (DECPAM)                                                    |                |
| `^[[?2004h`   | Set bracketed paste mode                                                       |                |
| `^[[1;52r`    | Set Scrolling Region [top;bottom] (default = full size of window) (DECSTBM)    |                |
| `^[[?12h`     | Start Blinking Cursor (att610)                                                 |                |
| `^[[?12l`     | Stop Blinking Cursor (att610)                                                  |                |
| `^[[27m`      | Character Attributes (SGR) Positive (not inverse)                              |                |
| `^[[23m`      | set italic mode                                                                | the nice gist  |
| `^[[29m`      | set strikethrough mode                                                         | the nice gist  |
| `^[[m`        | Character Attributes (SGR) P s = 0 → Normal (default)                          |                |
| `^[[H`        | Cursor Position [row;column] (default = [1,1]) (CUP)                           |                |
| `^[[2J`       | Erase in Display (ED) P s = 2 → Erase All                                      |                |
| `^[[2;1H`     | Cursor Position [row;column] (default = [1,1]) (CUP)                           |                |
| `▽`           | plain text                                                                     |                |
| `^[[6n`       | Device Status Report (DSR) Report Cursor Position (CPR) [row;column] as        |                |
| `^[[2;1H`     | Cursor Position [row;column] (default = [1,1]) (CUP)                           |                |
| `  `          | plain text                                                                     |                |
| `^[[1;1H`     | Cursor Position [row;column] (default = [1,1]) (CUP)                           |                |
| `^[[>c`       | request the terminal’s identification code                                     |                |
| `^[]10;?^G`   | ask text foreground color                                                      |                |
| `^[]11;?^G`   | ask text background color                                                      |                |
| `^[[?25l`     | Hide Cursor (DECTCEM)                                                          |                |
| `^[[2;1H`     | Cursor Position [row;column] (default = [1,1]) (CUP)                           |                |
| `^[[38;5;12m` | Character Attributes (SGR) Set foreground color                                |                |
| `~`           | plain text                                                                     |                |

Some comments:
* "Application Cursor Keys" and "Application Keypad" configure what xterm sends
  when the user presses arrow keys or keypad keys;
* changing cursor position and character attributes should be self-explanatory;
* bracketed paste cause xterm to wrap data inserted from an X clipboard into
  `\ESC[200~` `\ESC[201~` so that the application can distinguish between keyboard input
  and copy-paste from the clipboard;
* "Alternate Screen Buffer" is a feature that helps vim to restore the screen after
  it exists; it's common for tui apps to enable alternative screen buffer and
  then disable it when they finish.

Existing apps are a useful source of information. Sometimes the easiest way to
answer your questions is to investigate the behavior and source code of an existing
tool. Digging through ANSI escape codes might seem complicated but with practice,
it quickly becomes easier. I wish there would be a tool that would create a
table above automatically ¯\_(ツ)_/¯.

### Conclusion

With disabled tty features, we've explored how keyboard input from users reaches
command-line tools. Then we've explored that the output of command-line tools is
a sequence of instructions for a terminal. We also saw that different tools
visualize non-printable characters differently, and we've improved our mental
resilience by getting accustomed to different notations and by trying different
tools. We also practiced analyzing the behavior of the terminal and command-line
tools so that we can figure out how things work and why they don't work as
expected.

In the part 2, we will explore tty behavior. Stay tuned :) 

