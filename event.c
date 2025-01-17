Table_View *table_under_current_view()
{
    switch (global_app_state.current_view) {
        case APP_VIEW_USER_TABLES:
            return &global_app_state.user_tables;
        case APP_VIEW_SELECTED_TABLE:
            return &global_app_state.selected_table;
    }
}

//void load_table_with_data(target_table, query)
int load_table_with_data(Data_Table **target_table, char *sql)
{
    // Query data.
    sqlite3 *db = global_app_state.db;
    sqlite3_stmt *stmt;

    int err = prepare_query_for_user_tables(db, &stmt, sql);
    if (err) return err;

    // Populate models.
    int col_count = sqlite3_column_count(stmt);
    printf("COL COUNT %d\n", col_count);
    Data_Table *table = mem_pool_allocate_data_table(global_mempool, col_count);
    populate_data_table_from_sqlite(table, db, stmt);
    *target_table = table;

    // Cleanup
    sqlite3_finalize(stmt);
}

void dispatch_event(Event event)
{
    Table_View *table_view = table_under_current_view();
    View_Cursor *cursor = &table_view->cursor;

start:
    switch (event.id) {
        case APP_EVENT_LOAD_USER_TABLES: {
            char sql[255] = "select schema, name, type, ncol, wr, strict "
                            "from pragma_table_list;";
            int err = load_table_with_data(&global_app_state.user_tables.table, sql);
            if (err) return; // TODO: Deal with this meaningfully.

            event = (Event){ APP_EVENT_VIEW_TABLE, DYTYPE_INT, .data_as_int = APP_VIEW_USER_TABLES};
            goto start;
        } break;

        case APP_EVENT_LOAD_SELECTED_TABLE: {
            char sql[255];
            sprintf(sql, "select * from %s;", event.data_as_text);
            int err = load_table_with_data(&global_app_state.selected_table.table, sql);
            if (err) return; // TODO: Deal with this meaningfully.

            event = (Event){ APP_EVENT_VIEW_TABLE, DYTYPE_INT, .data_as_int = APP_VIEW_SELECTED_TABLE};
            goto start;
        } break;

        case APP_EVENT_VIEW_TABLE:
            global_app_state.current_view = event.data_as_int;

            table_view = table_under_current_view();
            cursor = &table_view->cursor;
            break;

        case UI_EVENT_CURSOR_UP: {
            if (cursor->row > 0) {
                --cursor->row;
            }
        } break;

        case UI_EVENT_CURSOR_DOWN: {
            if (cursor->row < table_view->table->columns->cell_count -1) {
                ++cursor->row;
            }
        } break;

        case UI_EVENT_CURSOR_RIGHT: {
            event = (Event){
                APP_EVENT_LOAD_SELECTED_TABLE,
                DYTYPE_TEXT,
                .data_as_text = table_view->table->columns[1].cells[cursor->row].str_data,
            };
    endwin();
            goto start;
        } break;
    }

    switch (global_app_state.current_view) {
        case APP_VIEW_USER_TABLES: {
            View_Table_Model viewmodel = {
                .table_name = "User Tables",
                .cursor = cursor,
                .table = table_view->table,
            };
            view_table(viewmodel);
        } break;

        case APP_VIEW_SELECTED_TABLE: {
            View_Table_Model viewmodel = {
                .table_name = event.data_as_text,
                .cursor = cursor,
                .table = table_view->table,
            };
            view_table(viewmodel);
        } break;

        default:
            fprintf(stderr, "Error: No view renderer for %s\n", global_app_state.current_view);
    }
}
