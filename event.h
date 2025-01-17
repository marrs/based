enum event_id {
    EVENT_LOAD_USER_TABLES,
    EVENT_LOAD_SELECTED_TABLE,
    EVENT_CURSOR_UP,
    EVENT_CURSOR_DOWN,
    EVENT_CURSOR_RIGHT,
    EVENT_VIEW_TABLE,
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
