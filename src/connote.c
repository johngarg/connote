#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "config.h"

// connote <cmd> --title <title> --keywords <kw1> <kw2> --sig <sig>

void output_dir(bool use_connote_dir, char *dir_path) {
  if (use_connote_dir) {
    connote_dir(dir_path);
  } else {
    snprintf(dir_path, sizeof(dir_path), "./");
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
  if (argc < 3) {
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
      use_connote_dir = true;
      break;
    default:
      exit(EXIT_FAILURE);
    }
  }

  // Output the parsed arguments for testing purposes
  test_argument_parsing(argv, argc, sig, title, kw_count, keywords);

  // Where is the file going?
  char dir_path[MAX_NAME_LEN];
  // Get the directory in which the note will be written, save this to
  // `dir_path`
  output_dir(use_connote_dir, dir_path);
  printf("Output dir: %s\n", dir_path);

  // Allocate MAX_NAME_LEN for new file name
  char new_file_name[MAX_NAME_LEN];
  // Allocate a buffer of 15 chars + the null terminator for the ID
  char id[ID_LEN + 1];

  bool creation_timestamp_okay;

  // Count the non-option arguments
  int non_option_args = 0;
  char *cmd = argv[optind];
  for (int index = optind; index < argc; index++) {
    non_option_args++;
  }

  // connote file command
  if (non_option_args > 1 && strcmp(cmd, "file") == 0) {
    // If argc > 1 and the first argument is "file", we are trying to rename
    // an existing file

    optind++; // Increment past the <cmd> argument
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
        if (!creation_timestamp_okay)
          return EXIT_FAILURE;
      }

      assert(0 && "Not implemented");

      /* remove_unwanted_chars(title); */
      /* printf("Cleaned: %s\n", title); */

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
      /* make_filename(new_file_name, id, sig, title, keywords); */
    }
  } else if (strcmp(cmd, "file") == 0) {
    // Here we are writing a new file
    printf("Making new file...\n");

    // Make a new ID based on the current time
    bool timestamp_generation_okay = generate_timestamp_now(id);
    if (!timestamp_generation_okay)
      return EXIT_FAILURE;

    printf("Here...\n");
    connote_file(dir_path, id, sig, title, keywords, kw_count, ".md", new_file_name);
    printf("%s", new_file_name);
  }

  return EXIT_SUCCESS;
}
