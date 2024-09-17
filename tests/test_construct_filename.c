#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "../src/utils.h"

void test_construct_filename() {
  char filename[MAX_NAME_LEN];
  char *keywords[2] = {"kw1", "kw2"};

  // Initialise filename to empty string
  filename[0] = '\0';
  construct_filename("20230903T123456", "12a=1", "test-title", keywords, 2, 1,
                     filename);
  assert(strcmp(filename, "20230903T123456==12a=1--test-title__kw1_kw2.md") ==
         0);

  filename[0] = '\0';
  char *keywords2[] = {};
  construct_filename("20230903T123456", "", "test-title", keywords2, 0, 1,
                     filename);
  assert(strcmp(filename, "20230903T123456--test-title.md") == 0);

  filename[0] = '\0';
  construct_filename("20230903T123456", "", "", keywords2, 0, 1, filename);
  assert(strcmp(filename, "20230903T123456.md") == 0);

  // The function doesn't complain if the ID is shorter than expected
  filename[0] = '\0';
  construct_filename("20230903T123", "", "", keywords2, 0, 1, filename);
  assert(strcmp(filename, "20230903T123.md") == 0);

  printf("All tests passed for construct_filename.\n");
}

void test_write_frontmatter_to_buffer() {
  char buffer[2048]; // Allocate a buffer to hold the output

  // Sample data
  char *id = "20240903T123456";
  char *sig = "12a=1a7=5";
  char *title = "Sample Title";
  char *keywords[] = {"keyword1", "keyword2"};
  size_t kw_count = 2;

  write_frontmatter_to_buffer(buffer, sizeof(buffer), id, sig, title, keywords,
                              kw_count);
  assert(strcmp(buffer, "---\n"
                        "title: Sample Title\n"
                        "date: 2024-09-03T12:34:56\n"
                        "tags: [keyword1, keyword2]\n"
                        "identifier: 20240903T123456\n"
                        "signature: 12a.1a7.5\n"
                        "aliases: [20240903T123456]\n"
                        "---") == 0);

  printf("All tests passed for write_frontmatter_to_buffer.\n");
}

int main() {
  test_construct_filename();
  test_write_frontmatter_to_buffer();

  return 0;
}
