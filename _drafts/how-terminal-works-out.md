---
layout: post
title:  "How terminal works. Part 2: Xterm, CLI tools output"
categories: terminal
---

* TOC
{:toc}

## Bash output

In this blog post we will discuss what terminal emulators emulate.

Terminals (either hardware devices or software programs) perform two main tasks:
* visualize the output of tools like bash, cat, top, vi, etc. to the user;
* pass user input to command-line tools.

We've already discussed how the terminal handles user input. Now let's discuss how
it visualizes bash output. xterm maintains state of a "scene" (a buffer) and
exposes API to modify that state. API is centered around a concept of a cursor
and instructions which modify the buffer. For example:
* insert a character in the current position and move the cursor to the right (I
  am sorry for [RTL scripts](https://en.wikipedia.org/wiki/Right-to-left_script));
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

## History

My current understanding is that `longlong` ago terminal vendors where
implementing terminals with similar features, but they implemented different
APIs. This led to the creation of terminfo - a database of terminal features
which led to the creation of software terminal compatibility libraries such as
ncurses. At some point industry matured and produced an ANSI standard that
standardized terminal API and gave us words ANSI-compatible terminal and
ANSI-escape sequences. The first popular terminal that supported the ANSI
standard was VT100. VT100 was so successful that it made ANSI not only a standard
on a paper but also a de-facto standard. That's why nowadays by saying
"VT100-compatible terminal" and "ANSI-compatible terminal" some people mean the
same thing. Modern command line tools sometimes aren't concerned with
compatibility and they might not use terminfo. Instead, they expect an
ANSI-compatible terminal emulator and assume support for certain features.

The evolution of terminals didn't stop after the release of the ANSI standard. Modern
terminal emulators continue to invent new features which have a different level of
adoption. Here are examples of features with different level of support:

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

* focusing on modern software terminal emulators makes life easier;

  modern major terminal emulators are ANSI compatible and hence:
  * they support similar syntax for ANSI escape sequences (with differences in how
    errors are handled and how arguments for non-standard commands are parsed,
    a good example is [truecolor escape
    codes](https://github.com/termstandard/colors));
  * they support many similar features, so by using widely supported features
    and writing enough "if" conditions one can achieve terminal-independent code
    for modern terminals.

## Practice 1: ask xterm to execute commands

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

## Practice 2: Explore vim output

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
renders its interface using commands which jump to the beginning of each line
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

I wish there would be a tool that would create a table above automatically ¯\_(ツ)_/¯.

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
answer your questions is to investigate the behavior or source code of an
existing tool. Digging through ANSI escape codes might seem complicated but with
practice, it quickly becomes easier. One also can consult source code of
existing terminal emulators, one good example is recently created
[Alacritty](https://github.com/alacritty/alacritty/blob/master/alacritty_terminal/src/ansi.rs#L1085).

## Conclusion

With disabled tty features, we've explored how keyboard input from users reaches
command-line tools. Then we've explored that the output of command-line tools is
a sequence of instructions for a terminal. We also saw that different tools
visualize non-printable characters differently, and we've improved our mental
resilience by getting accustomed to different notations and by trying different
tools. We also practiced analyzing the behavior of the terminal and command-line
tools so that we can figure out how things work and why they don't work as
expected.

In the part 2, we will explore tty behavior. Stay tuned :) 

