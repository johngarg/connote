#include <assert.h>
#include <ctype.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

// Function to check if directory exists and create it if it doesn't
int make_directory_if_not_exists(const char *path) {
  struct stat st = {0};

  // Check if the directory exists
  if (stat(path, &st) == -1) {
    // Directory doesn't exist, attempt to create it
    if (mkdir(path, 0700) == -1) {
      fprintf(stderr, "ERROR: Making connote directory failed.\n");
      return FAILURE;
    } else {
      printf("Directory created: %s\n", path);
    }
  } else {
    printf("Directory already exists: %s\n", path);
  }

  return SUCCESS;
}

// Function to trim a string
void trim_string(char *str) {
  size_t start = 0;
  size_t end = strlen(str) - 1;

  // Remove leading whitespace
  while (isspace(str[start])) {
    start++;
  }

  // Remove trailing whitespace
  while (end > start && isspace(str[end])) {
    end--;
  }

  // If the string was trimmed, adjust the null terminator
  if (start > 0 || end < (strlen(str) - 1)) {
    memmove(str, str + start, end - start + 1);
    str[end - start + 1] = '\0';
  }
}

// Function to remove unwanted characters from a string. The predicate function
// should return true if the char is unwanted and should be removed
void remove_unwanted_chars(char *str, const char *unwanted_chars) {
  int src = 0, dst = 0;

  while (str[src] != '\0') {
    if (!(strchr(unwanted_chars, str[src]) != NULL)) {
      str[dst++] = str[src];
    }
    src++;
  }

  // Null-terminate the result string
  str[dst] = '\0';
}

bool file_exists(const char *filename) {
  return access(filename, F_OK) != -1; // F_OK checks for file existence
}

// Replace spaces and underscores with `s` in `str`.
void replace_spaces_and_underscores(char *str, char s) {
  for (int i = 0; str[i]; i++) {
    if (str[i] == ' ') {
      str[i] = s;
    } else if (str[i] == '_') {
      str[i] = s;
    }
  }
}

void replace_ch1_with_ch2_in_dest(char *source, char *dest, char ch1, char ch2, size_t dest_size) {
  // NULL pointer check
  if (source == NULL || dest == NULL) {
    return; // Handle the error as appropriate (e.g., log an error)
  }

  // Calculate the length of the source string
  size_t source_len = strlen(source);

  // Ensure dest has enough space to hold the result (including null terminator)
  if (dest_size <= source_len) {
    // Error handling: dest is not large enough
    return;
  }

  // Iterate over the source string and replace characters
  for (size_t i = 0; i < source_len; i++) {
    if (source[i] == ch1) {
      dest[i] = ch2;
    } else {
      dest[i] = source[i];
    }
  }

  // Null-terminate the destination string
  dest[source_len] = '\0';
}

// Replace non-ascii characters with spaces
void replace_non_ascii(char *str) {
  while (*str) {
    if (*str < 0 || *str > 127) {
      *str = ' ';
    }
    str++;
  }
}

int file_creation_timestamp(const char *file_path, char *dest) {
  // Copies creation timestamp of file located at `file_path` to the string
  // `dest`
  struct stat file_stat;

  // Get file status information
  if (stat(file_path, &file_stat) == -1) {
    fprintf(stderr, "ERROR: Problem reading creation date of %s.\n", file_path);
    return FAILURE;
  }

  // Convert the st_ctime (metadata change time) to local time
  struct tm *t = localtime(&file_stat.st_ctime);

  // Returns the number of characters in the array, not counting the string
  // termination.
  assert(strftime(dest, ID_LEN + 1, ID_FORMAT, t) == ID_LEN);

  return SUCCESS;
}

// Puts the current date and time into `dest`
int generate_timestamp_now(char *dest) {
  struct tm *local;
  time_t t = time(NULL);
  local = localtime(&t);
  if (strftime(dest, ID_LEN + 1, ID_FORMAT, local) == 0) {
    fprintf(stderr, "ERROR: Failed to format time when generating new timestamp.\n");
    return FAILURE;
  };
  return SUCCESS;
}

