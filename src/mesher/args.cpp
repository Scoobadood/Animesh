#include "args.h"
/**
 * Print usage instructions and exit
 */
void 
usage( const char *name ) {
    std::cout << "Usage : " << name << " [-s surfel_file | -d source_directory] [-m]" << std::endl;
    exit(-1);
}

parse_args(int argc, char *argv[], Arguments& args);
  bool args_ok = false;
  if( argc == 3 ) {
    args.file_or_directory = argv[2];

    if( argv[1][0] == '-') {
      if( argv[1][1] == 's' || argv[1][1] == 'S' ) {
          args_ok = true;
          args.load_source = FILE;
      } else if( argv[1][1] == 'd' || argv[1][1] == 'D' ) {
          args_ok = true;
          args.load_source = DIRECTORY;
      }
    }
  }
  if( !args_ok ) {
    usage(argv[0]);
  }
}
