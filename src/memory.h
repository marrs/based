typedef struct memory_page Memory_Page;

struct memory_page {
    size_t size;
    size_t used;
    char *cursor;
    char *data;
    Memory_Page *next;
    Memory_Page *prev;
};

typedef struct dymem {
    size_t init_page_size;
    int page_count;
    Memory_Page *first_page;
    Memory_Page *page_cursor;
} Dymem;

typedef struct vector {
    size_t page_size;
    size_t el_size;
    int len;
    Memory_Page *first_page;
    Memory_Page *page_cursor;
} Vector;

typedef struct vector_iter {
    size_t el_size;
    size_t page_size;
    size_t page_offset;
    int step;
    int len;
    Memory_Page *page;
    char *cursor;
} Vector_Iter;

#define vec_loop(vi, type, el) for ((el) = (type *)(vi->page->data); NULL != (el); (el) = (type *)vec_next((vi)))
