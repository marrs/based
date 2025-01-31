enum event_id {
    UI_EVENT_KEY_PRESS,
    UI_EVENT_CURSOR_UP,
    UI_EVENT_CURSOR_DOWN,
    UI_EVENT_CURSOR_RIGHT,

    APP_EVENT_LOAD_USER_TABLES,
    APP_EVENT_LOAD_TABLE,
    APP_EVENT_VIEW_TABLE,
    APP_EVENT_REFRESH_VIEW,
    APP_EVENT_CREATE_RECORD,
    APP_EVENT_EDIT_FILE,
};

typedef struct Event {
    enum event_id id;
    enum dytype data_type;
    union {
        int data_as_int;
        float data_as_float;
        char data_as_char;
        char *data_as_text;
        void *data_as_null;
    };
} Event;

#define plain_event(evt) (Event){ evt, DYTYPE_NULL, .data_as_null = NULL }
