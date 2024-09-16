#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <regex.h>

// connote <cmd> --title <title> --keywords <kw1> <kw2> --sig <sig>

#define MAX_KEYS 16
#define MAX_NAME_LEN 512

#define ID_FORMAT "%Y%m%dT%H%M%S"
#define ID_REGEX "([0-9]{8})T([0-9]{6})"
#define ID_LEN 15

// Be careful to escape backslashes in the macro
#define ID_REGEX "([0-9]{8})T([0-9]{6})"
#define TITLE_REGEX "--([^=|\\.|_|@]*)((==.*|__.*|@@" ID_REGEX "|\\..*)*)$"
#define SIG_REGEX "==([^-|\\.|_|@]*)(--.*|__.*|@@" ID_REGEX "|\\..*)$"
#define KW_REGEX "__([^-|\\.|=|@]*)(--.*|__.*|@@" ID_REGEX "|\\..*)$"

#define UNWANTED_CHARS "[]{}!@#$%^&*()=+'\"?,.\\|;:~`‘’“”/]*"

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

void test_argument_parsing(char **argv, int argc, const char *title, const char *sig, int kw_count, char **keywords) {
    printf("TITLE: %s\n", title ? title : "None");
    printf("KEYWORDS: \n");
    if (kw_count > 0) {
        for (int i = 0; i < kw_count; i++) {
            printf("  kw[%d]: %s\n", i, keywords[i]);
        }
    } else {
        printf("None\n");
    }
    printf("SIGNATURE: %s\n", sig ? sig : "None");

    for (int index = optind; index < argc; index++) {
        printf("NON-OPTION: %s\n", argv[index]);
    }
}

bool file_exists(const char *filename) {
    return access(filename, F_OK) != -1; // F_OK checks for file existence
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

void print_usage() {
    printf("Usage: connote file --title <title> --keywords <kw1> <kw2> <kw3> --sig <signature>\n");
}

bool has_valid_id(const char *s) {
    // Expects input like "20240908T123445". This would need to be changed if
    // ID_FORMAT is changed
    if (s[8] != 'T') return false;

    for (int i = 0; i < 8; ++i) {
        if (!isdigit(s[i])) return false;
    }

    for (int i = 9; i < ID_LEN; ++i) {
        if (!isdigit(s[i])) return false;
    }

    return true;
}

void make_filename(char *dest, char *id, const char *sig, const char *title, char **keywords) {
    assert(0 && "Not implemented");
}

int main(int argc, char *argv[]) {
    // Expect at least three args: `connote <cmd> <arg>`
    if (argc < 3) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    // Initialise the filename data
    const char *title = NULL;
    const char *sig = NULL;
    char *keywords[MAX_KEYS];  // Array to store keywords, assuming max 10
    int kw_count = 0;

    // Define long options
    static struct option long_options[] = {
        {"title", required_argument, 0, 't'},
        {"keywords", required_argument, 0, 'k'},
        {"sig", required_argument, 0, 's'},
        {"from-yaml", no_argument, 0, 'y'},
        {0, 0, 0, 0}  // End of options
    };

    // Parsing options
    int opt;
    bool signature_set = false;
    bool title_set = false;
    bool keywords_set = false;
    while ((opt = getopt_long(argc, argv, "t:k:s:y", long_options, NULL)) != -1) {
        switch (opt) {
            case 't':
                title = optarg;  // Get title argument
                title_set = true;
                break;
            case 'k':
                // Get keywords, assuming they are separated by spaces and provided as multiple arguments
                keywords[kw_count++] = optarg;
                while (optind < argc && argv[optind][0] != '-') {
                    keywords[kw_count++] = argv[optind++];
                    if (kw_count >= MAX_KEYS) {
                        printf("ERROR: Too many keywords, max allowed is %d.\n", MAX_KEYS);
                        break;
                    }
                }
                keywords_set = true;
                break;
            case 's':
                sig = optarg;  // Get signature argument
                signature_set = true;
                break;
            case 'y':
                // This is reached when --from-yaml is encountered
                printf("From YAML flag is set.\n");
                break;
            default:
                exit(EXIT_FAILURE);
        }
    }

    // Output the parsed arguments for testing purposes
    test_argument_parsing(argv, argc, title, sig, kw_count, keywords);

    // Allocate a buffer of 15 chars + the null terminator for the ID
    char id[ID_LEN+1];

    bool creation_timestamp_okay;
    // connote file command
    if (argc > 1 && strcmp(argv[optind], "file") == 0) {
        // Increment past the <cmd> argument
        optind++;
        // Loop over input files and rename them
        for (int i = optind; i < argc; i++) {

            // Check whether the file exists
            assert(file_exists(argv[i]));

            // Try and read the id
            if (has_valid_id(argv[i])) {
                strncpy(id, argv[i], 15);
                id[15] = '\0';
            } else {
                // Returns true if extraction of creation timestamp is
                // successful
                creation_timestamp_okay = file_creation_timestamp(argv[i], id);
                if (!creation_timestamp_okay) return EXIT_FAILURE;
            }

            assert(0 && "Not implemented");

            // Try and read the signature
            if (!signature_set) {

            }
            // Try and read the title
            if (!title_set) {

            }
            // Try and read the keywords
            if (!keywords_set) {

            }

            // Write the new filename to new_file_name
            char new_file_name[MAX_NAME_LEN];
            make_filename(new_file_name, id, sig, title, keywords);
        }
    }

    return EXIT_SUCCESS;
}
