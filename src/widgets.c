typedef struct table_widget_layout {
    int offset;
    int column_width;
} Table_Widget_Layout;

void table_widget(Data_Table *table, View_Cursor *cursor)
{
    Table_Widget_Layout table_layout = { 4, 20 };
    Data_Column *column = NULL;
    Data_Cell *cell = NULL;

    mvprintw(0, 0, "%d columns, %d rows.\n", table->col_count, table->row_count);
    int col_idx = 0;
    Vector_Iter *col_iter = new_vector_iter(table->column_vec);
    vec_loop (col_iter, Data_Column, column) {
        // Display table header.
        char pk_symbol[] = "(PK) ";
        char fk_symbol[] = "(FK) ";
        // TODO:
        // - Enable utf8 so that the key symbol can be printed
        //   - u8"🔑";
        //   - "\0x1F511";
        attron(A_BOLD);
            mvprintw(
                    table_layout.offset,
                    table_layout.column_width * col_idx,
                    "  %s%s%s\n",
                    column->is_pk? pk_symbol : "",
                    NULL != column->fk_table? fk_symbol : "",
                    column->name);
        attroff(A_BOLD);

        // Display table data.
        int cell_idx = 0;
        Vector_Iter *cell_iter = new_vector_iter(column->cell_vec);
        vec_loop (cell_iter, Data_Cell, cell) {
            mvprintw(
                    table_layout.offset + 1 + cell_idx,
                    table_layout.column_width * col_idx,
                    "  %s\n",
                    cell->str_data);
            ++cell_idx;
        } delete_vector_iter(cell_iter);
        ++col_idx;
    } reset_vector_iter(col_iter);

    // Display table cursor.
    attron(COLOR_PAIR(1));
        col_idx = 0;
        if (table->row_count) {
            vec_loop (col_iter, Data_Column, column) {
                cell = (Data_Cell *)vec_seek(column->cell_vec, cursor->row);
                mvprintw(
                        table_layout.offset + 1 + cursor->row,
                        1 + table_layout.column_width * col_idx,
                        " %s\n",
                        cell->str_data);
                ++col_idx;
            }
        }
        delete_vector_iter(col_iter);
    attroff(COLOR_PAIR(1));
}

void status_bar_widget(char *msg)
{
    mvprintw(13, 4, msg);
}
