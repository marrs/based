/* Data Model
 * ==========
 *
 *  Objects associated with storing and managing the
 *  data fetched from various resources used by the app.
 *
 *  Data_Table
 *  ----------
 *
 *  Supports the view for rendering tabular data.
 *
 *  Optimised for reading data by column, not row.
 *  This is preferred because table layout will be based on
 *  column width.
 *
 *  Data is fetched from the SQL database and then transferred to
 *  an internal memory store.  This consists of 2 global dymem buffers,
 *  one for the original data, and one for the string representation.
 *  Text is stored in the string buffer.
 *
 *  Refernces to these data are found in Data_Cell objects.  These are
 *  stored in an array representing their columns.
 *
 *  While the Data_Cell objects are stored contiguously by column, their
 *  data are stored by row.  This is because there is only one memory buffer
 *  and data fetched from the database is written row by row.
 *  It may improve performance to group the data by column as well.
 *  This would require a pool of buffers, one per column.
 *
 *  Currently, the db data are stored in global buffers, while the Data_Cell
 *  indices are stored in buffers located in the Data_Column objects.
 *
 *  This is inconsistent and confusing.  I'm considering a single memory
 *  layer (architecturally speaking) to handle the storage of all data, with
 *  the the Data_* structs sitting above.  Memory would be accessed via the
 *  structs, but managed by the data layer.
 *
 *  We also have some RAII (currently denoted by functions prefixed with
 *  `new_`, but sometimes also `init_`(!), which I want to get rid of.
 *
 *  The cell value type will be stored in Data_Cell in case
 *  we wish to work with the original types.
 *
 *  From https://sqlite.org/c3ref/c_blob.html:
 *    Every value in SQLite has one of five fundamental datatypes:
 *
 *     64-bit signed integer
 *     64-bit IEEE floating point number
 *     string
 *     BLOB
 *     NULL
 */

void init_app_memory_pool(App_Memory_Pool *pool)
{
    pool->table_count = 0;
    pool->dymem_tables = dymem_init(sizeof(Data_Table));
    pool->tables = (Data_Table *)pool->dymem_tables->data;
}

Data_Table *mem_pool_allocate_data_table(App_Memory_Pool *pool, int col_count)
{
    Data_Table *table = (Data_Table *)dymem_allocate(pool->dymem_tables, sizeof(Data_Table));
    ++pool->table_count;

    table->col_count = col_count;
    table->data_mem = (Data_Memory *)dymem_init(MB(2));
    table->data_mem->dymem_bin_data = dymem_init(MB(2));
    table->data_mem->dymem_str_data = dymem_init(MB(2));
    table->data_mem->dymem_meta_data = dymem_init(KB(1));
    table->dymem_columns = dymem_init(sizeof(Data_Column) * 10);
    table->columns = (Data_Column *)table->dymem_columns->data;
    return table;
}

Data_Column *data_table_allocate_column(Data_Table *table, int col_name_len)
{
    Data_Column *column = (Data_Column *)dymem_allocate(table->dymem_columns, sizeof(Data_Column));
    column->name = dymem_allocate(
            table->data_mem->dymem_meta_data,
            col_name_len + 1);
    column->dymem_cells = dymem_init(sizeof(Data_Cell) * 200);
    column->cells = (Data_Cell *)column->dymem_cells->data;

    return column;
}

Data_Cell *data_column_allocate_cell(Data_Column *column)
{
    Data_Cell *cell = (Data_Cell *)dymem_allocate(column->dymem_cells, sizeof(Data_Cell));
    ++column->cell_count;

    return cell;
}

