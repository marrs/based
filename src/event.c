//void load_table_with_data(target_table, query)
int load_table_with_data(Data_Table **target_table, char *table_name, char *sql)
{
    // Query data.
    sqlite3 *db = global_app_state.db;
    sqlite3_stmt *stmt;

    int err = prepare_query_for_user_tables(db, &stmt, sql);
    if (err) return err;

    // Populate models.
    int col_count = sqlite3_column_count(stmt);
    Data_Table *table = new_data_table_from_table_pool(global_table_pool, table_name, col_count);
    populate_data_table_from_sqlite(table, db, stmt);
    *target_table = table;

    // Cleanup
    sqlite3_finalize(stmt);
}

void dispatch_event(Event event)
{
    endwin();
start:
    switch (event.id) {
        case APP_EVENT_LOAD_USER_TABLES: {
            char sql[255] = "select schema, name, type, ncol, wr, strict "
                            "from pragma_table_list;";
            int err = load_table_with_data(&global_app_state.user_tables.table, "Available tables", sql);
            if (err) return; // TODO: Deal with this meaningfully.

            global_app_state.current_table = &global_app_state.user_tables;
            event = (Event){ APP_EVENT_VIEW_TABLE, DYTYPE_INT, .data_as_int = APP_VIEW_TABLE};
            goto start;
        } break;

        case APP_EVENT_LOAD_SELECTED_TABLE: {
            char sql[255];
            sprintf(sql, "select * from %s;", event.data_as_text);
            int err = load_table_with_data(&global_app_state.selected_table.table, event.data_as_text, sql);
            if (err) return; // TODO: Deal with this meaningfully.

            global_app_state.current_table = &global_app_state.selected_table;
            event = (Event){ APP_EVENT_VIEW_TABLE, DYTYPE_INT, .data_as_int = APP_VIEW_TABLE};
            goto start;
        } break;

        case APP_EVENT_VIEW_TABLE:
            global_app_state.current_view = event.data_as_int;
            break;

        case UI_EVENT_CURSOR_UP: {
            if (global_app_state.current_table->cursor.row > 0) {
                --global_app_state.current_table->cursor.row;
            }
        } break;

        case UI_EVENT_CURSOR_DOWN: {
            if (global_app_state.current_table->cursor.row < global_app_state.current_table->table->row_count -1) {
                ++global_app_state.current_table->cursor.row;
            }
        } break;

        case UI_EVENT_CURSOR_RIGHT: {
            Data_Column *col = (Data_Column *)vec_seek(global_app_state.current_table->table->column_vec, 1);
            Data_Cell *cell = (Data_Cell *)vec_seek(col->cell_vec, global_app_state.current_table->cursor.row);
            event = (Event){
                APP_EVENT_LOAD_SELECTED_TABLE,
                DYTYPE_TEXT,
                .data_as_text = cell->str_data,
            };
            goto start;
        } break;
    }

    switch (global_app_state.current_view) {

        case APP_VIEW_TABLE: {
            View_Table_Model viewmodel = {
                .cursor = &global_app_state.current_table->cursor,
                .table = global_app_state.current_table->table,
            };
            enable_curses();
            view_table(viewmodel);
        } break;

        default:
            fprintf(stderr, "Error: No view renderer for %s\n", global_app_state.current_view);
    }
}