int match_pattern_against_str(char *str, char *pattern, size_t *start, size_t *end) {
  // Matches the regex `pattern` against `str` and puts the start and end
  // indices of the successful match in `start` and `end`. Returns `true` if
  // match found and false otherwise
  regex_t regex;
  regmatch_t matches[5];
  int reti;

  // Compile the regular expression
  reti = regcomp(&regex, pattern, REG_EXTENDED);
  if (reti) {
    fprintf(stderr, "Could not compile regex\n");
    return FAILURE;
  }

  // Execute the regular expression
  reti = regexec(&regex, str, 5, matches, 0);
  if (!reti && matches[1].rm_so != -1) {
    // Match found!
    // As defined, all regex expressions contain the match in index 1
    *start = (size_t)matches[1].rm_so;
    *end = (size_t)matches[1].rm_eo;
    return SUCCESS;
  }

  if (reti != REG_NOMATCH) {
    // Something went wrong
    char msgbuf[128];
    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
    fprintf(stderr, "ERROR: Regex match failed: %s\n", msgbuf);
  }

  // Free compiled regular expression
  regfree(&regex);

  return FAILURE;
}

// Function to copy a slice into a pre-allocated destination string
int str_copy_slice(const char *src, size_t start, size_t end, char *dest, size_t dest_size) {
  size_t length = end - start;
  bool overflow = false;

  // Ensure we don't copy more than the destination can hold
  if (length >= dest_size) {
    length = dest_size - 1;
    overflow = true;
  }

  // Copy the slice into the destination string
  strncpy(dest, src + start, length);

  // Add null terminator to the destination string
  dest[length] = '\0';

  return overflow ? FAILURE : SUCCESS;
}

// Function to append a slice of a string to a buffer using snprintf
int str_append_slice(const char *src, size_t start, size_t end, char *dest, size_t dest_size, size_t *current_pos) {
  size_t length = end - start;
  bool overflow = false;

  // Ensure we don't append more than the remaining buffer can hold
  if (*current_pos + length >= dest_size) {
    length = dest_size - *current_pos - 1; // Leave space for null terminator
    overflow = true;
  }

  // Append the slice using snprintf and update the current position
  snprintf(dest + *current_pos, dest_size - *current_pos, "%.*s", (int)length, src + start);
  *current_pos += length; // Update the current position

  return overflow ? FAILURE : SUCCESS;
}

// In denote this takes an extra parameter: `dir_path`
int format_file_name(char *dir_path, char *id, char *sig, char *title, char **keywords, size_t kw_count,
                     char *extension, char *dest_filename) {
  size_t max_len_without_ext = MAX_PATH_LEN - strlen(extension) - 1;
  size_t current_pos = 0;

  // If there is no ID, we cannot construct a filename
  if (id == NULL || id[0] == '\0') {
    fprintf(stderr, "ERROR: No ID passed to format_file_name.\n");
    return FAILURE;
  }

  // If there is no directory we cannot format the file path
  if (dir_path == NULL || dir_path[0] == '\0') {
    fprintf(stderr, "ERROR: No directory path passed to format_file_name.\n");
    return FAILURE;
  }

  str_append_slice(dir_path, 0, strlen(dir_path), dest_filename, max_len_without_ext, &current_pos);

  assert(strlen(id) == ID_LEN);
  str_append_slice(id, 0, strlen(id), dest_filename, max_len_without_ext, &current_pos);

  if (sig != NULL && sig[0] != '\0') {
    str_append_slice("==", 0, 2, dest_filename, max_len_without_ext, &current_pos);
    // Sluggify signature
    sluggify_signature(sig);
    str_append_slice(sig, 0, strlen(sig), dest_filename, max_len_without_ext, &current_pos);
  }

  if (title != NULL && title[0] != '\0') {
    str_append_slice("--", 0, 2, dest_filename, max_len_without_ext, &current_pos);
    // Sluggify title
    sluggify_title(title);
    str_append_slice(title, 0, strlen(title), dest_filename, max_len_without_ext, &current_pos);
  }

  if (kw_count > 0) {
    str_append_slice("_", 0, 1, dest_filename, max_len_without_ext, &current_pos);
    for (size_t i = 0; i < kw_count; i++) {
      str_append_slice("_", 0, 1, dest_filename, max_len_without_ext, &current_pos);
      // Sluggify keyword
      sluggify_keyword(keywords[i]);
      str_append_slice(keywords[i], 0, strlen(keywords[i]), dest_filename, max_len_without_ext, &current_pos);
    }
  }

  if (extension != NULL && extension[0] == '.') {
    str_append_slice(extension, 0, strlen(extension), dest_filename, max_len_without_ext, &current_pos);
  }

  return SUCCESS;
}

