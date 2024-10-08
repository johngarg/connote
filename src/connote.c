#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "utils.h"

// connote <cmd> --title <title> --keywords <kw1> <kw2> --sig <sig>

void output_dir(bool use_connote_dir, char *dir_path) {
  if (use_connote_dir) {
    connote_dir(dir_path);
  } else {
    snprintf(dir_path, MAX_PATH_LEN, "./");
  }
}

void test_argument_parsing(char **argv, int argc, char *sig, char *title, int kw_count, char **keywords) {
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

void print_usage() {
  printf("Usage: connote file --title <title> --keywords <kw1> <kw2> <kw3> "
         "--sig <signature>\n");
}

int main(int argc, char *argv[]) {
  // Expect at least three args: `connote <cmd> <arg>`
  if (argc < 2) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  // Initialise the filename data
  char *title = NULL;
  char *sig = NULL;
  char *keywords[MAX_KEYS]; // Array to store keywords, assuming max 10
  int kw_count = 0;

  // Define long options
  static struct option long_options[] = {
      {    "title", required_argument, 0, 't'},
      { "keywords", required_argument, 0, 'k'},
      {      "sig", required_argument, 0, 's'},
      {"from-yaml",       no_argument, 0, 'y'},
      {      "dir",       no_argument, 0, 'd'},
      {          0,                 0, 0,   0}  // End of options
  };

  // Parsing options
  int opt;
  bool signature_set = false;
  bool title_set = false;
  bool keywords_set = false;
  bool use_connote_dir = false;

  while ((opt = getopt_long(argc, argv, "t:k:s:yd", long_options, NULL)) != -1) {
    switch (opt) {
    case 't':
      title = optarg; // Get title argument
      title_set = true;
      break;
    case 'k':
      // Get keywords, assuming they are separated by spaces and provided as
      // multiple arguments
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
      sig = optarg; // Get signature argument
      signature_set = true;
      break;
    case 'y':
      // This is reached when --from-yaml is encountered
      printf("From YAML flag is set.\n");
      break;
    case 'd':
      // This means write the file to the connote directory set in the config
      // file
      printf("'Use connote directory' is set.\n");
      use_connote_dir = true;
      break;
    default:
      exit(EXIT_FAILURE);
    }
  }

  // Output the parsed arguments for testing purposes
  test_argument_parsing(argv, argc, sig, title, kw_count, keywords);

  // Where is the file going?
  char dir_path[MAX_PATH_LEN];

  // Allocate MAX_PATH_LEN for new file name
  char new_file_name[MAX_PATH_LEN];
  // Allocate a buffer of 15 chars + the null terminator for the ID
  char id[ID_LEN + 1];

  // Count the non-option arguments
  int non_option_args = 0;
  char *cmd = argv[optind];
  for (int index = optind; index < argc; index++) {
    non_option_args++;
  }

  // connote new
  if (strcmp(cmd, "new") == 0) {
    // Here we are writing a new file

    // Make a new ID based on the current time
    if (generate_timestamp_now(id) != SUCCESS)
      return EXIT_FAILURE;

    // Get the directory in which the note will be written, save this to
    // `dir_path`
    output_dir(use_connote_dir, dir_path);

    // Create new file with components and write frontmatter
    connote_file(dir_path, id, sig, title, keywords, kw_count, ".md", new_file_name);
    // Print the created file for the user
    printf("%s\n", new_file_name);

    return EXIT_SUCCESS;
  }

  // connote rename
  if (strcmp(cmd, "rename") == 0) {
    if (non_option_args < 2)
      return EXIT_FAILURE;

    optind++; // Increment past the <cmd> argument
    // Loop over input files and rename them
    for (int i = optind; i < argc; i++) {

      printf("argv[%d]: %s\n", i, argv[i]);

      // Check whether the file exists
      if (!file_exists(argv[i])) {
        fprintf(stderr, "ERROR: File does not exist: %s\n", argv[i]);
        return EXIT_FAILURE;
      }

      // Attempt to read the creation date of the file for use as the file ID
      if (has_valid_id(argv[i])) {
        read_id(argv[i], id);
      } else {
        // Check whether extraction of creation timestamp is successful
        if (file_creation_timestamp(argv[i], id) != SUCCESS) {
          fprintf(stderr, "ERROR: Could not retrieve timestamp from file %s.\n", argv[i]);
          return EXIT_FAILURE;
        }
      }

      char filename_sig[MAX_SIG_LEN] = {0};
      if (!signature_set) {
        try_match_and_write_component(argv[i], filename_sig, SIG_REGEX, MAX_SIG_LEN);
      }

      char filename_title[MAX_TITLE_LEN] = {0};
      if (!title_set) {
        try_match_and_write_component(argv[i], filename_title, TITLE_REGEX, MAX_TITLE_LEN);
      }

      if (!keywords_set) {
        char matched_keywords[MAX_KEYS * MAX_KW_LEN] = {0};
        int outcome = try_match_and_write_component(argv[i], matched_keywords, KW_REGEX, MAX_KEYS * MAX_KW_LEN);
        if (outcome == SUCCESS) {
          char keywords_array[MAX_KW_LEN][MAX_KEYS] = {0};
          // Create an array of pointers to the rows of keywords
          for (int i = 0; i < MAX_KEYS; i++) {
            // Just overwrite the existing `keywords` array here
            keywords[i] = keywords_array[i];
          }
          kw_count = split_at_char(matched_keywords, '_', keywords, MAX_KEYS, MAX_KW_LEN);
        }
      }

      // Construct new file name
      int last_slash = last_slash_pos(argv[i]);
      if (last_slash != -1) {
        strncpy(dir_path, argv[i], last_slash+1);
        dir_path[last_slash+1] = '\0';
      } else {
        strncpy(dir_path, "./", 2);
        dir_path[2] = '\0';
      }

      printf("dir_path: %s\n", dir_path);
      format_file_name(dir_path, id, sig ? sig : filename_sig, title ? title : filename_title, keywords, kw_count,
                       ".md", new_file_name);

      // Rename the file
      if (rename(argv[i], new_file_name) == 0) {
        printf("%s -> %s\n", argv[i], new_file_name);
      } else {
        fprintf(stderr, "ERROR: Could not rename file %s\n", argv[i]);
      }

    }

    return EXIT_SUCCESS;
  }

  if (strcmp(cmd, "backlinks") == 0) {
    assert(false && "Not implemented yet");
    return EXIT_SUCCESS;
  }

  if (strcmp(cmd, "search") == 0) {
    assert(false && "Not implemented yet");
    return EXIT_SUCCESS;
  }

  if (strcmp(cmd, "doctor") == 0) {
    assert(false && "Not implemented yet");
    return EXIT_SUCCESS;
  }

  if (strcmp(cmd, "journal") == 0) {
    assert(false && "Not implemented yet");
    return EXIT_SUCCESS;
  }

  // No command matched
  return EXIT_FAILURE;
}
