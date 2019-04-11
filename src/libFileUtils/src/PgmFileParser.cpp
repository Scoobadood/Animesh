/**
 * Loads Frames from data
 */

#include <fstream>
#include <string>
#include <FileUtils/PgmFileParser.h>

/**
 * Read the provided file and return a PgmData object.
 * File should be a type P2 at this point.
 */
PgmData read_pgm( const std::string& file_name ) {
	using namespace std;

	// File format is 
	// P2      <-- Version
	// 640 480 <-- Dimesnions
	// 553 ... <-- Max value
	// 0 0 0 0 ... Data
	enum ParseState {
		EXPECTING_VERSION,
		EXPECTING_WIDTH,
		EXPECTING_HEIGHT,
		EXPECTING_MAX_VAL,
		EXPECTING_DATA,
		DONE
	};
	ParseState state = EXPECTING_VERSION;
	int width;
	int height;
	int max_val;
	int expected_data;
	int pixel_data;
	vector<int> data;
	process_file_by_lines( file_name, [&](const string& text_line){
		using namespace std;

		vector<string> tokens = tokenise(text_line);
		for( auto token : tokens ) {
    		switch( state ) {
				case EXPECTING_VERSION:
					if( token == "P2" ) {
						state = EXPECTING_WIDTH;
					} else {
						throw std::runtime_error("read_pgm cannot handle files of type other than P2");
					}
	    			break;
	    		case EXPECTING_WIDTH:
	    			width = stoi( token );
					state = EXPECTING_HEIGHT;
					break;
	    		case EXPECTING_HEIGHT:
	    			height = stoi( token );
	    			expected_data = width * height;
					state = EXPECTING_MAX_VAL;
					break;
	    		case EXPECTING_MAX_VAL:
	    			max_val = stoi( token );
					state = EXPECTING_DATA;
					break;
	    		case EXPECTING_DATA:
	    			pixel_data = stoi( token );
	    			data.push_back(pixel_data);
	    			expected_data--;
	    			if( expected_data == 0 ) {
	    				state = DONE;
	    			}
					break;
	    		default:
	    			break;
			}
		}
	});

	return PgmData{ (size_t)width, (size_t)height, data};
}