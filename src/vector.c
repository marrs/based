// TODO: Allocate this from mem manager and init here.
Vector *new_vector(const size_t el_size, const int el_count)
{
    Vector *vec = (Vector *)malloc(sizeof(Vector));
    vec->el_size = el_size;
    vec->page_size = el_size * el_count;
    vec->page_el_count = el_count;
    vec->len = 0;
    vec->first_page = new_mem_page(vec->page_size);
    vec->page_cursor = vec->first_page;
    *vec->page_cursor->data = '\0';
    return vec;
}

void delete_vector(Vector *vec)
{
    free_mempages(vec->first_page);
    free(vec);
}

void reset_vector_iter(Vector_Iter *veci)
{
    veci->page_offset = 0;
    veci->step = 0;
    veci->page = veci->first_page;
    veci->cursor = veci->page->data;
}

Vector_Iter *new_vector_iter(Vector *vec)
{
    Vector_Iter *veci = (Vector_Iter *)malloc(sizeof(Vector_Iter));

    veci->page_size = vec->page_size;
    veci->el_size = vec->el_size;
    veci->len = vec->len;
    veci->first_page = vec->first_page;
    reset_vector_iter(veci);

    if (!vec->len) {
        veci->cursor = NULL;
    }

    return veci;
}

void delete_vector_iter(Vector_Iter *veci)
{
    if (NULL != veci) {
        free(veci);
    }
}

void *vec_push_empty(Vector *vec)
{
    void *cursor = vec->page_cursor->cursor;

    vec->page_cursor->used += vec->el_size;
    ++vec->len;
    if (vec->page_cursor->used >= vec->page_cursor->size) {
        // Used memory should never exceed page size.  If it has, a data
        // corruption has occured and we must abort.
        assert(vec->page_cursor->used == vec->page_cursor->size);

        if (NULL == vec->page_cursor->next) {
            vec->page_cursor->next = new_mem_page(vec->page_size);
        }
        vec->page_cursor->next->prev = vec->page_cursor;
        vec->page_cursor = vec->page_cursor->next;
        vec->page_cursor->cursor = vec->page_cursor->data; 
    } else {
        assert(vec->page_cursor->used <= vec->page_cursor->size - vec->el_size);
        vec->page_cursor->cursor += vec->el_size;
    }

    return cursor;
}

void vec_push(Vector *vec, void *el_src)
{
    char *cursor = vec_push_empty(vec);
    memcpy(cursor, (char *)el_src, vec->el_size);
}

void *vec_pop(Vector *vec)
{
    if (vec->len) {
        vec->page_cursor->cursor -= vec->el_size;
        if (vec->page_cursor->cursor < (char *)vec->page_cursor) {
            vec->page_cursor = vec->page_cursor->prev;
            vec->page_cursor->cursor = (char *)vec->page_cursor + vec->page_size - vec->el_size;
        }
        vec->page_cursor->used -= vec->el_size;
        --vec->len;
        return vec->page_cursor->cursor;
    }
    return NULL;
}

void *vec_seek(Vector *vec, size_t idx)
{
    const size_t page_size = vec->page_size;
    const size_t el_size = vec->el_size;

    Memory_Page *page = vec->first_page;

    int page_number = 1;
    while (page_number * page->size <= idx * el_size) {
        page = page->next;
        ++page_number;
    }
    char *el = page->data;
    size_t offset_to_page = (page_number - 1) * vec->page_el_count;
    size_t page_offset = idx - offset_to_page; 

    el += page_offset * el_size;
    return (void *)el;
}

void *vec_next(Vector_Iter *veci)
{
    if (NULL == veci) {
        return NULL;
    }
    if (veci->step >= veci->len - 1) {
        assert(veci->step == veci->len -1);
        return NULL;
    }

    veci->page_offset += veci->el_size;
    veci->cursor += veci->el_size;

    if (veci->page_offset >= veci->page_size) {
        assert(veci->page_offset == veci->page_size);

        // We have not reached the end of the vector, so there
        // should be another page if we are at the end of the
        // current one.
        assert(NULL != veci->page->next);

        veci->page = veci->page->next;
        veci->page_offset = 0;
        veci->cursor = veci->page->data;
    }

    ++veci->step;
    return veci->cursor;
}