// TODO: Implement way to release memory.
Data_Cell *init_data_cell_from_sqlite_row(
        Data_Memory *mem,
        Data_Cell *datacell,
        sqlite3_stmt *stmt,
        int col_idx)
{

    datacell->type = sqlite3_column_type(stmt, col_idx);

    switch (datacell->type) {
        case SQLITE_NULL:
            datacell->type = DYTYPE_NULL;

            datacell->str_size = 4;
            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->str_size + 1);
            strcpy(datacell->str_data, "NULL");

            datacell->raw_size = 0;
            datacell->raw_data = NULL;
            break;

        case SQLITE_INTEGER:
            datacell->type = DYTYPE_INT;

            datacell->raw_size = sizeof(int);
            datacell->raw_data = (int *)dymem_allocate(mem->dymem_bin_data, datacell->raw_size);
            *(int *)datacell->raw_data = sqlite3_column_int(stmt, col_idx);

            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, TEXT_LEN_FOR_LARGEST_INT);
            strcpy(datacell->str_data, sqlite3_column_text(stmt, col_idx));
            datacell->str_size = strlen(datacell->str_data);
            break;

        case SQLITE_FLOAT:
            datacell->type = DYTYPE_FLOAT;

            datacell->raw_size = sizeof(double);
            datacell->raw_data = (double *)dymem_allocate(mem->dymem_bin_data, datacell->raw_size);
            *(double *)datacell->raw_data = sqlite3_column_double(stmt, col_idx);

            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, TEXT_LEN_FOR_LARGEST_FLOAT);
            strcpy(datacell->str_data, (char *)sqlite3_column_text(stmt, col_idx));
            datacell->str_size = strlen(datacell->str_data);


            break;
        case SQLITE_BLOB:
            datacell->type = DYTYPE_BLOB;

            datacell->str_size = 4;
            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->str_size + 1);
            strcpy(datacell->str_data, "BLOB");

            datacell->raw_size = sqlite3_column_bytes(stmt, col_idx);
            datacell->raw_data = (void *)dymem_allocate(mem->dymem_bin_data, datacell->raw_size);
            memcpy(datacell->raw_data, sqlite3_column_blob(stmt, col_idx), datacell->raw_size);
            break;

        case SQLITE_TEXT:
            datacell->type = DYTYPE_TEXT;

            datacell->str_size = sqlite3_column_bytes(stmt, col_idx);
            datacell->raw_size = datacell->str_size + 1;
            datacell->raw_data = NULL;

            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->raw_size);
            strcpy(datacell->str_data, sqlite3_column_text(stmt, col_idx));
            break;

        default: // Unknown sqlite type
            datacell->type = DYTYPE_NULL;

            datacell->raw_size = 0;
            datacell->raw_data = NULL;

            datacell->str_size = 7;
            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->str_size + 1);
            strcpy(datacell->str_data, "UNKNOWN");
    }

    return datacell;
}

void *init_data_column(Data_Column *datacol)
{
    datacol->cell_count = 0;
}

void populate_data_table_from_sqlite(Data_Table *table, sqlite3 *db, sqlite3_stmt *stmt)
{

    int status = 0;
    Data_Cursor cursor = { table, 0, 0 };
    Data_Column *column = NULL;
    Data_Cell *cell = NULL;

    // Populate column names
    loop (idx, table->col_count) {
        Data_Column *column = data_table_allocate_column(table, strlen(sqlite3_column_name(stmt, idx)));
        init_data_column(column);
        strcpy(column->name, sqlite3_column_name(stmt, idx));
    }

    // Populate data
    for (cursor.col_idx = 0; status = sqlite3_step(stmt); ++cursor.col_idx) {
        if (SQLITE_ROW == status) {

            for (cursor.row_idx = 0;
            cursor.row_idx < table->col_count;
            ++cursor.row_idx) {
                Data_Column *column = &table->columns[cursor.row_idx];
                cell = data_column_allocate_cell(column);
                init_data_cell_from_sqlite_row(table->data_mem, cell, stmt, cursor.row_idx);
            }
        } else {
            switch (status) {
            case SQLITE_DONE: printw("Success: Done.\n"); break;
            case SQLITE_MISUSE:
                printw("Error (SQLITE_MISUSE) stepping through statement: %s\n", sqlite3_errmsg(db));
                break;
            default:
                printw("Error (%d) stepping through statement: %s\n", status, sqlite3_errmsg(db));
            }
            break;
        }
    }
}

int prepare_query_for_user_tables(sqlite3 *db, sqlite3_stmt **stmt, char *sql)
{
    int err = 0;
    err = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
    if (err) {
        printw("Error preparing statement: %s\n", sqlite3_errmsg(db));
    }
    return err;
}
