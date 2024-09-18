#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>

// Macros
#define MAX_KEYS 16
#define MAX_NAME_LEN 512
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


// string operations
void remove_unwanted_chars(char *str, const char *unwanted_chars);
void replace_spaces_and_underscores(char *str, char s);
void replace_non_ascii(char *str);
void append_slice(const char *src, size_t start, size_t end, char *dest,
                  size_t dest_size, size_t *current_pos);
bool match_pattern_against_str(char *str, char *pattern, size_t start,
                               size_t end);
bool str_copy_slice(const char *src, size_t start, size_t end, char *dest,
                    size_t dest_size);
bool str_append_slice(const char *src, size_t start, size_t end, char *dest,
                      size_t dest_size, size_t *current_pos);
void replace_ch1_with_ch2_in_dest(char *source, char *dest, char ch1, char ch2);
void trim_string(char *str);
void replace_consecutive_chars(char *str, char c);
void downcase(char *str);
void rtrim_tokens(char *str, const char *unwanted_chars);
void ltrim_tokens(char *str, const char *unwanted_chars);

// file stuff
bool file_creation_timestamp(const char *file_path, char *dest);
bool has_valid_id(const char *str);
bool file_exists(const char *filename);
bool generate_timestamp_now(char *dest);
bool construct_filename(char *id, char *sig, char *title, char **keywords,
                        size_t kw_count, int type, char *dest_filename);
bool write_new_connote_file(char *id, char *sig, char *title, char **keywords,
                            size_t kw_count, int type, char *dest_filename);
void write_frontmatter_to_buffer(char *buffer, size_t buffer_size, char *id,
                                 char *sig, char *title, char **keywords,
                                 size_t kw_count);

// Component sluggification
void slug_hyphenate(char *str);
void slug_put_equals(char *str);
void sluggify_title(char *str);
void sluggify_signature(char *str);
void sluggify_keyword(char *str);
void sluggify_keywords(char **keywords, size_t kw_count);

#endif // UTILS_H_
