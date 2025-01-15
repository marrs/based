void view_user_tables(enum app_event event)
{
    Cursor *cursor = &global_app_state.user_tables.cursor;
    Data_Table *table = global_app_state.user_tables.table;

    switch (event) {
        case EVENT_CURSOR_UP:
            if (cursor->row > 0) {
                --cursor->row;
            }
            break;

        case EVENT_CURSOR_DOWN:
            if (cursor->row < table->column_data->cell_count -1) {
                ++cursor->row;
            }
            break;
        default:
            //cursor->row = 0;
    }

    table_widget(table, cursor);
    help_widget();

    refresh();
}
