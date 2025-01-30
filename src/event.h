enum event_id {
    APP_EVENT_LOAD_USER_TABLES,
    APP_EVENT_LOAD_TABLE,
    APP_EVENT_VIEW_TABLE,
    UI_EVENT_CURSOR_UP,
    UI_EVENT_CURSOR_DOWN,
    UI_EVENT_CURSOR_RIGHT,
};

typedef struct Event {
    enum event_id id;
    enum dytype data_type;
    union {
        int data_as_int;
        float data_as_float;
        char *data_as_text;
        void *data_as_null;
    };
} Event;
