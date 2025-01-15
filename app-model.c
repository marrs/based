enum app_view_id {
    APP_VIEW_USER_TABLES = 0,
};

enum app_event {
    EVENT_INIT_VIEW,
    EVENT_CURSOR_UP,
    EVENT_CURSOR_DOWN,
};

typedef struct cursor {
    int row;
} Cursor;

typedef struct app_model {
    enum app_view_id current_view;
    struct {
        Cursor cursor;
        Data_Table *table;
    } user_tables;
} App_Model;

App_Model global_app_state;
