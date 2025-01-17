/* Dynamic Memory
 * ==============
 *
 * Memory is dynamic, both in the sense that it can grow,
 * and it can assign different parts of its memory to different
 * types.
 *
 * Once initialised, Dymem can only ever grow or be freed entirely.
 * It cannot be shrunk or defragmented.
 *
 * Memory is initialised once, with a specified page size.
 * If more memory is required, another page size will be
 * allocated.
 *
 * Memory must be allocated through the dymem_allocate fn.
 * It can be allocated to any data type and must be cast
 * on assignment.
 */

Dymem *dymem_init(size_t page_size)
{
    Dymem *mem = (Dymem *)malloc(sizeof(Dymem));
    mem->page_size = page_size;
    mem->size = page_size;
    mem->used = 0;
    mem->data = (void *)malloc(mem->size);
    mem->cursor = (void *)mem->data;

    return mem;
}

// FIXME: Cursor will point to invalid address on realloc.
// Pages need to be implemented.
void *dymem_allocate(struct dymem *mem, size_t len)
{
    void *cursor = mem->cursor;

    if (mem->used + len > mem->size) {
        if (len > mem->page_size) {
            mem->page_size = len;
        }
        mem->size += mem->page_size;
        mem->data = (void *)realloc(mem->data, mem->size);
    }
    mem->used += len;
    mem->cursor += len;

    return cursor;
}
