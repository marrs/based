#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <locale.h>

#include <ncurses.h>
#include <sqlite3.h>
#include <cyaml/cyaml.h>

#include "util.h"
#include "memory.h"
#include "event.h"
#include "model.h"
#include "view.h"

#include "util.c"
#include "memory.c"
#include "data-model.c"
#include "yaml.c"
#include "widgets.c"
#include "view.c"
