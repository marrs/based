{
    describe("new_record_from_cyaml_file") {
        it("maps cyaml record data to record fields") {
            printf(
                "\n%sInternal representation:\n%sRecord:\n",
                test_padding[test_padlen + 1],
                test_padding[test_padlen + 2]
            );

            Data_Record *rec = new_record_from_cyaml_file("test/expected/basic-record.cyml");
            Data_Field *field = NULL;

            Vector_Iter *iter = new_vector_iter(rec->field_vec);
            vec_loop (iter, Data_Field, field) {
                printf("%s- %s: %s\n", test_padding[test_padlen + 2], field->name, field->value);
            } delete_vector_iter(iter);

            delete_data_record(rec);
        } tested;
    } tested;
}
