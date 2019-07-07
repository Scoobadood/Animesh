#pragma once

#include <iostream>

typedef struct Arguments {
  enum Source {
    FILE,
    COMPUTE
  };

  // Directory to load files from or the state file.
  std::string file_or_directory;

  // If true, load correspondences from file
  bool load_correspondences_from_file;
  std::string correspondence_file_name;


  // Load from file.
  Source source;
} MesherArguments;

/**
 * Read arguments into args. Exit if they are invalid.
 */
void
parse_args(int argc, char *argv[], MesherArguments& args);
