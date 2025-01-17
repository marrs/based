#define KB(x) 1000 * (x)
#define MB(x) 1000 * KB(x)

#define TEXT_LEN_FOR_LARGEST_INT 20

// Unverified. See https://stackoverflow.com/a/1701085
#define TEXT_LEN_FOR_LARGEST_FLOAT 24

#define return_on_err(x) if ((x)) return;
#define loop(x, count) for (int (x) = 0; (x) < (count); (++x))

int str_starts_with(char *str, const char *prefix)
{
    return (0 == strncmp(prefix, str, strlen(prefix)));
}

void enable_curses()
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLUE);  // Cursor colours.
    clear();
    noecho();
}
