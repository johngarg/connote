#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "../src/utils.h"

void test_construct_filename() {
    char filename[MAX_NAME_LEN];
    char *keywords[2] = {"kw1", "kw2"};

    // Initialise filename to empty string
    filename[0] = '\0';
    construct_filename("20230903T123456", "12a=1", "test-title", keywords, 2, 1, filename);
    assert(strcmp(filename, "20230903T123456==12a=1--test-title__kw1_kw2.md") == 0);

    filename[0] = '\0';
    char *keywords2[] = {};
    construct_filename("20230903T123456", "", "test-title", keywords2, 0, 1, filename);
    assert(strcmp(filename, "20230903T123456--test-title.md") == 0);

    filename[0] = '\0';
    construct_filename("20230903T123456", "", "", keywords2, 0, 1, filename);
    assert(strcmp(filename, "20230903T123456.md") == 0);

    printf("All tests passed for construct_filename.\n");
}

int main() {
    test_construct_filename();

    return 0;
}
