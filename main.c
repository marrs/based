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

void view_table(Data_Table *table)
{
    printw("Column count: %d\n", table->col_count);
    loop(col_idx, table->col_count) {
        Data_Column *column = &table->column_data[col_idx];
        mvprintw(4, 20 * col_idx, "  %s\n", column->name);
        Data_Cell *cell = (Data_Cell *)column->dymem_cells->data;
        loop (idx, column->cell_count) {
            mvprintw(5 + idx, 20 * col_idx, "  %s\n", cell->str_data);
            ++cell;
        }
    }
    printw("\n");

    refresh();
}

void ui_show_user_tables(sqlite3 *db)
{
    // Query data.
    sqlite3_stmt *stmt;
    char sql[255] = "select schema, name, type, ncol, wr, strict "
                    "from pragma_table_list;";
    return_on_err(prepare_query_for_user_tables(db, &stmt, sql));

    // Populate data.
    int col_count = sqlite3_column_count(stmt);
    Data_Table *table = new_data_table(col_count);
    populate_data_table_from_sqlite(table, db, stmt);

    // Display data.
    view_table(table);

    // Cleanup
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
