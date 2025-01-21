enum dytype {
    DYTYPE_NULL = 0,
    DYTYPE_INT,
    DYTYPE_FLOAT,
    DYTYPE_TEXT,
    DYTYPE_BLOB,
};

#define KB(x) 1000 * (x)
#define MB(x) 1000 * KB(x)

#define TEXT_LEN_FOR_LARGEST_INT 20

// Unverified. See https://stackoverflow.com/a/1701085
#define TEXT_LEN_FOR_LARGEST_FLOAT 24

#define return_on_err(x) if ((x)) return;

#define loop(x, count) for (int (x) = 0; (x) < (count); (++x))
#define loop_from(x, offset, count) for (int (x) = (offset); (x) < (count); (++x))
