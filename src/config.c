#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "utils.h"

// Function to remove surrounding quotes from a string if present
char *remove_quotes(char *str) {
  size_t len = strlen(str);
  if (len > 1 && str[0] == '"' && str[len - 1] == '"') {
    str[len - 1] = '\0'; // Remove closing quote
    str++;               // Move past opening quote
  }
  return str;
}

// Function to parse config file and set connote_path
int parse_connote_config(const char *filename, char *connote_path, size_t connote_path_size) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "ERROR: Error opening connote config file.\n");
    return FAILURE;
  }

  char line[MAX_CONFIG_LINE_LENGTH];
  while (fgets(line, MAX_CONFIG_LINE_LENGTH, file)) {
    char *key = strtok(line, "=");
    char *value = strtok(NULL, "=");

    if (key && value) {
      trim_string(key);
      trim_string(value);
      value = remove_quotes(value); // Remove quotes from value

      if (strcmp(key, "connote_path") == 0) {
        strncpy(connote_path, value, connote_path_size); // Copy string value
      }
    } else {
      fprintf(stderr, "ERROR: Error reading connote config file.\n");
      return FAILURE;
    }
  }

  fclose(file);

  return connote_path != NULL && connote_path[0] != '\0' ? SUCCESS : FAILURE;
}

int connote_dir(char *connote_path) {

  // Expand the home directory path
  const char *home = getenv("HOME");
  char config_path[MAX_PATH_LEN] = {0};
  snprintf(config_path, MAX_PATH_LEN, "%s/.connote", home);

  int read_config_outcome = parse_connote_config(config_path, connote_path, MAX_PATH_LEN);
  if (read_config_outcome != SUCCESS)
    return FAILURE;

  printf("Config file successfully parsed:\n  connote_dir = %s\n", connote_path);

  // Make the config directory if it doesn't already exist
  int mkdir_outcome = make_directory_if_not_exists((const char *)connote_path);
  if (mkdir_outcome != SUCCESS)
    return FAILURE;

  return SUCCESS;
}
