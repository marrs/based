typedef struct view_table_model {
    char *table_name;
    View_Cursor *cursor;
    Table *table;
    char *status_bar_text;
} View_Table_Model;
