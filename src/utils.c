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

// TODO Replace spaces and underscores with hyphens in `str`.
void hyphenate(char *str) {
  for (int i = 0; str[i]; i++) {
    if (str[i] == ' ') {
      str[i] = '-';
    } else if (str[i] == '_') {
      str[i] = '-';
    }
  }
}

bool file_creation_timestamp(const char *file_path, char *dest) {
    // Copies creation timestamp of file located at `file_path` to the string `dest`
    struct stat file_stat;

    // Get file status information
    if (stat(file_path, &file_stat) == -1) {
        fprintf(stderr, "ERROR: Problem reading creation date of %s", file_path);
        return false;
    }

    // Convert the st_ctime (metadata change time) to local time
    struct tm *t = localtime(&file_stat.st_ctime);

    // Returns the number of characters in the array, not counting the string
    // termination.
    assert(strftime(dest, ID_LEN+1, ID_FORMAT, t) == ID_LEN);

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
