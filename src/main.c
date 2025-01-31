#include "includes.h"

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

#include "event.c"

int main(int argc, char **argv)
{

    setlocale(LC_ALL, "");

    // Init app state:
    global_app_state.current_view = APP_VIEW_TABLE;

    global_app_state.user_tables.cursor = (View_Cursor){ 0, 0 };
    global_app_state.user_tables.table = NULL;

    global_app_state.loaded_table_vec = new_vector(sizeof(Table_View), 20);

    global_app_state.current_table_view = &global_app_state.user_tables;

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
    dispatch_app_event(event);

    char input_ch;
    while (input_ch = getch()) {
        event = (Event){ UI_EVENT_KEY_PRESS, DYTYPE_CHAR, .data_as_char = input_ch };
        dispatch_ui_event(event);
    }

exit:
    return shut_down(db);
}
