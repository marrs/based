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

Dymem *dymem_init(size_t page_size)
{
    Dymem *mem = (Dymem *)malloc(sizeof(Dymem));

    mem->init_page_size = page_size;
    mem->page_count = 0;
    mem->first_page = NULL;
    mem->page_cursor = NULL;

    return mem;
}

void dymem_free(Dymem *mem)
{
    free_mempages(mem->first_page);
    free(mem);
}

