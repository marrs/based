typedef struct data_cell {
    enum dytype type;
    size_t raw_size;
    size_t str_size;
    char *str_data;
    void *raw_data;
} Data_Cell;

typedef struct data_memory {
    Dymem *dymem_bin_data;
    Dymem *dymem_str_data;
    Dymem *dymem_meta_data;
} Data_Memory;

typedef struct data_table {
    int col_count;
    int row_count;
    const char *name;
    Vector *column_vec;
    Data_Memory *data_mem;
} Data_Table;

typedef struct data_column {
    int cell_count;  // Number of cells in column (exc. name).
    char *name;
    int type;
    int is_not_null;
    int is_pk;
    int is_read_only;
    Data_Table *fk_table;
    struct data_column *fk_column;
    Vector *cell_vec;
} Data_Column;

typedef struct data_field {
    char *name;
    char *value;
} Data_Field;

typedef struct data_record {
    int field_count;
    Vector *field_vec;
    Dymem *dymem_data;
} Data_Record;

typedef struct table_pool {
    int table_count;
    Vector *table_vec;
} Table_Pool;

typedef struct data_cursor {
    Data_Table *data_table;
    int row_idx;
    int col_idx;
} Data_Cursor;

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
    Data_Table *table;
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
