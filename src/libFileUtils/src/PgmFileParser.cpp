/**
 * Loads Frames from data
 */

#include <fstream>
#include <sstream>
#include <string>
#include <FileUtils/PgmFileParser.h>

std::vector<std::string> tokenise(const std::string& line){
	using namespace std;

    // Vector of string to save tokens 
    vector <string> tokens; 
      
    // stringstream class check1 
    stringstream check1(line); 
    string intermediate; 
      
    // Tokenizing w.r.t. space ' ' 
    while(getline(check1, intermediate, ' ')) { 
        tokens.push_back(intermediate); 
    } 
    return tokens;
}


Frame load_frame_from_file( const std::string& file_name ) {
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
					state = EXPECTING_WIDTH;
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

	return Frame{ (size_t)width, (size_t)height, data};
}