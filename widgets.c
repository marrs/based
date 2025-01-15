typedef struct table_widget_layout {
    int offset;
    int column_width;
} Table_Widget_Layout;

void table_widget(Data_Table *table, Cursor *cursor)
{
    Table_Widget_Layout table_layout = { 4, 20 };
    Data_Column *column = NULL;
    Data_Cell *cell = NULL;

    mvprintw(0, 0, "Column count: %d\n", table->col_count);
    loop(col_idx, table->col_count) {
        column = &table->column_data[col_idx];

        // Display table header.
        mvprintw(
                table_layout.offset,
                table_layout.column_width * col_idx,
                "  %s\n", column->name);

        // Display table data.
        cell = (Data_Cell *)column->dymem_cells->data;
        loop (idx, column->cell_count) {
            mvprintw(
                    table_layout.offset + 1 + idx,
                    table_layout.column_width * col_idx,
                    "  %s\n",
                    cell->str_data);
            ++cell;
        }
    }

    // Display table cursor.
    attron(COLOR_PAIR(1));
    loop(col_idx, table->col_count) {
        column = &table->column_data[col_idx];
        cell = (Data_Cell *)column->dymem_cells->data;
        cell += cursor->row;
        mvprintw(
                table_layout.offset + 1 + cursor->row,
                1 + table_layout.column_width * col_idx,
                " %s\n",
                cell->str_data);
    }
    attroff(COLOR_PAIR(1));
}

void help_widget()
{
    char help_msg[255] = "Options are (q)uit and (e)dit.\n";
    mvprintw(13, 4, help_msg);
}
