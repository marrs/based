Table_View *table_under_current_view()
{
    switch (global_app_state.current_view) {
        case APP_VIEW_USER_TABLES:
            return &global_app_state.user_tables;
        case APP_VIEW_SELECTED_TABLE:
            return &global_app_state.selected_table;
    }
}

void dispatch_event(Event event)
{
    Table_View *table_view = table_under_current_view();
    View_Cursor *cursor = &table_view->cursor;

start:
    switch (event.id) {
        case EVENT_VIEW_TABLE:
            global_app_state.current_view = APP_VIEW_SELECTED_TABLE;

            table_view = table_under_current_view();
            cursor = &table_view->cursor;
            break;

        case EVENT_CURSOR_UP: {
            if (cursor->row > 0) {
                --cursor->row;
            }
        } break;

        case EVENT_CURSOR_DOWN: {
            if (cursor->row < table_view->table->columns->cell_count -1) {
                ++cursor->row;
            }
        } break;

        case EVENT_CURSOR_RIGHT: {
            if (cursor->row < table_view->table->columns->cell_count -1) {
                //Event event = (Event){ EVENT_VIEW_TABLE, DYTYPE_TEXT, cell_value(cursor) };
                event = (Event){ EVENT_VIEW_TABLE, DYTYPE_TEXT, .data_as_text = "table_name" };
                goto start;
            }
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
