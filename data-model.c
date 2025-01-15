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

enum dytype {
    DYTYPE_NULL = 0,
    DYTYPE_INT,
    DYTYPE_FLOAT,
    DYTYPE_TEXT,
    DYTYPE_BLOB,
};

typedef struct data_cell {
    enum dytype type;
    size_t raw_size;
    size_t str_size;
    char *str_data;
    void *raw_data;
} Data_Cell;

typedef struct data_column {
    int cell_count;  // Number of cells in column.
    Dymem *dymem_cells;
} Data_Column;

typedef struct data_table {
    int col_count;
    Data_Column *column_data;
} Data_Table;

typedef struct data_cursor {
    Data_Table *data_table;
    int row_idx;
    int col_idx;
} Data_Cursor;


Data_Cell *init_data_cell_from_sqlite_row(
        Data_Cell *datacell,
        sqlite3_stmt *stmt,
        int col_idx)
{

    datacell->type = sqlite3_column_type(stmt, col_idx);

    switch (datacell->type) {
        case SQLITE_NULL:
            datacell->type = DYTYPE_NULL;

            datacell->str_size = 4;
            datacell->str_data = dymem_allocate(global_db_str_mem, datacell->str_size + 1);
            strcpy(datacell->str_data, "NULL");

            datacell->raw_size = 0;
            datacell->raw_data = NULL;
            break;

        case SQLITE_INTEGER:
            datacell->type = DYTYPE_INT;

            datacell->raw_size = sizeof(int);
            datacell->raw_data = (int *)dymem_allocate(global_db_bin_mem, datacell->raw_size);
            *(int *)datacell->raw_data = sqlite3_column_int(stmt, col_idx);

            datacell->str_data = dymem_allocate(global_db_str_mem, TEXT_LEN_FOR_LARGEST_INT);
            sprintf(datacell->str_data, "%d", *(int *)datacell->raw_data);
            datacell->str_size = strlen(datacell->str_data);
            break;

        case SQLITE_FLOAT:
            datacell->type = DYTYPE_FLOAT;

            datacell->raw_size = sizeof(double);
            datacell->raw_data = (double *)dymem_allocate(global_db_bin_mem, datacell->raw_size);
            *(double *)datacell->raw_data = sqlite3_column_double(stmt, col_idx);

            datacell->str_data = dymem_allocate(global_db_str_mem, TEXT_LEN_FOR_LARGEST_FLOAT);
            sprintf(datacell->str_data, "%d", *(double *)datacell->raw_data);
            datacell->str_size = strlen(datacell->str_data);


            break;
        case SQLITE_BLOB:
            datacell->type = DYTYPE_BLOB;

            datacell->str_size = 4;
            datacell->str_data = dymem_allocate(global_db_str_mem, datacell->str_size + 1);
            strcpy(datacell->str_data, "BLOB");

            datacell->raw_size = sqlite3_column_bytes(stmt, col_idx);
            datacell->raw_data = (void *)dymem_allocate(global_db_bin_mem, datacell->raw_size);
            memcpy(datacell->raw_data, sqlite3_column_blob(stmt, col_idx), datacell->raw_size);
            break;

        case SQLITE_TEXT:
            datacell->type = DYTYPE_TEXT;

            datacell->raw_size = sqlite3_column_bytes(stmt, col_idx);
            datacell->raw_data = NULL;

            datacell->str_size = datacell->raw_size - 1;
            datacell->str_data = dymem_allocate(global_db_str_mem, datacell->raw_size);
            strcpy(datacell->str_data, sqlite3_column_text(stmt, col_idx));
            break;

        default: // Unknown sqlite type
            datacell->type = DYTYPE_NULL;

            datacell->raw_size = 0;
            datacell->raw_data = NULL;

            datacell->str_size = 7;
            datacell->str_data = dymem_allocate(global_db_str_mem, datacell->str_size + 1);
            strcpy(datacell->str_data, "UNKNOWN");
    }

    return datacell;
}

void *init_data_column(Data_Column *datacol)
{
    datacol->cell_count = 0;
    datacol->dymem_cells = dymem_init(200 * sizeof(Data_Cell));
}

Data_Table *new_data_table(int col_count)
{
    Data_Table *datatable = (Data_Table *)malloc(sizeof(Data_Table));

    datatable->col_count = col_count;

    datatable->column_data = (Data_Column *)malloc(sizeof(Data_Column) * col_count);

    loop (idx, datatable->col_count) {
        init_data_column(&datatable->column_data[idx]);
    }

    datatable->column_data;

    return datatable;
}
