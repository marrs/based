/* Data Model
 * ==========
 *
 *  Objects associated with storing and managing the
 *  data fetched from various resources used by the app.
 *
 *  struct tabular_data
 *  -------------------
 *
 *  Supports the view for rendering tabular data.
 *
 *  Optimised for reading data by column, not row.
 *  This is preferred because table layout will be based on
 *  column width.
 */

struct dymem {
    size_t page_size;
    size_t size;
    size_t used;
    char *cursor;
    char *data;
};

struct dymem *init_dymem(size_t page_size)
{
    struct dymem *mem = (struct dymem *)malloc(sizeof(struct dymem));
    mem->page_size = page_size;
    mem->size = page_size;
    mem->used = 0;
    mem->data = (void *)malloc(mem->size);
    mem->cursor = mem->data;

    return mem;
}

struct col_data {
    int cell_count;  // Number of cells in column.
    struct dymem *cells;   // Each cell separated by null char.
    struct dymem *cell_index;
};

struct tabular_data {
    int col_count;
    struct col_data **col_data;
};

struct col_data *init_col_data()
{
    struct col_data *coldata = (struct col_data *)malloc(sizeof(struct col_data));
    coldata->cell_count = 200;
    coldata->cells = init_dymem(MB(2));
    coldata->cell_index = init_dymem(coldata->cell_count * sizeof(char *));

    return coldata;
}

struct tabular_data *init_tabular_data(int col_count)
{
    struct tabular_data *tabdata = (struct tabular_data *)malloc(sizeof(struct tabular_data));

    tabdata->col_count = col_count;

    tabdata->col_data = (struct col_data **)malloc(sizeof(struct col_data) * col_count);

    loop (idx, tabdata->col_count) {
        tabdata->col_data[idx] = init_col_data();
    }

    return tabdata;
}

