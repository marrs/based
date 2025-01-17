typedef struct dymem {
    int page_size;
    int size;
    int used;
    void *cursor;
    void *data;
} Dymem;
