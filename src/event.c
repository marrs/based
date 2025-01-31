void dispatch_app_event(Event event)
{
    endwin();
start:
    switch (event.id) {
        case APP_EVENT_LOAD_USER_TABLES: {
            char sql[] = "select schema, name, type, ncol, wr, strict "
                          "from pragma_table_list;";
            int err = new_table_with_query_using_sqlite(&global_app_state.user_tables.table, "Available tables", sql);
            //if (err) return; // TODO: Deal with this meaningfully.
            if (err) {
                printf("SQLITE ERROR: %s\n", sqlite3_errmsg(global_app_state.db));
                exit(1);
            }

            global_app_state.current_table_view = &global_app_state.user_tables;
            event = (Event){ APP_EVENT_VIEW_TABLE, DYTYPE_INT, .data_as_int = APP_VIEW_TABLE};

            goto start;
        } break;

        case APP_EVENT_LOAD_TABLE: {
            global_app_state.current_table_view = (Table_View *)vec_push_empty(global_app_state.loaded_table_vec);
            global_app_state.current_table_view->cursor = (View_Cursor){ 0, 0 };
            int err = new_table_with_data_using_sqlite(&global_app_state.current_table_view->table, event.data_as_text);
            if (err) return; // TODO: Deal with this meaningfully.

            event = (Event){ APP_EVENT_VIEW_TABLE, DYTYPE_INT, .data_as_int = APP_VIEW_TABLE};
            goto start;
        } break;

        case APP_EVENT_VIEW_TABLE:
            global_app_state.current_view = event.data_as_int;
            break;

        case APP_EVENT_CREATE_RECORD: {
            Data_Table *table = global_app_state.current_table_view->table;
            char *newrec_filename = "tmp_create_record.yml";
            FILE *file = fopen(newrec_filename, "w");
            if (!file) {
                // TODO: Handle error in UI.
                fprintf(stderr, "Failed to open temp yaml file for writing: %s\n", newrec_filename);
            }
            Vector_Iter *iter = new_vector_iter(table->column_vec);
            Data_Column *col = NULL;

            vec_loop (iter, Data_Column, col) {
                if (!col->is_read_only) {
                    fprintf(file, "%s:\n", col->name);
                }
            } delete_vector_iter(iter);
            fclose(file);
            event = (Event){ APP_EVENT_EDIT_FILE, DYTYPE_TEXT, .data_as_text = newrec_filename};
            goto start;
        } break;

        case APP_EVENT_EDIT_FILE: {
            pid_t pid = fork();

            if (0 == pid) { // child
                char *args[] = {"/usr/bin/vim", event.data_as_text, NULL};
                execvp(args[0], args);
                // TODO:
                // - Parse results on file save
                // - Identify fields without required data (inc. foreign records)
            } else {        // parent
                int status;
                waitpid(pid, &status, 0);
                printw("Child process finished\n");
            }
        } break;

        case APP_EVENT_REFRESH_VIEW: break;
    }

    switch (global_app_state.current_view) {

        case APP_VIEW_TABLE: {
            View_Table_Model viewmodel = {
                .cursor = &global_app_state.current_table_view->cursor,
                .table = global_app_state.current_table_view->table,
            };
            enable_curses();
            view_table(viewmodel);
        } break;

        default:
            fprintf(stderr, "Error: No view renderer for %s\n", global_app_state.current_view);
    }
}

void dispatch_ui_event(Event event)
{
start:
    switch (event.id) {
        case UI_EVENT_KEY_PRESS:
            switch (event.data_as_char) {
            case 'j':
                event = plain_event(UI_EVENT_CURSOR_DOWN);
                goto start;
                break;

            case 'k':
                event = plain_event(UI_EVENT_CURSOR_UP);
                goto start;
                break;

            case 'l':
                event = plain_event(UI_EVENT_CURSOR_RIGHT);
                goto start;
                break;

            case 'c':
                dispatch_app_event(plain_event(APP_EVENT_CREATE_RECORD));
                break;

            case 'q': 
                shut_down(global_app_state.db);
                break;

            case 'e':
                dispatch_app_event(plain_event(APP_EVENT_EDIT_FILE));
                break;
            }
            break;

        case UI_EVENT_CURSOR_UP: {
            if (global_app_state.current_table_view->cursor.row > 0) {
                --global_app_state.current_table_view->cursor.row;
            }
            dispatch_app_event(plain_event(APP_EVENT_REFRESH_VIEW));
        } break;

        case UI_EVENT_CURSOR_DOWN: {
            if (global_app_state.current_table_view->cursor.row
                    < global_app_state.current_table_view->table->row_count -1) {
                ++global_app_state.current_table_view->cursor.row;
            }
            dispatch_app_event(plain_event(APP_EVENT_REFRESH_VIEW));
        } break;

        case UI_EVENT_CURSOR_RIGHT: {
            Data_Column *col
                = (Data_Column *)
                  vec_seek(global_app_state.current_table_view->table->column_vec, 1);

            Data_Cell *cell
                = (Data_Cell *)
                  vec_seek(
                          col->cell_vec,
                          global_app_state.current_table_view->cursor.row
                          );
            event = (Event){
                APP_EVENT_LOAD_TABLE,
                DYTYPE_TEXT,
                .data_as_text = cell->str_data,
            };
            dispatch_app_event(event);
        } break;

    }
}
