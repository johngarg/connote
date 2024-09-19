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

void replace_ch1_with_ch2_in_dest(char *source, char *dest, char ch1, char ch2) {
  size_t i;
  for (i = 0; i < strlen(source); i++) {
    if (source[i] == ch1) {
      dest[i] = ch2;
    } else {
      dest[i] = source[i];
    }
  }
  dest[i] = '\0'; // Terminate the string
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

bool file_creation_timestamp(const char *file_path, char *dest) {
  // Copies creation timestamp of file located at `file_path` to the string
  // `dest`
  struct stat file_stat;

  // Get file status information
  if (stat(file_path, &file_stat) == -1) {
    fprintf(stderr, "ERROR: Problem reading creation date of %s.\n", file_path);
    return false;
  }

  // Convert the st_ctime (metadata change time) to local time
  struct tm *t = localtime(&file_stat.st_ctime);

  // Returns the number of characters in the array, not counting the string
  // termination.
  assert(strftime(dest, ID_LEN + 1, ID_FORMAT, t) == ID_LEN);

  return true;
}

// Puts the current date and time into `dest`
bool generate_timestamp_now(char *dest) {
  struct tm *local;
  time_t t = time(NULL);
  local = localtime(&t);
  if (strftime(dest, ID_LEN + 1, ID_FORMAT, local) == 0) {
    fprintf(stderr, "ERROR: Failed to format time when generating new timestamp.\n");
    return false;
  };
  return true;
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

bool match_pattern_against_str(char *str, char *pattern, size_t start, size_t end) {
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
    return false;
  }

  // Execute the regular expression
  reti = regexec(&regex, str, 5, matches, 0);
  if (!reti && matches[1].rm_so != -1) {
    // Match found!
    // As defined, all regex expressions contain the match in index 1
    start = matches[1].rm_so;
    end = matches[1].rm_eo;
    return true;
  }

  if (reti != REG_NOMATCH) {
    // Something went wrong
    char msgbuf[128];
    regerror(reti, &regex, msgbuf, sizeof(msgbuf));
    fprintf(stderr, "ERROR: Regex match failed: %s\n", msgbuf);
  }

  // Free compiled regular expression
  regfree(&regex);

  return false;
}

// Function to copy a slice into a pre-allocated destination string
bool str_copy_slice(const char *src, size_t start, size_t end, char *dest, size_t dest_size) {
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

  return !overflow;
}

// Function to append a slice of a string to a buffer using snprintf
bool str_append_slice(const char *src, size_t start, size_t end, char *dest, size_t dest_size, size_t *current_pos) {
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

  return !overflow;
}

bool construct_filename(char *id, char *sig, char *title, char **keywords, size_t kw_count, int type,
                        char *dest_filename) {
  size_t max_len_without_ext = MAX_NAME_LEN - 4;
  size_t current_pos = 0;

  // If there is no ID, we cannot construct a filename
  if (id == NULL || id[0] == '\0') {
    fprintf(stderr, "ERROR: No ID passed to construct_filename.\n");
    return false;
  }

  str_append_slice(id, 0, strlen(id), dest_filename, max_len_without_ext, &current_pos);

  if (sig != NULL && sig[0] != '\0') {
    str_append_slice("==", 0, 2, dest_filename, max_len_without_ext, &current_pos);
    str_append_slice(sig, 0, strlen(sig), dest_filename, max_len_without_ext, &current_pos);
  }

  if (title != NULL && title[0] != '\0') {
    str_append_slice("--", 0, 2, dest_filename, max_len_without_ext, &current_pos);
    str_append_slice(title, 0, strlen(title), dest_filename, max_len_without_ext, &current_pos);
  }

  if (kw_count > 0) {
    str_append_slice("_", 0, 1, dest_filename, max_len_without_ext, &current_pos);
    for (size_t i = 0; i < kw_count; i++) {
      str_append_slice("_", 0, 1, dest_filename, max_len_without_ext, &current_pos);
      str_append_slice(keywords[i], 0, strlen(keywords[i]), dest_filename, max_len_without_ext, &current_pos);
    }
  }

  if (type == 1) {
    str_append_slice(".md", 0, 3, dest_filename, max_len_without_ext, &current_pos);
    return true;
  }

  fprintf(stderr, "ERROR: Currently only markdown format with yaml frontmatter "
                  "is supported.\n");
  return false;
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
  struct tm t = {0};
  // Parse the input string "YYYYMMDDTHHMMSS"
  sscanf(id, "%4d%2d%2dT%2d%2d%2d", &t.tm_year, &t.tm_mon, &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec);
  // Adjust the values for tm structure
  t.tm_year -= 1900; // tm_year is years since 1900
  t.tm_mon -= 1;     // tm_mon is 0-11
  // Format the time into the desired output format
  char date[20]; // Buffer to hold the formatted time
  strftime(date, sizeof(date), "%Y-%m-%dT%H:%M:%S", &t);

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
  replace_ch1_with_ch2_in_dest(sig, new_sig, '=', '.');

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

bool write_new_connote_file(char *id, char *sig, char *title, char **keywords, size_t kw_count, int type,
                            char *dest_filename) {
  // Write a new file and (1) provide it with a denote-compliant filename from
  // the data passed in and save this to `dest_filename`, and (2) write the
  // associated frontmatter to the beginning of the file.
  //
  // The `type` argument is:
  //   0: For an org file
  //   1: For a markdown file with yaml frontmatter
  //   2: For a markdown file with toml frontmatter
  //   3: For a txt file

  char buffer[2048];
  // Write the full title to the frontmatter as provided by the user
  write_frontmatter_to_buffer(buffer, sizeof(buffer), id, sig, title, keywords, kw_count);

  // TODO Clean up the title for use in the filename
  // char new_title[MAX_NAME_LEN];
  // clean_title(title, new_title);

  construct_filename(id, sig, title, keywords, kw_count, type, dest_filename);

  FILE *f = fopen(dest_filename, "w");
  if (f) {
    fprintf(f, "%s", buffer);
    fclose(f);
  }

  return true;
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