void date_from_id(char *id, char *dest) {
  struct tm t = {0};
  // Parse the input string "YYYYMMDDTHHMMSS"
  sscanf(id, "%4d%2d%2dT%2d%2d%2d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
  // Adjust the values for tm structure
  t.tm_year -= 1900; // tm_year is years since 1900
  t.tm_mon -= 1;     // tm_mon is 0-11
  // Format the time into the desired output format
  strftime(dest, 32, "%Y-%m-%dT%H:%M:%S", &t);
}

// Function to write frontmatter to a buffer instead of a file
void write_frontmatter_to_buffer(char *buffer, size_t buffer_size, char *id, char *sig, char *title, char **keywords,
                                 size_t kw_count) {
  char *buf_ptr = buffer;
  size_t remaining_size = buffer_size;

  // Use snprintf to write to the buffer instead of fprintf
  int written = snprintf(buf_ptr, remaining_size, "---\n");
  buf_ptr += written;
  remaining_size -= written;

  // TITLE
  written = snprintf(buf_ptr, remaining_size, "title: %s\n", title ? title : "");
  buf_ptr += written;
  remaining_size -= written;

  // DATE
  // Get date from ID
  char date[20]; // Buffer to hold the formatted time
  date_from_id(id, date);

  // Write the formatted date to the buffer
  written = snprintf(buf_ptr, remaining_size, "date: %s\n", date);
  buf_ptr += written;
  remaining_size -= written;

  // KEYWORDS
  // I like to call keywords tags
  written = snprintf(buf_ptr, remaining_size, "tags: [");
  buf_ptr += written;
  remaining_size -= written;

  // Iterate through the keywords and print
  for (size_t i = 0; i < kw_count; i++) {
    written = snprintf(buf_ptr, remaining_size, "%s", keywords[i]);
    buf_ptr += written;
    remaining_size -= written;

    if (i < kw_count - 1) {
      written = snprintf(buf_ptr, remaining_size, ", ");
      buf_ptr += written;
      remaining_size -= written;
    }
  }

  written = snprintf(buf_ptr, remaining_size, "]\n");
  buf_ptr += written;
  remaining_size -= written;

  // ID
  written = snprintf(buf_ptr, remaining_size, "identifier: %s\n", id);
  buf_ptr += written;
  remaining_size -= written;

  // SIGNATURE
  // Replace '=' with '.' in the signature
  char new_sig[MAX_SIG_LEN];
  replace_ch1_with_ch2_in_dest(sig, new_sig, '=', '.', MAX_SIG_LEN);

  written = snprintf(buf_ptr, remaining_size, "signature: %s\n", sig ? new_sig : "");
  buf_ptr += written;
  remaining_size -= written;

  // ALIASES
  written = snprintf(buf_ptr, remaining_size, "aliases: [%s]\n", id);
  buf_ptr += written;
  remaining_size -= written;

  // Finalize frontmatter
  written = snprintf(buf_ptr, remaining_size, "---");
}

int connote_file(char *dir_path, char *id, char *sig, char *title, char **keywords, size_t kw_count, char *extension,
                 char *dest_filename) {
  // Write a new file and (1) provide it with a denote-compliant filename from
  // the data passed in and save this to `dest_filename`, and (2) write the
  // associated frontmatter to the beginning of the file.

  char buffer[2048];
  // Write the full title to the frontmatter as provided by the user
  write_frontmatter_to_buffer(buffer, sizeof(buffer), id, sig, title, keywords, kw_count);

  if (strcmp(extension, ".md") != 0) {
    fprintf(stderr, "ERROR: Only markdown files with yaml frontmatter are currently supported.\n");
    return FAILURE;
  }

  format_file_name(dir_path, id, sig, title, keywords, kw_count, extension, dest_filename);
  printf("dest_filename: %s\n", dest_filename);

  FILE *f = fopen(dest_filename, "w");
  if (f) {
    fprintf(f, "%s", buffer);
    fclose(f);
  }

  return SUCCESS;
}

void downcase(char *str) {
  for (int i = 0; str[i]; i++) {
    str[i] = tolower((unsigned char)str[i]);
  }
}

void replace_consecutive_chars(char *str, char c) {
  int src = 0, dst = 0;
  char last_char = '\0'; // To track the previous character

  while (str[src] != '\0') {
    if (str[src] != c || last_char != c) {
      str[dst++] = str[src]; // Copy character to destination
    }
    last_char = str[src]; // Update the last character
    src++;
  }

  // Null-terminate the resulting string
  str[dst] = '\0';
}

// Function to trim unwanted tokens from the left of the string
void ltrim_tokens(char *str, const char *unwanted_chars) {
  int start = 0;

  // Move 'start' forward as long as the current character is one of the unwanted ones
  while (str[start] != '\0' && strchr(unwanted_chars, str[start]) != NULL) {
    start++;
  }

  // Shift the remaining part of the string to the beginning
  if (start > 0) {
    int i = 0;
    while (str[start] != '\0') {
      str[i++] = str[start++];
    }
    str[i] = '\0'; // Null-terminate the result string
  }
}

// Function to trim unwanted tokens from the right of the string
void rtrim_tokens(char *str, const char *unwanted_chars) {
  int len = strlen(str);

  // Start from the end of the string and move backwards
  while (len > 0 && strchr(unwanted_chars, str[len - 1]) != NULL) {
    len--; // Reduce length for each unwanted character
  }

  // Null-terminate the string at the new length
  str[len] = '\0';
}

void slug_hyphenate(char *str) {
  // Replace spaces and underscores with hyphens in str. Also replace multiple
  // hyphens with a single one and remove any leading and trailing hyphen.
  replace_spaces_and_underscores(str, '-');
  replace_consecutive_chars(str, '-');
  ltrim_tokens(str, "-");
  rtrim_tokens(str, "-");
}

void slug_put_equals(char *str) {
  // Replace spaces and underscores with equals signs in str. Also replace
  // multiple equals signs with a single one and remove any leading and trailing
  // signs.
  replace_spaces_and_underscores(str, '=');
  replace_consecutive_chars(str, '=');
  ltrim_tokens(str, "=");
  rtrim_tokens(str, "=");
}

void sluggify_title(char *str) {
  const char *unwanted_chars = "[]{}!@#$%^&*()+'\"?,.\\|;:~`‘’“”/=";
  // Start by removing whitespace characters at beginning and end
  trim_string(str);
  // Remove unwanted characters from the string
  remove_unwanted_chars(str, unwanted_chars);
  // Replace spaces and underscores with hyphen for title
  slug_hyphenate(str);
  // Downcase everything
  downcase(str);
  // Remove consecutive tokens when they mark parts of the file name
  replace_consecutive_chars(str, '-');
  replace_consecutive_chars(str, '=');
  replace_consecutive_chars(str, '@');
  replace_consecutive_chars(str, '_');
  // Trim string
  rtrim_tokens(str, "=@_+-");
}

void sluggify_signature(char *str) {
  const char *unwanted_chars = "[]{}!@#$%^&*()+'\"?,.\\|;:~`‘’“”/-";
  // Start by removing whitespace characters at beginning and end
  trim_string(str);
  // Remove unwanted characters from the string
  remove_unwanted_chars(str, unwanted_chars);
  // Replace spaces and underscores with equals signs for signature
  slug_put_equals(str);
  // Downcase everything
  downcase(str);
  // Remove consecutive tokens when they mark parts of the file name
  replace_consecutive_chars(str, '-');
  replace_consecutive_chars(str, '=');
  replace_consecutive_chars(str, '@');
  replace_consecutive_chars(str, '_');
  // Trim string
  rtrim_tokens(str, "=@_+-");
}

void sluggify_keyword(char *str) {
  const char *unwanted_chars = "[]{}!@#$%^&*()+'\"?,.\\|;:~`‘’“”/_ -=";
  // Start by removing whitespace characters at beginning and end
  trim_string(str);
  // Remove unwanted characters from the string
  remove_unwanted_chars(str, unwanted_chars);
  // Downcase everything
  downcase(str);
  // Remove consecutive tokens when they mark parts of the file name
  replace_consecutive_chars(str, '-');
  replace_consecutive_chars(str, '=');
  replace_consecutive_chars(str, '@');
  replace_consecutive_chars(str, '_');
  // Trim string
  rtrim_tokens(str, "=@_+-");
}

void sluggify_keywords(char **keywords, size_t kw_count) {
  for (size_t i = 0; i < kw_count; i++) {
    sluggify_keyword(keywords[i]);
  }
}

bool has_valid_id(const char *str) {
  // Expects input like "20240908T123445". This would need to be changed if
  // ID_FORMAT is changed
  if (str[8] != 'T')
    return false;

  for (int i = 0; i < 8; ++i) {
    if (!isdigit(str[i]))
      return false;
  }

  for (int i = 9; i < ID_LEN; ++i) {
    if (!isdigit(str[i]))
      return false;
  }

  return true;
}

void read_id(const char *filename, char *id) {
  strncpy(id, filename, ID_LEN);
  id[ID_LEN] = '\0';
}

// Try and match the `regex` pattern against the filename. If there is a match,
// write the match to `component` and return SUCCESS, otherwise return FAILURE
int try_match_and_write_component(char *filename, char *component, char *regex, size_t component_len) {
  size_t start = 0;
  size_t end = 0;

  int outcome = match_pattern_against_str(filename, regex, &start, &end);
  if (outcome == SUCCESS) {
    str_copy_slice(filename, start, end, component, component_len);
    return SUCCESS;
  }

  return FAILURE;
}

int split_at_char(char *str, char ch, char **array, size_t array_size, size_t max_str_len) {
  if (str == NULL || array == NULL || array_size == 0 || max_str_len == 0) {
    return -1; // Error: Invalid input
  }

  size_t count = 0; // Tracks how many substrings are found
  char *start = str;
  char *end = NULL;

  while ((end = strchr(start, ch)) != NULL) {
    if (count >= array_size) {
      return -1; // Error: Array size is too small
    }

    // Get the length of the current substring
    size_t length = end - start;

    // Limit the length to max_str_len - 1 to ensure space for the null
    // terminator
    if (length >= max_str_len) {
      length = max_str_len - 1;
    }

    // Copy the substring into the pre-allocated buffer
    strncpy(array[count], start, length);
    array[count][length] = '\0'; // Null-terminate the substring
    count++;

    start = end + 1; // Move past the delimiter
  }

  // Add the last part after the final delimiter
  if (count < array_size) {
    // Limit the length of the final substring
    size_t length = strlen(start);
    if (length >= max_str_len) {
      length = max_str_len - 1;
    }

    strncpy(array[count], start, length);
    array[count][length] = '\0'; // Null-terminate the substring
    count++;
  } else {
    return -1; // Error: Array size is too small
  }

  return count; // Return the number of substrings
}
