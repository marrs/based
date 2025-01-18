#include <stdio.h>

int is_ws_char(char x)
{
    return x == '\t' || x == '\n' || x == ' ';
}
int main()
{
    const char *yml_filename = "test/basic-record.yml";
    FILE *ymlf = fopen(yml_filename, "r");
    if (!ymlf) {
        fprintf(stderr, "Failed to open file: %s\n", yml_filename);
        return 1;
    }

    char cursor;

    enum cyaml_write_mode {
        CYAML_IDLE,
        CYAML_NEW_LINE,
        CYAML_FIELD_NAME,
        CYAML_FIELD_VALUE,
    };

    const char *indent_str = "    ";
    int write_mode = CYAML_IDLE,
        is_new_line = 1,
        is_field_name = 0;

    fprintf(stdout, "record:\n");

    while ((cursor = fgetc(ymlf)) != EOF) {
        if (is_field_name) {
            is_field_name = 0;
        }
        if (is_new_line) {
            is_new_line = 0;

            if (is_ws_char(cursor)) {
                if (CYAML_FIELD_VALUE == write_mode) {
                    fprintf(stdout, "%s", indent_str);
                } else {
                    write_mode = CYAML_FIELD_VALUE;
                    fprintf(stdout, "\n    value:\n");
                    is_new_line = 1;
                }
            } else {
                write_mode = CYAML_FIELD_NAME;
                fprintf(stdout, "  -\n    name: ");
            }
        } else {
            switch (cursor) {
                case '\n':
                    is_new_line = 1;
                    break;
                case ':':
                    if (CYAML_FIELD_NAME == write_mode) {
                        write_mode = CYAML_IDLE;
                    }
                    break;
            }
        }
        switch (write_mode) {
            case CYAML_IDLE:
                break;
            default:
                putc(cursor, stdout);
        }
    }

    return 0;
}
