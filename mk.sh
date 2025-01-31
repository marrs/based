#!/usr/bin/sh

src="${1:-src/main.c}"

# Flag for memory info in stderr
    # -g -fsanitize=address \

gcc $(pkg-config --cflags sqlite3) \
    $(pkg-config --cflags ncurses) \
    $(pkg-config --cflags libcyaml) \
    -g -fsanitize=address \
    -obin "$src" \
    $(pkg-config --libs ncurses) \
    $(pkg-config --libs sqlite3) \
    $(pkg-config --libs libcyaml) \

#gcc $(pkg-config --cflags sqlite3) \
#    $(pkg-config --cflags ncurses) \
#    $(pkg-config --cflags libcyaml) \
#    -g \
#    -obin "$src" \
#    $(pkg-config --libs ncurses) \
#    $(pkg-config --libs sqlite3) \
#    $(pkg-config --libs libcyaml) \
