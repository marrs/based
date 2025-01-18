#!/usr/bin/sh

src="${1:-src/main.c}"

#gcc $(pkg-config --cflags sqlite3) \
#    $(pkg-config --cflags ncurses) \
#    -g -fsanitize=address \
#    -obin main.c \
#    $(pkg-config --libs ncurses) \
#    $(pkg-config --libs sqlite3)

gcc $(pkg-config --cflags sqlite3) \
    $(pkg-config --cflags ncurses) \
    -g \
    -obin "$src" \
    $(pkg-config --libs ncurses) \
    $(pkg-config --libs sqlite3)
