#include "TestProperties.h"
#include <Properties/Properties.h>

void TestProperties::SetUp( ) {}
void TestProperties::TearDown( ) {}


/* ********************************************************************************
 * *
 * *  Test average rosy vectors
 * *   
 * ********************************************************************************/
TEST_F( TestProperties, EmptyFileShouldLoad) {
    Properties p{"empty.properties"};
}

TEST_F( TestProperties, IntegerValueShouldReadWithSpaces) {
    Properties p{"values.properties"};

    int expected = 1;
    int actual = p.getIntProperty("int_p");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, IntegerValueShouldReadWithNoSpaces) {
    Properties p{"values.properties"};
    int expected = 10;
    int actual = p.getIntProperty("int_p2");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, FloatValueShouldReadWithSpaces) {
    Properties p{"values.properties"};

    int expected = 2.0f;
    int actual = p.getFloatProperty("float_p");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, FloatValueShouldReadWithNoSpaces) {
    Properties p{"values.properties"};
    int expected = 20.0f;
    int actual = p.getFloatProperty("float_p2");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, MissingValueShouldThrow) {
    Properties p{"values.properties"};
    try {
        p.getProperty("Missing");
        FAIL() << "Expected std:: out_of_range";
    } catch( std::out_of_range const & err ) {
        EXPECT_EQ( err.what(), std::string( "map::at:  key not found") );
    } catch ( ... ) {
        FAIL( ) << "Expected std::out_of_range";
    }
}

TEST_F( TestProperties, FileWithCommentsShouldParse) {
    Properties p{"values_with_comments.properties"};
    int expected = 20.0f;
    int actual = p.getFloatProperty("float_p2");
    EXPECT_EQ(expected, actual);
}
