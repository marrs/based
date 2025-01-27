#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#include <ncurses.h>
#include <sqlite3.h>

#include "util.h"
#include "memory.h"
#include "event.h"
#include "model.h"
#include "view.h"

#include "util.c"
#include "memory.c"
#include "data-model.c"
#include "widgets.c"
#include "view.c"
#include "event.c"

int shut_down(sqlite3 *db)
{
    int err = 0;
    printf("Closing database connection...");
    if (err = sqlite3_close_v2(db)) {
        fprintf(stderr, "failed:\n  %s\n", sqlite3_errmsg(db));
    } else {
        printf("done.\n");
    }
    endwin();
    exit(err);
}

int main(int argc, char **argv)
{
    // Init app state:
    global_app_state.current_view = APP_VIEW_TABLE;

    global_app_state.user_tables.cursor = (View_Cursor){ 0, 0 };
    global_app_state.user_tables.table = NULL;

    global_app_state.selected_table.cursor = (View_Cursor){ 0, 0 };
    global_app_state.selected_table.table = NULL;

    global_app_state.current_table = &global_app_state.user_tables;

    sqlite3 *db = NULL;
    int err = 0;

    global_table_pool = (Table_Pool *)malloc(sizeof(Table_Pool));
    init_table_pool(global_table_pool);

    if (argc > 1) {
        err = sqlite3_open_v2(argv[1], &db, SQLITE_OPEN_READONLY, 0);
        if (err) {
            fprintf(stderr, "Error opening %s: %s\n", argv[1], sqlite3_errmsg(db));
            goto exit;
        }
    } else {
        fprintf(stderr, "Exiting: no sqlite database provided\n");
        goto exit;
    }
    global_app_state.db = db;

    enable_curses();

    Event event = (Event){ APP_EVENT_LOAD_USER_TABLES, DYTYPE_NULL, .data_as_null = NULL };
    dispatch_event(event);

    char input_ch;
    while (input_ch = getch()) {
        switch (input_ch) {
            case 'j':
                event = (Event){ UI_EVENT_CURSOR_DOWN, DYTYPE_NULL, .data_as_null = NULL };
                dispatch_event(event);
                break;

            case 'k':
                event = (Event){ UI_EVENT_CURSOR_UP, DYTYPE_NULL, .data_as_null = NULL };
                dispatch_event(event);
                break;

            case 'l':
                event = (Event){ UI_EVENT_CURSOR_RIGHT, DYTYPE_NULL, .data_as_null = NULL };
                dispatch_event(event);
                break;

            case 'q': {
                  goto exit;
            } break;
            case 'e': {
                pid_t pid = fork();

                if (pid == 0) { // child
                    char *args[] = {"/usr/bin/vim", NULL};
                    execvp(args[0], args);
                } else {        // parent
                    int status;
                    waitpid(pid, &status, 0);
                    printw("Child process finished\n");
                }
            } break;
            default: continue;
        }
    }

exit:
    return shut_down(db);
}
