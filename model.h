typedef struct data_cell {
    enum dytype type;
    size_t raw_size;
    size_t str_size;
    char *str_data;
    void *raw_data;
} Data_Cell;

typedef struct data_column {
    int cell_count;  // Number of cells in column (exc. name).
    char *name;
    Dymem *dymem_cells;
    Data_Cell *cells;
} Data_Column;

typedef struct data_memory {
    Dymem *dymem_bin_data;
    Dymem *dymem_str_data;
    Dymem *dymem_meta_data;
} Data_Memory;

typedef struct data_table {
    int col_count;
    char *name;
    Dymem *dymem_columns;
    Data_Column *columns;
    Data_Memory *data_mem;
} Data_Table;

typedef struct app_memory_pool {
    int table_count;
    Data_Table *tables;
    Dymem *dymem_tables;
} App_Memory_Pool;

typedef struct data_cursor {
    Data_Table *data_table;
    int row_idx;
    int col_idx;
} Data_Cursor;

enum app_view_id {
    APP_VIEW_USER_TABLES = 0,
    APP_VIEW_SELECTED_TABLE,
    APP_VIEW_SELECTED_RECORD,
};

typedef struct view_cursor {
    int row;
    int col;
} View_Cursor;

typedef struct table_view {
    View_Cursor cursor;
    Data_Table *table;
} Table_View;

typedef struct app_model {
    sqlite3 *db;
    enum app_view_id current_view;
    Table_View user_tables;
    Table_View selected_table;
    /*
    struct {
        Cursor cursor;
        //Data_Record *record;
    } selected_record;
    */
} App_Model;

App_Model global_app_state;
App_Memory_Pool *global_mempool;
