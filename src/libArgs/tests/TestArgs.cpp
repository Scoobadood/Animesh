#include "gtest/gtest.h"
#include <Args/Args.h>

namespace {
    /* **********************************************************************
     * *                                                                    *
     * * Args Parser Constructor tests                                      *
     * *                                                                    *
     * **********************************************************************/
	TEST(TestArgs, Constructor) { 
        int argc = 0;
        char * argv[]{ const_cast<char*>("-f"), const_cast<char*>("-i 20")};

    	try {
        	Args args( argc, argv );
        	FAIL() << "Expected std::invalid_argument";
    	}
	    catch ( std::invalid_argument const & err ){
	        EXPECT_EQ( err.what(), std::string( "Normal vector must be unit") );
	    }
	    catch( ... ) {
	        FAIL( ) <<"Expected std::invalid_argument";
	    }
    }
}