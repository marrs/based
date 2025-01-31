{
    describe("vec_seek") {
        it("returns the element for given index, across page boundaries") {
            Vector *vec = new_vector(sizeof(int), 3);
            int *result = NULL;
            loop (idx, 10) {
                vec_push(vec, &idx);
                result = (int *)vec_seek(vec, idx);
                expect_int_eq(*result, idx);
            }
            delete_vector(vec);
        } tested;
    } tested;

    describe("vec_next") {
        Vector *vec = new_vector(sizeof(int), 3);

        loop(idx, 10) {
            vec_push(vec, &idx);
        }
        Vector_Iter *veci = new_vector_iter(vec);

        it("iterates the elements of a vector until it reaches the end") {
            int *result;
            loop_from (idx, 1, 10) {
                result = (int *)vec_next(veci);
                expect_int_eq(*result, idx);
            }
        } tested;

        it("leaves the cursor in place if the vector is at the last element") {
            expect_int_eq(*veci->cursor, 9);
        } tested;

        it("returns NULL if the cursor is at the last element") {
            expect_ptr_eq(NULL, vec_next(veci));
        } tested;

        delete_vector(vec);
        delete_vector_iter(veci);
    } tested;
}
