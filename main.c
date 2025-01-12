#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

#include <ncurses.h>

int main()
{
    char help_msg[255] = "Options are (q)uit and (e)dit.\n";
	initscr();			/* Start curses mode 		  */
	printw("Ncurses Test\n");	
	printw(help_msg);
	refresh();			/* Print it on to the real screen */
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
            } else { // parent
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
    return 0;
}
