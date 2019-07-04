#pragma once

typedef struct Arguments {
  enum LoadSource {
    FILE,
    DIRECTORY
  };

  // Directory to load files from or the state file.
  std::string file_or_directory;

  // Load from file.
  LoadSource load_source;
} Arguments;

/**
 * Read arguments into args. Exit if they are invalid.
 */
void
parse_args(int argc, char *argv[], Arguments& args);
