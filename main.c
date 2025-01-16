#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

#include <ncurses.h>
#include <sqlite3.h>

#include "util.c"
#include "memory.c"
#include "data-model.c"
#include "app-model.c"

#include "widgets.c"
#include "view.c"

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
    exit(0);
}

int str_starts_with(char *str, const char *prefix)
{
    return (0 == strncmp(prefix, str, strlen(prefix)));
}

int prepare_query_for_user_tables(sqlite3 *db, sqlite3_stmt **stmt, char *sql)
{
    int err = 0;
    err = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
    if (err) {
        printw("Error preparing statement: %s\n", sqlite3_errmsg(db));
    }
    return err;
}

void app_event_dispatcher(enum app_event evt)
{
    switch (global_app_state.current_view) {
        case APP_VIEW_USER_TABLES:
            view_user_tables(evt);
            break;

        default:
            fprintf(stderr, "Error: No view renderer for %s\n", global_app_state.current_view);
    }
}

void ui_event_dispatcher(char key)
{
    enum app_event event;
    switch(key) {
        case 'j':
            event = EVENT_CURSOR_DOWN;
            break;

        case 'k':
            event = EVENT_CURSOR_UP;
            break;
        default: return;
    }

    app_event_dispatcher(event);
}

int main(int argc, char **argv)
{

    // Init app state:
    global_app_state.current_view = APP_VIEW_USER_TABLES;
    global_app_state.user_tables.table = NULL;

    sqlite3 *db = NULL;
    int err = 0;
    Cursor cursor = { 0 };

    App_Memory_Pool *mempool = (App_Memory_Pool *)malloc(sizeof(App_Memory_Pool));
    init_app_memory_pool(mempool);

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

    printw("Ncurses Test\n");

    // Query db for user tables
    {
        // Query data.
        sqlite3_stmt *stmt;
        char sql[255] = "select schema, name, type, ncol, wr, strict "
                        "from pragma_table_list;";

        err = prepare_query_for_user_tables(db, &stmt, sql);
        if (err) goto exit;

        // Populate models.
        int col_count = sqlite3_column_count(stmt);
        Data_Table *table = mem_pool_allocate_data_table(mempool, col_count);
        populate_data_table_from_sqlite(table, db, stmt);
        global_app_state.user_tables.table = table;

        // Cleanup
        sqlite3_finalize(stmt);
    }

    // Init curses
    initscr();
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_BLUE);  // Cursor colours.
    clear();
    noecho();

    app_event_dispatcher(EVENT_INIT_VIEW);

    char input_ch;
    while (input_ch = getch()) {
        switch (input_ch) {
            case 'j':
            case 'k':
                ui_event_dispatcher(input_ch);
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
        }
    }

exit:
    return shut_down(db);
}
