#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "../src/utils.h"

void test_format_file_name() {
  char filename[MAX_PATH_LEN];
  char kw1[16] = "kw1";
  char kw2[16] = "kw2";
  char *keywords[] = {kw1, kw2};

  // Initialise filename to empty string
  filename[0] = '\0';
  char sig[32] = "12a=1";
  char title[64] = "test-title";
  char ext[8] = ".md";
  char dir_path[64] = "/tmp/connote/";
  format_file_name(dir_path, "20230903T123456", sig, title, keywords, 2, ext, filename);
  assert(strcmp(filename, "/tmp/connote/20230903T123456==12a=1--test-title__kw1_kw2.md") == 0);

  filename[0] = '\0';
  char *keywords2[] = {};
  format_file_name(dir_path, "20230903T123456", "", title, keywords2, 0, ".md", filename);
  assert(strcmp(filename, "/tmp/connote/20230903T123456--test-title.md") == 0);

  filename[0] = '\0';
  format_file_name(dir_path, "20230903T123456", "", "", keywords2, 0, ".md", filename);
  assert(strcmp(filename, "/tmp/connote/20230903T123456.md") == 0);

  printf("All tests passed for format_file_name.\n");
}

void test_write_frontmatter_to_buffer() {
  char buffer[2048]; // Allocate a buffer to hold the output

  // Sample data
  char id[16] = "20240903T123456";
  char sig[32] = "12a=1a7=5";
  char title[64] = "Sample Title";
  char kw1[32] = "keyword1";
  char kw2[32] = "keyword2";
  char *keywords[] = {kw1, kw2};
  size_t kw_count = 2;

  write_frontmatter_to_buffer(buffer, sizeof(buffer), id, sig, title, keywords, kw_count);
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
  /* char test_str1[32] = "..@#H*έl/lo!!!, --test- "; */
  char test_str1[32] = "12a=1";
  remove_unwanted_chars(test_str1, "[]{}!@#$%^&*()+'\"?,.\\|;:~`‘’“”/-");
  /* assert(strcmp(test_str1, "Hέllo --test- ") == 0); */
  assert(strcmp(test_str1, "12a=1") == 0);

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

void test_regex_functions() {

  size_t start = 0;
  size_t end = 0;
  int outcome;

  char *filename = "20240923T174318==12=a.md";

  outcome = match_pattern_against_str(filename, SIG_REGEX, &start, &end);
  assert(outcome == SUCCESS);
  assert(start == 17);
  assert(end == 21);

  // Just one equals sign should fail
  outcome = match_pattern_against_str("20240923T174318=12=a.md", SIG_REGEX, &start, &end);
  assert(outcome == FAILURE);

  // Filename matching functions
  char sig[MAX_SIG_LEN] = {0};
  try_match_and_write_component(filename, sig, SIG_REGEX, MAX_SIG_LEN);
  assert(strcmp(sig, "12=a") == 0);

  char *filename_2 = "20240923T174318==12=a__kw1_kw2_kw3.md";
  char kws[MAX_KW_LEN * MAX_KEYS] = {0};
  try_match_and_write_component(filename_2, kws, KW_REGEX, MAX_KW_LEN * MAX_KEYS);
  assert(strcmp(kws, "kw1_kw2_kw3") == 0);

  printf("All tests passed for regex functions.\n");
}

int main() {
  test_format_file_name();
  test_write_frontmatter_to_buffer();
  test_sluggify_functions();
  test_regex_functions();

  return 0;
}
