typedef struct table_cell {
    enum dytype type;
    size_t raw_size;
    size_t str_size;
    char *str_data;
    void *raw_data;
} Table_Cell;

typedef struct table_memory {
    Dymem *dymem_bin_data;
    Dymem *dymem_str_data;
    Dymem *dymem_meta_data;
} Table_Memory;

typedef struct table {
    int col_count;
    int row_count;
    const char *name;
    Vector *column_vec;
    Table_Memory *data_mem;
} Table;

typedef struct table_column {
    int cell_count;  // Number of cells in column (exc. name).
    char *name;
    int type;
    int is_not_null;
    int is_pk;
    int is_read_only;
    Table *fk_table;
    struct table_column *fk_column;
    Vector *cell_vec;
} Table_Column;

typedef struct record_field {
    char *name;
    char *value;
} Record_Field;

typedef struct record {
    int field_count;
    Vector *field_vec;
    Dymem *dymem_data;
} Record;

typedef struct table_pool {
    int table_count;
    Vector *table_vec;
} Table_Pool;

typedef struct table_cursor {
    Table *table;
    int row_idx;
    int col_idx;
} Table_Cursor;

enum app_view_id {
    APP_VIEW_TABLE = 0,
    APP_VIEW_SELECTED_RECORD,
};

typedef struct view_cursor {
    int row;
    int col;
} View_Cursor;

typedef struct table_view {
    View_Cursor cursor;
    Table *table;
} Table_View;

typedef struct app_model {
    sqlite3 *db;
    enum app_view_id current_view;
    Table_View user_tables;
    Vector *loaded_table_vec;
    Table_View *current_table_view;
    /*
    struct {
        Cursor cursor;
        //Data_Record *record;
    } selected_record;
    */
} App_Model;

App_Model global_app_state;
Table_Pool *global_table_pool;
