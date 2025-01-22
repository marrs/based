/* Data Model
 * ==========
 *
 *  Objects associated with storing and managing the data fetched from various
 *  resources used by the app.
 *
 *  Data_Table
 *  ----------
 *
 *  Supports the view for rendering tabular data.
 *
 *  Optimised for reading data by column, not row.  This is preferred because
 *  table layout will be based on column width.
 *
 *  The memory for the db data is managed by Table_Pool, which stores all
 *  the data associated with a single table/view in Data_Memory.  The data are
 *  accessed via the Data_Table struct members.  The memory pool can store
 *  multiple such collections.
 *
 *  Data queried from the SQL database is copied to Data_Memory.  Text data are
 *  copied straight to str_data.  Binary data are stored in raw_data, but a
 *  string representation is also kept in str_data.  Only the contents of
 *  str_data will be displayed to the user.
 *
 *  Refernces to these data are found in Data_Cell objects.  These are stored
 *  in an array representing their columns.
 *
 *  While the Data_Cell objects are stored contiguously by column, their data
 *  are stored by row.  This is because there is only one memory buffer and
 *  data fetched from the database is written row by row.  It may improve
 *  performance to group the data by column as well.  This would require a pool
 *  of buffers, one per column.
 *
 *  The cell value type will be stored in Data_Cell in case we wish to work
 *  with the original types.
 *
 *  From https://sqlite.org/c3ref/c_blob.html: Every value in SQLite has one of
 *  five fundamental datatypes:
 *
 *     64-bit signed integer 64-bit IEEE floating point number string BLOB NULL
 */

void init_table_pool(Table_Pool *pool)
{
    pool->table_count = 0;
    pool->table_vec = new_vector(sizeof(Data_Table), 3);
}

Data_Table *new_data_table_from_table_pool(Table_Pool *pool, char *name, int col_count)
{
    Data_Table *table = (Data_Table *)vec_push_empty(pool->table_vec);
    ++pool->table_count;

    table->col_count = col_count;
    table->row_count = 0;
    table->name = name;
    table->data_mem = (Data_Memory *)dymem_init(MB(2));
    table->data_mem->dymem_bin_data = dymem_init(MB(2));
    table->data_mem->dymem_str_data = dymem_init(MB(2));
    table->data_mem->dymem_meta_data = dymem_init(KB(1));
    table->column_vec = new_vector(sizeof(Data_Column), 10);
    return table;
}

Data_Column *new_column_from_data_table(Data_Table *table, const char *name, size_t name_len)
{
    Data_Column *column = (Data_Column *)vec_push_empty(table->column_vec);
    column->name = dymem_allocate(
            table->data_mem->dymem_meta_data,
            name_len + 1);
    column->cell_vec = new_vector(sizeof(Data_Cell), 200);
    column->cell_count = 0;
    strcpy(column->name, name);
    return column;
}

Data_Cell *allocate_cell_from_data_column(Data_Column *column)
{
    Data_Cell *cell = (Data_Cell *)vec_push_empty(column->cell_vec);
    ++column->cell_count;

    return cell;
}

// TODO: Implement way to release memory.
Data_Cell *init_data_cell_from_sqlite_row(
        Data_Table *table,
        Data_Cell *datacell,
        sqlite3_stmt *stmt,
        int col_idx)
{

    Data_Memory *mem = table->data_mem;
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

void populate_data_table_from_sqlite(Data_Table *table, sqlite3 *db, sqlite3_stmt *stmt)
{
    int status = 0;
    Data_Column *column = NULL;
    Data_Cell *cell = NULL;

    // Create columns and populate their names.
    loop (idx, table->col_count) {
        Data_Column *column = new_column_from_data_table(
            table,
            sqlite3_column_name(stmt, idx),
            strlen(sqlite3_column_name(stmt, idx))
        );
    }

    // Populate data.
    for (int col_idx = 0; status = sqlite3_step(stmt); ++col_idx) {
        if (SQLITE_ROW == status) {
            loop (row_idx, table->column_vec->len) {
                Data_Column *column = vec_seek(table->column_vec, row_idx);
                cell = allocate_cell_from_data_column(column);
                init_data_cell_from_sqlite_row(
                    table,
                    cell,
                    stmt,
                    row_idx
                );
            }
            ++table->row_count;
        } else {
            // TODO: Roll these messages into the event loop so we can see them in the status bar.
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
        // TODO: Roll this message into the event loop so we can see it in the status bar.
        printw("Error preparing statement: %s\n", sqlite3_errmsg(db));
    }
    return err;
}
