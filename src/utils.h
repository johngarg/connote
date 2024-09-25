#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>
#include <stdio.h>

// Macros
#define MAX_KEYS 16
#define MAX_KW_LEN 64
#define MAX_TITLE_LEN 512
#define MAX_PATH_LEN 2048
#define MAX_SIG_LEN 64
#define UNWANTED_CHARS "[]{}!@#$%^&*()=+'\"?,.\\|;:~`‘’“”/]*"
#define ID_FORMAT "%Y%m%dT%H%M%S"
#define ID_LEN 15
// Be careful to escape backslashes in the macro
#define ID_REGEX "([0-9]{8}T[0-9]{6})"
#define TITLE_REGEX "--([^=|\\.|_|@]*)(==.*|__.*|@@" ID_REGEX "|\\..*)$"
#define SIG_REGEX "==([^-|\\.|_|@]*)(--.*|__.*|@@" ID_REGEX "|\\..*)$"
#define KW_REGEX "__([^-|\\.|=|@]*)(--.*|__.*|@@" ID_REGEX "|\\..*)$"
#define EXT_REGEX "(\\..*)"

enum ErrorCode { SUCCESS = 0, FAILURE = -1 };

// string operations
void remove_unwanted_chars(char *str, const char *unwanted_chars);
void replace_spaces_and_underscores(char *str, char s);
void replace_non_ascii(char *str);
void append_slice(const char *src, size_t start, size_t end, char *dest, size_t dest_size, size_t *current_pos);
int match_pattern_against_str(char *str, char *pattern, size_t *start, size_t *end);
int str_copy_slice(const char *src, size_t start, size_t end, char *dest, size_t dest_size);
int str_append_slice(const char *src, size_t start, size_t end, char *dest, size_t dest_size, size_t *current_pos);
void replace_ch1_with_ch2_in_dest(char *source, char *dest, char ch1, char ch2, size_t dest_size);
void trim_string(char *str);
void replace_consecutive_chars(char *str, char c);
void downcase(char *str);
void rtrim_tokens(char *str, const char *unwanted_chars);
void ltrim_tokens(char *str, const char *unwanted_chars);
int split_at_char(char *str, char ch, char **array, size_t array_size, size_t max_str_len);

// Dir stuff
int make_directory_if_not_exists(const char *path);

// file stuff
int file_creation_timestamp(const char *file_path, char *dest);
bool file_exists(const char *filename);
int generate_timestamp_now(char *dest);
int format_file_name(char *dir_path, char *id, char *sig, char *title, char **keywords, size_t kw_count,
                     char *extension, char *dest_filename);
int connote_file(char *dir_path, char *id, char *sig, char *title, char **keywords, size_t kw_count, char *extension,
                 char *dest_filename);
void write_frontmatter_to_buffer(char *buffer, size_t buffer_size, char *id, char *sig, char *title, char **keywords,
                                 size_t kw_count);

// Component sluggification
void slug_hyphenate(char *str);
void slug_put_equals(char *str);
void sluggify_title(char *str);
void sluggify_signature(char *str);
void sluggify_keyword(char *str);
void sluggify_keywords(char **keywords, size_t kw_count);

// Reading filename
bool has_valid_id(const char *str);
void read_id(const char *filename, char *id);
int try_match_and_write_component(char *filename, char *component, char *regex, size_t component_len);
#endif // UTILS_H_
