#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

#include <ncurses.h>
#include <sqlite3.h>

void ui_show_tables(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    int err = 0;
    char sql[255] = "pragma table_list;";
    err = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (err) {
        printw("Error preparing statement: %s\n", sqlite3_errmsg(db));
    } else {
        err = sqlite3_step(stmt);
        switch (err) {
            case SQLITE_DONE: {
                printw("Success: Done.\n");
            } break;
            case SQLITE_ROW: {
                printw("Success: Row returned.\n");
            } break;
            case SQLITE_MISUSE: {
                printw("Error (SQLITE_MISUSE) stepping through statement: %s\n", sqlite3_errmsg(db));
            } break;
            default: {
                printw("Error (%d) stepping through statement: %s\n", err, sqlite3_errmsg(db));
            }
        }
    }
    refresh();
    sqlite3_finalize(stmt);
}

int main(int argc, char **argv)
{
    sqlite3 *db = NULL;
    int err = 0;

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
    ui_show_tables(db);

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
    endwin();
    printf("Closing database connection...");
    if (err = sqlite3_close_v2(db)) {
        fprintf(stderr, "failed:\n  %s\n", sqlite3_errmsg(db));
    } else {
        printf("done.\n");
    }
    return 0;
}
