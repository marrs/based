#include <stdio.h>
#include <cyaml/cyaml.h>

typedef struct data_field {
	char *name;
	char *value;
} Data_Field;

typedef struct yaml_data_record {
    unsigned int fields_count;
    Data_Field *fields;
} Yaml_Data_Record;


static const cyaml_schema_field_t field_mapping_schema[] = {
	CYAML_FIELD_STRING_PTR("name", CYAML_FLAG_POINTER,
			Data_Field, name,
			0, CYAML_UNLIMITED),
	CYAML_FIELD_STRING_PTR("value", CYAML_FLAG_POINTER,
			Data_Field, value,
			0, CYAML_UNLIMITED),
	CYAML_FIELD_END
};

static const cyaml_schema_value_t field_schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_DEFAULT,
			Yaml_Data_Record, field_mapping_schema),
};

static const cyaml_schema_field_t top_mapping_schema[] = {
	CYAML_FIELD_SEQUENCE("record", CYAML_FLAG_POINTER,
			Yaml_Data_Record, fields, &field_schema,
			0, CYAML_UNLIMITED),
	CYAML_FIELD_END
};

/* CYAML value schema for the top level mapping. */
static const cyaml_schema_value_t top_schema = {
	CYAML_VALUE_MAPPING(CYAML_FLAG_POINTER,
			Yaml_Data_Record, top_mapping_schema),
};


static const cyaml_config_t config = {
	.log_fn = cyaml_log,            /* Use the default logging function. */
	.mem_fn = cyaml_mem,            /* Use the default memory allocator. */
	.log_level = CYAML_LOG_WARNING, /* Logging errors and warnings only. */
};

/* Main entry point from OS. */
int main()
{
	cyaml_err_t err;
    Yaml_Data_Record *record;

	/* Load input file. */
	err = cyaml_load_file("test/basic-record.yml", &config,
			&top_schema, (cyaml_data_t **)&record, NULL);
	if (CYAML_OK != err) {
		fprintf(stderr, "ERROR: %s\n", cyaml_strerror(err));
		return 1;
	}

	/* Use the data. */
	printf("Internal representation:\n  Record:\n");
    Data_Field *field;
	for (unsigned idx = 0; idx < record->fields_count; ++idx) {
        field = &record->fields[idx];
		printf("    - %s: %s\n", field->name, field->value);
	}

	/* Free the data */
	cyaml_free(&config, &top_schema, record, 0);

	return 0;
}
