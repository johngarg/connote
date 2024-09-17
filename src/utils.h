#ifndef UTILS_H_
#define UTILS_H_

// Macros
#define MAX_KEYS 16
#define MAX_NAME_LEN 512
#define UNWANTED_CHARS "[]{}!@#$%^&*()=+'\"?,.\\|;:~`‘’“”/]*"
#define ID_FORMAT "%Y%m%dT%H%M%S"
#define ID_REGEX "([0-9]{8})T([0-9]{6})"
#define ID_LEN 15
// Be careful to escape backslashes in the macro
#define ID_REGEX "([0-9]{8})T([0-9]{6})"
#define TITLE_REGEX "--([^=|\\.|_|@]*)((==.*|__.*|@@" ID_REGEX "|\\..*)*)$"
#define SIG_REGEX "==([^-|\\.|_|@]*)(--.*|__.*|@@" ID_REGEX "|\\..*)$"
#define KW_REGEX "__([^-|\\.|=|@]*)(--.*|__.*|@@" ID_REGEX "|\\..*)$"

bool is_unwanted_char(char c);
void remove_unwanted_chars(char *str);
void hyphenate(char *str);
bool file_creation_timestamp(const char *file_path, char *dest);
bool has_valid_id(const char *str);
bool file_exists(const char *filename);

#endif // UTILS_H_
