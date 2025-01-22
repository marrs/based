#include <assert.h>
/* Dynamic Memory
 * ==============
 *
 * Memory is dynamic, both in the sense that it can grow,
 * and it can assign different parts of its memory to different
 * types.
 *
 * Memory is stored in a linked list of pages.  Data are not allowed
 * to straddle page boundaries, as their references may not be
 * contiguous.
 *
 * Individual pages cannot be freed; instead, the entire page set
 * must be freed at once.
 *
 * Dymem
 * -----
 *
 * Once initialised, Dymem can only ever grow or be freed entirely.
 * It cannot be shrunk or defragmented.
 *
 * Dymem is initialised once, with no pages.
 *
 * The first page is allocated when memory is required.  The page
 * size will be set to the value of init_page_size unless the
 * amount of memory required exceeds this amount, in which case
 * the page size will be set to the required amount.
 *
 * Memory must be allocated through the dymem_allocate fn.
 * It can be allocated to any data type and must be cast
 * on assignment.
 *
 * dymem_allocate will attempt to fully fill a page before advancing
 * the page cursor to the next available page.
 *
 * Vector
 * ------
 *
 *  A vector is like dymem, except all its elements must be of the
 *  same size.
 *
 *  Unlike dymem, its first page is initialised when the vector is
 *  created, ready to receive data that are pushed to it.
 */

Dymem *dymem_init(size_t page_size)
{
    Dymem *mem = (Dymem *)malloc(sizeof(Dymem));

    mem->init_page_size = page_size;
    mem->page_count = 0;
    mem->first_page = NULL;
    mem->page_cursor = NULL;

    return mem;
}

Memory_Page *new_mem_page(size_t page_size)
{
    Memory_Page *page = (Memory_Page *)malloc(sizeof(Memory_Page));
    page->size = page_size;
    page->used = 0;
    page->cursor = page->data = (char *)malloc(page->size);
    page->next = NULL;
    page->prev = NULL;
    return page;
}

Memory_Page *dymem_new_page(Dymem *mem, size_t init_len)
{
    return new_mem_page(init_len > mem->init_page_size? init_len : mem->init_page_size);
}

void *dymem_allocate(struct dymem *mem, size_t len)
{
    Memory_Page *page = NULL;

    if (NULL == mem->page_cursor) {
        page = dymem_new_page(mem, len);

        mem->page_cursor = page;
        if (NULL == mem->first_page) {
            // We always assign the first page of memory on allocation, not
            // initialisation.  We do this because, in theory, it's possible
            // that the initial page size could always be too small for a
            // dataset using it (e.g. video assets); in which case, the first
            // page would never get used if it had been created on init.

            mem->first_page = page;
        }
    } else {
        page = mem->page_cursor;
    }

    void *mem_cursor = NULL;

    if (page->used + len > page->size) {
        // Create page for new data.
        Memory_Page *new_page = dymem_new_page(mem, len);
        new_page->used = len;
        new_page->cursor += len;

        // Append new_page to list.
        Memory_Page *last_page = page;
        while (NULL != last_page->next) {
            last_page = last_page->next;
        }
        last_page->next = new_page;
    } else {
        // Data can fit in existing page.
        mem_cursor = page->cursor;
        page->used += len;
        page->cursor += len;
    }

    while (mem->page_cursor->used >= mem->page_cursor->size) {
        // Used memory should never exceed page size.  If it has, a data
        // corruption has occured and we must abort.
        assert(mem->page_cursor->used == mem->page_cursor->size);

        mem->page_cursor = mem->page_cursor->next;
        if (NULL == mem->page_cursor) { break; }
    }

    return mem_cursor;
}

void free_mempages(Memory_Page *first_page)
{
    Memory_Page *page_cursor;
    Memory_Page *next_page;
    for (page_cursor = first_page, next_page = page_cursor->next;

         NULL != next_page;

         page_cursor = next_page,
         next_page = next_page->next)
    {
        free(page_cursor->data);
        free(page_cursor);
    }
    free(page_cursor->data);
    free(page_cursor);
}

void dymem_free(Dymem *mem)
{
    free_mempages(mem->first_page);
    free(mem);
}

// TODO: Allocate this from mem manager and init here.
Vector *new_vector(const size_t el_size, const int el_count)
{
    Vector *vec = (Vector *)malloc(sizeof(Vector));
    vec->el_size = el_size;
    vec->page_size = el_size * el_count;
    vec->len = 0;
    vec->first_page = new_mem_page(vec->page_size);
    vec->page_cursor= vec->first_page;
    return vec;
}

void reset_vector_iter(Vector_Iter *veci)
{
    veci->page_offset = 0;
    veci->step = 0;
    veci->page = veci->first_page;
    veci->cursor = veci->first_page->data;
}

Vector_Iter *new_vector_iter(Vector *vec)
{
    Vector_Iter *veci = (Vector_Iter *)malloc(sizeof(Vector_Iter));

    veci->page_size = vec->page_size;
    veci->el_size = vec->el_size;
    veci->len = vec->len;
    veci->first_page = vec->first_page;
    reset_vector_iter(veci);

    return veci;
}

void delete_vector_iter(Vector_Iter *veci)
{
    free(veci);
}

void delete_vector(Vector *vec)
{
    free_mempages(vec->first_page);
    free(vec);
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
    void *cursor = vec_push_empty(vec);
    memcpy(cursor, (char *)el_src, vec->el_size);
}

void *vec_seek(Vector *vec, size_t idx)
{
    const size_t page_size = vec->page_size;
    const size_t el_size = vec->el_size;
    const size_t page_el_count = page_size / el_size;

    Memory_Page *page = vec->first_page;

    int page_number = 1;
    while (page_number * page->size <= idx * el_size) {
        page = page->next;
        ++page_number;
    }
    char *el = page->data;
    size_t offset_to_page = (page_number - 1) * page_el_count;
    size_t page_offset = idx - offset_to_page; 

    el += page_offset * el_size;
    return (void *)el;
}

void *vec_next(Vector_Iter *veci)
{
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
    } else {
    }

    ++veci->step;
    return veci->cursor;
}
