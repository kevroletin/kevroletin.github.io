#!/bin/bash

set -e

pts=$1
if [ -z $1 ]; then
    echo -e "\nUsage: $0 /dev/pts/<n>\n";
    exit;
fi
echo $pts

stty -icanon -echo

printf "\x1b7" > $pts # save cursor position

function cleanup {
    printf "\x1b8" > $pts # restore saved cursor position
}

trap cleanup EXIT

while true; do
    char=$(dd bs=1 count=1 2> /dev/null)   # read a single character
    case $char in
        [wk] ) cmd="A" ;;                  # move cursor up
        [sj] ) cmd="B" ;;                  # move cursor down
        [ah] ) cmd="D" ;;                  # move cursor left
        [dl] ) cmd="C" ;;                  # move cursor right
        . ) printf " " > $pts; cmd="D" ;;
        " ") printf "x" > $pts; cmd="D" ;;
        "" ) exit; ;;
        * ) continue ;;  # erase till end of line
    esac
    printf "\x1b[$cmd" > $pts
done
