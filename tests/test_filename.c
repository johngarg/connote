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

void test_sluggify_functions() {
  char test_str1[32] = "..@#H*έl/lo!!!, --test- ";
  remove_unwanted_chars(test_str1, UNWANTED_CHARS);
  assert(strcmp(test_str1, "Hέllo --test- ") == 0);

  // Tests from denote-test
  char test_str2[32] = "this-is-!@#test";
  sluggify_title(test_str2);
  assert(strcmp(test_str2, "this-is-test") == 0);

  char test_str3[64] = "There are no-ASCII ： characters ｜ here 😀";
  replace_non_ascii(test_str3);
  assert(strcmp(test_str3, "There are no-ASCII     characters     here     ") == 0);

  char test_str4[64] = "__  This is   a    test  __  ";
  slug_hyphenate(test_str4);
  assert(strcmp(test_str4, "This-is-a-test") == 0);

  char test_str5[64] = " ___ !~!!$%^ This iS a tEsT ++ ?? ";
  sluggify_title(test_str5);
  assert(strcmp(test_str5, "this-is-a-test") == 0);

  char test_str6[64] = "__  This is   a    test  __  ";
  slug_put_equals(test_str6);
  assert(strcmp(test_str6, "This=is=a=test") == 0);

  char test_str7[64] = "--- ___ !~!!$%^ This -iS- a tEsT ++ ?? ";
  sluggify_signature(test_str7);
  assert(strcmp(test_str7, "this=is=a=test") == 0);

  char test_str8[64] = "--- ___ !~!!$%^ This -iS- a tEsT ++ ?? ";
  sluggify_keyword(test_str8);
  assert(strcmp(test_str8, "thisisatest") == 0);

  printf("All tests passed for sluggify functions.\n");
}

int main() {
  test_construct_filename();
  test_write_frontmatter_to_buffer();
  test_sluggify_functions();

  return 0;
}
