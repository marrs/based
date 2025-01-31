enum cyaml_write_mode {
    CYAML_IDLE,
    CYAML_NEW_LINE,
    CYAML_FIELD_NAME,
    CYAML_FIELD_VALUE,
};

typedef struct cyaml_field {
	char *name;
	char *value;
} Cyaml_Field;

typedef struct cyaml_record {
    unsigned int fields_count;
    Cyaml_Field *fields;
} Cyaml_Record;

static const cyaml_schema_field_t field_mapping_schema[] = {
	CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER,
			Cyaml_Field, name,
			0, CYAML_UNLIMITED),
	CYAML_FIELD_STRING_PTR("value", CYAML_FLAG_POINTER,
			Cyaml_Field, value,
			0, CYAML_UNLIMITED),
	CYAML_FIELD_END
};

static const cyaml_schema_value_t field_schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT,
			Cyaml_Record, field_mapping_schema),
};

static const cyaml_schema_field_t top_mapping_schema[] = {
	CYAML_FIELD_SEQUENCE("record", CYAML_FLAG_POINTER,
			Cyaml_Record, fields, &field_schema,
			0, CYAML_UNLIMITED),
	CYAML_FIELD_END
};

/* CYAML value schema for the top level mapping. */
static const cyaml_schema_value_t top_schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
			Cyaml_Record, top_mapping_schema),
};

static const cyaml_config_t config = {
	.log_fn = cyaml_log,            /* Use the default logging function. */
	.mem_fn = cyaml_mem,            /* Use the default memory allocator. */
	.log_level = CYAML_LOG_WARNING, /* Logging errors and warnings only. */
};

int convert_file_yaml_to_cyaml(char *y_filename, char *cy_filename)
{
    FILE *y_file = fopen(y_filename, "r");
    if (!y_file) {
        // TODO: Better error handling
        fprintf(stderr, "Failed to open file for reading: %s\n", y_filename);
        return 0;
    }

    FILE *cy_file = fopen(cy_filename, "w");
    if (!y_file) {
        // TODO: Better error handling
        fprintf(stderr, "Failed to open file for writing: %s\n", y_filename);
        return 0;
    }

    char cursor;

    const char *indent_str = "    ";
    int write_mode = CYAML_IDLE,
        is_new_line = 1,
        is_field_name = 0;

    fprintf(cy_file, "record:\n");

    while ((cursor = fgetc(y_file)) != EOF) {
        if (is_field_name) {
            is_field_name = 0;
        }
        if (is_new_line) {
            is_new_line = 0;

            if (is_ws_char(cursor)) {
                if (CYAML_FIELD_VALUE == write_mode) {
                    fprintf(cy_file, "%s", indent_str);
                } else {
                    write_mode = CYAML_FIELD_VALUE;
                    fprintf(cy_file, "\n%svalue:\n", indent_str);
                    is_new_line = 1;
                }
            } else {
                write_mode = CYAML_FIELD_NAME;
                fprintf(cy_file, "  -\n%sname: ", indent_str);
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
                putc(cursor, cy_file);
        }
    }
    return 1;
}

Record *new_record_from_cyaml_file(char *filename)
{
	cyaml_err_t err;
    Cyaml_Record *crec;

	err = cyaml_load_file(filename, &config,
			&top_schema, (cyaml_data_t **)&crec, NULL);  // TODO: (**)&!
	if (CYAML_OK != err) {
		fprintf(stderr, "ERROR: %s\n", cyaml_strerror(err));
		return NULL;
	}

    Record *rec = new_record(crec->fields_count);
    Record_Field datafield = (Record_Field){ .name = NULL, .value = NULL };
    Cyaml_Field *cfield = NULL;
    loop (idx, crec->fields_count) {
        cfield = &crec->fields[idx];
        datafield.name = (char *)dymem_allocate(rec->dymem_data, strlen(cfield->name) + 1);
        datafield.value = (char *)dymem_allocate(rec->dymem_data, strlen(cfield->value) + 1);

        strcpy(datafield.name, cfield->name);
        strcpy(datafield.value, cfield->value);
        vec_push(rec->field_vec, &datafield);
    }

	cyaml_free(&config, &top_schema, crec, 0);
    return rec;
}
