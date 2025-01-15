#!/usr/bin/sh

gcc $(pkg-config --cflags sqlite3) \
    $(pkg-config --cflags ncurses) \
    -g -fsanitize=address \
    -obin main.c \
    $(pkg-config --libs ncurses) \
    $(pkg-config --libs sqlite3)
