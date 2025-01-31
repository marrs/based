int is_ws_char(char x)
{
    return x == '\t' || x == '\n' || x == ' ';
}

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
