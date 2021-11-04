.default &: create_tty_1 create_tty_2

create_tty_1 : create_tty_1.c
	gcc --pedantic -g create_tty_1.c -l util -o create_tty_1

create_tty_2 : create_tty_2.c
	gcc --pedantic -g create_tty_2.c -l util -o create_tty_2
