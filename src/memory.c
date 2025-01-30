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

#include "dymem.c"
#include "vector.c"
