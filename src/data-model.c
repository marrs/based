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

int dytype_from_sqlite_str(const char *sqlite_type)
{
    if (NULL == sqlite_type) return DYTYPE_UNKNOWN;
    if (0 == strcmp("NULL", sqlite_type)) return DYTYPE_NULL;
    if (0 == strcmp("INTEGER", sqlite_type)) return DYTYPE_INT;
    if (0 == strcmp("FLOAT", sqlite_type)) return DYTYPE_FLOAT;
    if (0 == strcmp("BLOB", sqlite_type)) return DYTYPE_BLOB;
    if (0 == strcmp("TEXT", sqlite_type)) return DYTYPE_TEXT;
    return DYTYPE_UNKNOWN;
}

int dytype_from_sqlite(int sqlite_type)
{
    switch (sqlite_type) {
        case SQLITE_NULL: return DYTYPE_NULL;
        case SQLITE_INTEGER: return DYTYPE_INT;
        case SQLITE_FLOAT: return DYTYPE_FLOAT;
        case SQLITE_BLOB: return DYTYPE_BLOB;
        case SQLITE_TEXT: return DYTYPE_TEXT;
        default: return DYTYPE_UNKNOWN;
    }
}

char *sqlite3_type_as_str(int type)
{
    switch (type) {
        case SQLITE_NULL: return "NULL";
        case SQLITE_INTEGER: return "INT";
        case SQLITE_FLOAT: return "FLOAT";
        case SQLITE_BLOB: return "BLOB";
        case SQLITE_TEXT: return "TEXT";
        default: return "UNKNOWN";
    }
}

void init_table_pool(Table_Pool *pool)
{
    pool->table_count = 0;
    pool->table_vec = new_vector(sizeof(Data_Table), 3);
}

Data_Record *new_data_record(int field_count)
{
    Data_Record *record = (Data_Record *)malloc(sizeof(Data_Record));
    record->field_count = field_count;
    record->field_vec = new_vector(sizeof(Data_Field), field_count);
    record->dymem_data = dymem_init(KB(128));

    return record;
}

void delete_data_record(Data_Record *record)
{
    delete_vector(record->field_vec);
    dymem_free(record->dymem_data);
    free(record);
}

Data_Table *new_data_table_from_table_pool(Table_Pool *pool, const char *name, int col_count)
{
    Data_Table *table = (Data_Table *)vec_push_empty(pool->table_vec);
    ++pool->table_count;

    table->col_count = col_count;
    table->row_count = 0;
    table->name = name;
    table->data_mem = (Data_Memory *)malloc(sizeof(Data_Memory));
    table->data_mem->dymem_bin_data = dymem_init(MB(2));
    table->data_mem->dymem_str_data = dymem_init(MB(2));
    table->data_mem->dymem_meta_data = dymem_init(KB(1));
    table->column_vec = new_vector(sizeof(Data_Column), 10);
    return table;
}

Data_Column *new_column_from_data_table(Data_Table *table, const int type, const char *name, size_t name_len)
{
    Data_Column *column = (Data_Column *)vec_push_empty(table->column_vec);
    column->name = dymem_allocate(
            table->data_mem->dymem_meta_data,
            name_len + 1);
    strcpy(column->name, name);
    column->type = type;

    column->is_not_null = 0;
    column->is_pk = 0;
    column->is_read_only = 0;
    column->fk_table = NULL;
    column->fk_column = NULL;

    column->cell_vec = new_vector(sizeof(Data_Cell), 200);
    column->cell_count = 0;
    return column;
}

Data_Column *column_by_name_from_data_table(Data_Table *table, const char *name)
{
    Vector_Iter *iter = new_vector_iter(table->column_vec);
    Data_Column *col = NULL;

    vec_loop (iter, Data_Column, col) {
        if (0 == strcmp(name, col->name)) {
            delete_vector_iter(iter);
            return col;
        }
    }

    delete_vector_iter(iter);
    return NULL;
}

Data_Cell *allocate_cell_from_data_column(Data_Column *column)
{
    Data_Cell *cell = (Data_Cell *)vec_push_empty(column->cell_vec);
    ++column->cell_count;

    return cell;
}

