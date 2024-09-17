#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <regex.h>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

#include "utils.h"

// Function to trim a string
void trimString(char *str) {
  int start = 0;
  int end = strlen(str) - 1;

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

void remove_char_from_string(char *str, char c) {

}

// Function to check if a character is in the set of unwanted characters
bool is_unwanted_char(char c) {
    return strchr(UNWANTED_CHARS, c) != NULL;
}

// Function to remove unwanted characters from a string
void remove_unwanted_chars(char *str) {
    int src = 0, dst = 0;

    while (str[src] != '\0') {
        if (!is_unwanted_char(str[src])) {
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
    // Copies creation timestamp of file located at `file_path` to the string `dest`
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
    assert(strftime(dest, ID_LEN+1, ID_FORMAT, t) == ID_LEN);

    return true;
}

// Puts the current date and time into `dest`
bool generate_timestamp_now(char *dest) {
    struct tm *local;
    time_t t = time(NULL);
    local = localtime(&t);
    if (strftime(dest, ID_LEN+1, ID_FORMAT, local) == 0) {
        fprintf(stderr, "ERROR: Failed to format time when generating new timestamp.\n");
        return false;
    };
    return true;
}

bool has_valid_id(const char *str) {
    // Expects input like "20240908T123445". This would need to be changed if
    // ID_FORMAT is changed
    if (str[8] != 'T') return false;

    for (int i = 0; i < 8; ++i) {
        if (!isdigit(str[i])) return false;
    }

    for (int i = 9; i < ID_LEN; ++i) {
        if (!isdigit(str[i])) return false;
    }

    return true;
}

void append_slice(const char* src, size_t start, size_t end, char* dest, size_t dest_size, size_t* current_pos) {
    size_t length = end - start;

    // Ensure we don't append more than the remaining buffer can hold
    if (*current_pos + length >= dest_size) {
        length = dest_size - *current_pos - 1;  // Leave space for null terminator
    }

    // Append the slice using snprintf and update the current position
    snprintf(dest + *current_pos, dest_size - *current_pos, "%.*s", (int)length, src + start);
    *current_pos += length;  // Update the current position
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
bool str_copy_slice(const char* src, size_t start, size_t end, char* dest, size_t dest_size) {
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
bool str_append_slice(const char* src, size_t start, size_t end, char* dest, size_t dest_size, size_t* current_pos) {
    size_t length = end - start;
    bool overflow = false;

    // Ensure we don't append more than the remaining buffer can hold
    if (*current_pos + length >= dest_size) {
        length = dest_size - *current_pos - 1;  // Leave space for null terminator
        overflow = true;
    }

    // Append the slice using snprintf and update the current position
    snprintf(dest + *current_pos, dest_size - *current_pos, "%.*s", (int)length, src + start);
    *current_pos += length;  // Update the current position

    return !overflow;
}

bool construct_filename(char *id, const char *sig, const char *title, char **keywords, size_t kw_count, int type, char *dest_filename) {
    size_t max_len_without_ext = MAX_NAME_LEN-4;
    size_t current_pos = 0;

    // If there is no ID, we cannot construct a filename
    if (id == NULL || id[0] == '\0') {
        fprintf(stderr, "ERROR: No ID passed to construct_filename.\n");
        return false;
    }

    str_append_slice(id, 0, ID_LEN, dest_filename, max_len_without_ext, &current_pos);

    if (sig != NULL && sig[0] != '\0') {
        str_append_slice("==", 0, 2, dest_filename, max_len_without_ext, &current_pos);
        str_append_slice(sig, 0, strlen(sig), dest_filename, max_len_without_ext, &current_pos);
    }

    if (title != NULL && title[0] != '\0') {
        str_append_slice("--", 0, 2, dest_filename, max_len_without_ext,
                         &current_pos);
        str_append_slice(title, 0, strlen(title), dest_filename,
                         max_len_without_ext, &current_pos);
    }

    if (kw_count > 0) {
        str_append_slice("_", 0, 1, dest_filename, max_len_without_ext,
                         &current_pos);
        for (int i = 0; i < kw_count; i++) {
            str_append_slice("_", 0, 1, dest_filename, max_len_without_ext,
                             &current_pos);
            str_append_slice(keywords[i], 0, strlen(keywords[i]), dest_filename,
                             max_len_without_ext, &current_pos);
        }
    }

    if (type == 1) {
        str_append_slice(".md", 0, 3, dest_filename, max_len_without_ext, &current_pos);
        return true;
    }

    fprintf(stderr, "ERROR: Currently only markdown format with yaml frontmatter is supported.\n");
    return false;
}


bool write_new_connote_file(char *id, const char *title, const char *signature, char **keywords, int type, char *dest_filename) {
    // Write a new file and (1) provide it with a denote-compliant filename from
    // the data passed in and save this to `dest_filename`, and (2) write the
    // associated frontmatter to the beginning of the file.
    //
    // The `type` argument is:
    //   0: For an org file
    //   1: For a markdown file with yaml frontmatter
    //   2: For a markdown file with toml frontmatter
    //   3: For a txt file

    printf("Write new connote file!\n");

    return true;
}
