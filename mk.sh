#!/usr/bin/sh

gcc $(pkg-config --cflags sqlite3) \
    $(pkg-config --cflags ncurses) \
    -obin main.c \
    $(pkg-config --libs ncurses) \
    $(pkg-config --libs sqlite3)
