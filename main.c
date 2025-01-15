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

void populate_data_table_from_sqlite(Data_Table *datatable, sqlite3 *db, sqlite3_stmt *stmt)
{

    int status = 0;
    Data_Cursor cursor = { datatable, 0, 0 };
    Data_Cell *datacell = NULL;

    // Populate column names
    loop (idx, datatable->col_count) {
        // TODO:
        // sqlite3_column_name(stmt, idx);
    }

    // Populate data
    for (cursor.col_idx = 0; status = sqlite3_step(stmt); ++cursor.col_idx) {
        if (SQLITE_ROW == status) {

            for (cursor.row_idx = 0;
            cursor.row_idx < datatable->col_count;
            ++cursor.row_idx) {
                Data_Column *column = &datatable->column_data[cursor.row_idx];
                datacell = (Data_Cell *)dymem_allocate(column->dymem_cells, sizeof(Data_Cell));
                ++column->cell_count;

                init_data_cell_from_sqlite_row(datacell, stmt, cursor.row_idx);
            }
        } else {
            switch (status) {
            case SQLITE_DONE: printw("Success: Done.\n"); break;
            case SQLITE_MISUSE:
                printw("Error (SQLITE_MISUSE) stepping through statement: %s\n", sqlite3_errmsg(db));
                break;
            default:
                printw("Error (%d) stepping through statement: %s\n", status, sqlite3_errmsg(db));
            }
            break;
        }
    }
}

void ui_show_user_tables(sqlite3 *db)
{
    sqlite3_stmt *stmt;

    char sql[255] = "select schema, name, type, ncol, wr, strict "
                    "from pragma_table_list;";
    return_on_err(prepare_query_for_user_tables(db, &stmt, sql));

    int col_count = sqlite3_column_count(stmt);

    Data_Table *datatable = new_data_table(col_count);
    populate_data_table_from_sqlite(datatable, db, stmt);
    printw("col count: %d\n", col_count);
    /*
    for (int idx = 0; idx < col_count; ++idx) {
        printw("  %s ", sqlite3_column_name(stmt, idx));
    }
    printw("\n");
    refresh();
    */

    refresh();
    sqlite3_finalize(stmt);
}

int main(int argc, char **argv)
{
    sqlite3 *db = NULL;
    int err = 0;

    global_db_str_mem = dymem_init(MB(2));
    global_db_bin_mem = dymem_init(MB(2));

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

    char help_msg[255] = "Options are (q)uit and (e)dit.\n";
	initscr();          /* Start curses mode 		  */

    printw("Ncurses Test\n");

    // TODO:
    // - Then we will load data into our new struct from sqlite.
    // - Then we will render the data to the screen.

    ui_show_user_tables(db);

    printw(help_msg);
    refresh();          /* Print it on to the real screen */
    char input_ch;
    while (input_ch = getch()) {
        switch (input_ch) {
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
            clear();
            printw("Editing finished\n");
            printw(help_msg);
        } break;
        default: {
            printw(help_msg);
        }
        }
    }

exit:
    return shut_down(db);
}