// TODO: Implement way to release memory.
Data_Cell *new_cell_from_data_table_using_sqlite_row(
        Data_Table *table,
        Data_Column *column,
        sqlite3_stmt *stmt,
        int col_idx)
{
    Data_Cell *datacell = allocate_cell_from_data_column(column);
    Data_Memory *mem = table->data_mem;
    datacell->type = dytype_from_sqlite(sqlite3_column_type(stmt, col_idx));

    switch (datacell->type) {
        case DYTYPE_NULL:
            datacell->str_size = 4;
            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->str_size + 1);
            strcpy(datacell->str_data, "NULL");

            datacell->raw_size = 0;
            datacell->raw_data = NULL;
            break;

        case DYTYPE_INT:
            datacell->raw_size = sizeof(int);
            datacell->raw_data = (int *)dymem_allocate(mem->dymem_bin_data, datacell->raw_size);
            *(int *)datacell->raw_data = sqlite3_column_int(stmt, col_idx);

            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, TEXT_LEN_FOR_LARGEST_INT);
            strcpy(datacell->str_data, sqlite3_column_text(stmt, col_idx));
            datacell->str_size = strlen(datacell->str_data);
            break;

        case DYTYPE_FLOAT:
            datacell->raw_size = sizeof(double);
            datacell->raw_data = (double *)dymem_allocate(mem->dymem_bin_data, datacell->raw_size);
            *(double *)datacell->raw_data = sqlite3_column_double(stmt, col_idx);

            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, TEXT_LEN_FOR_LARGEST_FLOAT);
            strcpy(datacell->str_data, (char *)sqlite3_column_text(stmt, col_idx));
            datacell->str_size = strlen(datacell->str_data);


            break;
        case DYTYPE_BLOB:
            datacell->str_size = 4;
            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->str_size + 1);
            strcpy(datacell->str_data, "BLOB");

            datacell->raw_size = sqlite3_column_bytes(stmt, col_idx);
            datacell->raw_data = (void *)dymem_allocate(mem->dymem_bin_data, datacell->raw_size);
            memcpy(datacell->raw_data, sqlite3_column_blob(stmt, col_idx), datacell->raw_size);
            break;

        case DYTYPE_TEXT:
            datacell->str_size = sqlite3_column_bytes(stmt, col_idx);
            datacell->raw_size = datacell->str_size + 1;
            datacell->raw_data = NULL;

            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->raw_size);
            strcpy(datacell->str_data, sqlite3_column_text(stmt, col_idx));
            break;

        default: // Unknown sqlite type
            datacell->type = DYTYPE_UNKNOWN;

            datacell->raw_size = 0;
            datacell->raw_data = NULL;

            datacell->str_size = 7;
            datacell->str_data = (char *)dymem_allocate(mem->dymem_str_data, datacell->str_size + 1);
            strcpy(datacell->str_data, "UNKNOWN");
    }

    return datacell;
}

void new_columns_for_data_table_using_sqlite(Data_Table *table, sqlite3 *db, sqlite3_stmt *stmt)
{
    // Create columns and populate their names.
    loop (idx, table->col_count) {
        new_column_from_data_table(
            table,
            // XXX
            // Sqlite does not provide a decltype for pragma tables (apparently).
            // This isn't expected to be a problem, but it does mean that the
            // column->type will be inaccurate for some views.
            dytype_from_sqlite_str(sqlite3_column_decltype(stmt, idx)),
            sqlite3_column_name(stmt, idx),
            strlen(sqlite3_column_name(stmt, idx))
        );
    }
}

void handle_sqlite_step_status(sqlite3 *db, int status)
{
    // TODO: Roll these messages into the event loop so we can see them in the status bar.
    switch (status) {
    case SQLITE_DONE: printw("Success: Done.\n"); break;
    case SQLITE_MISUSE:
        printw("Error (SQLITE_MISUSE) stepping through statement: %s\n", sqlite3_errmsg(db));
        break;
    default:
        printw("Error (%d) stepping through statement: %s\n", status, sqlite3_errmsg(db));
    }
}

void populate_data_table_using_sqlite(Data_Table *table, sqlite3 *db, sqlite3_stmt *stmt)
{
    int status = 0;
    Data_Column *column = NULL;
    Data_Cell *cell = NULL;

    // Populate data.
    for (int col_idx = 0; status = sqlite3_step(stmt); ++col_idx) {
        if (SQLITE_ROW == status) {
            loop (col_idx, table->col_count) {
                Data_Column *column = vec_seek(table->column_vec, col_idx);
                cell = new_cell_from_data_table_using_sqlite_row(
                    table,
                    column,
                    stmt,
                    col_idx
                );
            }
            ++table->row_count;
        } else { handle_sqlite_step_status(db, status); break; }
    }
}

