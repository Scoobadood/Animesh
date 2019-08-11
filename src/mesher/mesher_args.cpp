#include "mesher_args.h"

/**
 * Print usage instructions and exit
 */
void
usage(const char *name) {
    std::cout << "Usage : " << name << " [-s surfel_file | -d source_directory [-c correspondence_file] ] "
              << std::endl;
    exit(-1);
}


/**
 * @return true if the given variable is a valid flag (starts with '-' and
 * if the provdied character.
 */
bool
is_valid_flag(const char *string, const char flag) {
    if (string == nullptr) return false;
    if (string[0] != '-') return false;
    if (strlen(string) != 2) return false;
    return (string[1] == flag);
}

/**
 * Parse the command line arguments into a Meherarguments structure.
 * Possible options are:
 * prog -s surfel file               -- Load all date
 * prog -d directory                 -- Compute all data including correspondences
 * prog -d directory -c corr_file    -- Compute surfels but load correspondencs
 * @param argc
 * @param argv
 * @param args
 */
void
parse_args(int argc, char *argv[], MesherArguments &args) {
    // Load or compute,but no correspondence file.
    if (argc == 3) {
        args.load_correspondences_from_file = false;
        args.file_or_directory = argv[2];
        if (is_valid_flag(argv[1], 's')) {
            args.source = MesherArguments::FILE;
            return;
        } else if (is_valid_flag(argv[1], 'd')) {
            args.source = MesherArguments::COMPUTE;
            return;
        } else {
            usage(argv[0]);
        }
    }

    // Should be compute with correspondence file
    else if (argc == 5) {
        args.load_correspondences_from_file = true;
        args.source = MesherArguments::COMPUTE;
        if (is_valid_flag(argv[1], 'd') && is_valid_flag(argv[3], 'c')) {
            args.file_or_directory = argv[2];
            args.correspondence_file_name = argv[4];
        } else if (is_valid_flag(argv[1], 'c') && is_valid_flag(argv[3], 'd')) {
            args.file_or_directory = argv[4];
            args.correspondence_file_name = argv[2];
        } else {
            usage(argv[0]);
        }
    } else {
        usage(argv[0]);
    }

    // TODO: Read these from file or args too.
    args.ts = 8.0f;
    args.tl = 3.0f;
}
