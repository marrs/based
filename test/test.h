#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static char test_pad0[] = "";
static char test_pad1[] = "  ";
static char test_pad2[] = "    ";
static char test_pad3[] = "      ";
static char test_pad4[] = "        ";
static char test_pad5[] = "          ";
char *test_padding[] = {
    test_pad0,
    test_pad1,
    test_pad2,
    test_pad3,
    test_pad4,
    test_pad5,
};
int test_padlen = 0;
int passing_tests = 0;
int failing_tests = 0;

#define describe(desc) \
    ++test_padlen; \
    printf("%s%s:\n", test_padding[test_padlen], desc);

#define it(desc) \
    ++test_padlen; \
    printf("%s- %s ", test_padding[test_padlen], desc);

typedef struct test_expectation {
    union {
        char *sv;
        char cv;
        int iv;
        void *vp;
    };
} Test_Expectation;

void expect_str_eq(const char *v1, const char *v2)
{
    if (strcmp(v1, v2) == 0) {
        ++passing_tests;
        printf("✓");
    } else {
        ++failing_tests;
        printf("\n%s  ❌ Expected \"%s\" to equal \"%s\"\n", test_padding[test_padlen], v1, v2);
    }
}

void expect_char_eq(const char v1, const char v2)
{
    if (v1 == v2) {
        ++passing_tests;
        printf("✓");
    } else {
        ++failing_tests;
        printf("\n%s  ❌ Expected \"%c\" to equal \"%c\"\n", test_padding[test_padlen], v1, v2);
    }
}

void expect_int_eq(const int v1, const int v2)
{
    if (v1 == v2) {
        ++passing_tests;
        printf("✓");
    } else {
        ++failing_tests;
        printf("\n%s  ❌ Expected \"%d\" to equal \"%d\"\n", test_padding[test_padlen], v1, v2);
    }
}

void expect_ptr_eq(const void *v1, const void *v2)
{
    if (v1 == v2) {
        ++passing_tests;
        printf("✓");
    } else {
        ++failing_tests;
        printf("\n%s  ❌ Expected \"%p\" to equal \"%p\"\n", test_padding[test_padlen], v1, v2);
    }
}

#define tested printf("\n"); --test_padlen;

int exit_testing()
{
    if (passing_tests > 0) {
        printf("%d passing tests.\n", passing_tests);
    }
    if (failing_tests > 0) {
        printf("%d failing tests.\n", failing_tests);
        return 1;
    }
    return 0;
}