int prepare_query_using_sqlite(sqlite3 *db, sqlite3_stmt **stmt, char *sql)
{
    int err = 0;
    err = sqlite3_prepare_v2(db, sql, -1, stmt, NULL);
    if (err) {
        // TODO: Roll this message into the event loop so we can see it in the status bar.
        printw("Error preparing statement: %s\n", sqlite3_errmsg(db));
    }
    return err;
}

int new_table_with_query_using_sqlite(Data_Table **target_table, char *table_name, char *sql)
{
    sqlite3 *db = global_app_state.db;
    sqlite3_stmt *tbl_stmt;

    int err = prepare_query_using_sqlite(db, &tbl_stmt, sql);
    if (err) return err;

    int col_count = sqlite3_column_count(tbl_stmt);
    Data_Table *table = new_data_table_from_table_pool(global_table_pool, table_name, col_count);

    // Load col data
    new_columns_for_data_table_using_sqlite(table, db, tbl_stmt);

    populate_data_table_using_sqlite(table, db, tbl_stmt);
    *target_table = table;

    sqlite3_finalize(tbl_stmt);

    return 0;
}

int new_table_with_data_using_sqlite(Data_Table **target_table, const char *table_name)
{
    char sql[255];
    sqlite3 *db = global_app_state.db;
    sqlite3_stmt *data_stmt = NULL;
    sqlite3_stmt *meta_stmt = NULL;
    sqlite3_stmt *rel_stmt = NULL;
    int col_count = 0;
    int err = 0;
    int status = 0;

    // Query data.
    sprintf(sql, "select * from %s;", table_name);
    err = prepare_query_using_sqlite(db, &data_stmt, sql);
    if (err) { return err; }

    sprintf(sql, "select `from`, `to`, `table` from pragma_foreign_key_list('%s');", table_name);
    err = prepare_query_using_sqlite(db, &rel_stmt, sql);
    if (err) { return err; }

    sprintf(sql, "select `name`, `notnull`, `pk` from pragma_table_info('%s')", table_name);
    err = prepare_query_using_sqlite(db, &meta_stmt, sql);
    if (err) { return err; }

    // Init table
    col_count = sqlite3_column_count(data_stmt);
    Data_Table *table = new_data_table_from_table_pool(global_table_pool, table_name, col_count);
    new_columns_for_data_table_using_sqlite(table, db, data_stmt);

    // Populate meta data
    Data_Column *column = NULL;

    for (int row_idx = 0; status = sqlite3_step(rel_stmt); ++row_idx) {
        if (SQLITE_ROW == status) {

            // For `from` column
            column = column_by_name_from_data_table(
                table,
                sqlite3_column_text(rel_stmt, 0)
            );
            assert(NULL != column);

            // For `table` column
            err = new_table_with_data_using_sqlite(
                &column->fk_table,
                sqlite3_column_text(rel_stmt, 2)
            );
            if (err) { return err; }

            // For `to` column
            column->fk_column = column_by_name_from_data_table(
                column->fk_table,
                sqlite3_column_text(rel_stmt, 1)
            );
            assert(NULL != column);

        } else { handle_sqlite_step_status(db, status); break; }
    }

    for (int row_idx = 0; status = sqlite3_step(meta_stmt); ++row_idx) {
        if (SQLITE_ROW == status) {

            // For `name` column
            column = column_by_name_from_data_table(
                table,
                sqlite3_column_text(meta_stmt, 0)
            );
            assert(NULL != column);

            // For `notnull` column
            column->is_not_null = sqlite3_column_int(meta_stmt, 1);

            // For `pk` column
            column->is_pk = sqlite3_column_int(meta_stmt, 2);

            if (column->is_pk
            &&  column->is_not_null
            &&  DYTYPE_INT == column->type) {
                column->is_read_only = 1;
            }


        } else { handle_sqlite_step_status(db, status); break; }
    }

    populate_data_table_using_sqlite(table, db, data_stmt);
    *target_table = table;

    // Cleanup
    sqlite3_finalize(data_stmt);
    sqlite3_finalize(meta_stmt);
    sqlite3_finalize(rel_stmt);

    return 0;
}
