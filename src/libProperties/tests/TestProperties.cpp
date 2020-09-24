#include "TestProperties.h"
#include <Properties/Properties.h>

void TestProperties::SetUp( ) {}
void TestProperties::TearDown( ) {}

#define EXPECT_THROW_WITH_MESSAGE(stmt, etype, whatstring) EXPECT_THROW( \
        try { \
            stmt; \
        } catch (const etype& ex) { \
            EXPECT_EQ(std::string(ex.what()), whatstring); \
            throw; \
        } \
    , etype)


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

    EXPECT_THROW_WITH_MESSAGE(
            p.getProperty("Missing"),
            std::out_of_range,
            "map::at:  key not found"
    );
}

TEST_F( TestProperties, FileWithCommentsShouldParse) {
    Properties p{"values_with_comments.properties"};
    int expected = 20.0f;
    int actual = p.getFloatProperty("float_p2");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, BooleanPropertyWithYesShouldBeTrue) {
    Properties p{"values_with_comments.properties"};
    bool expected = true;
    bool actual = p.getBooleanProperty("yes-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("YES-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("Yes-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("y-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("Y-property");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, BooleanPropertyWithTrueShouldBeTrue) {
    Properties p{"values_with_comments.properties"};
    bool expected = true;
    bool actual = p.getBooleanProperty("true-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("TRUE-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("True-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("t-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("T-property");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, BooleanPropertyWithNoShouldBeFalse) {
    Properties p{"values_with_comments.properties"};
    bool expected = false;
    bool actual = p.getBooleanProperty("no-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("NO-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("No-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("n-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("N-property");
    EXPECT_EQ(expected, actual);
}

TEST_F( TestProperties, BooleanPropertyWithFalseShouldBeFalse) {
    Properties p{"values_with_comments.properties"};
    bool expected = false;
    bool actual = p.getBooleanProperty("false-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("FALSE-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("False-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("f-property");
    EXPECT_EQ(expected, actual);

    actual = p.getBooleanProperty("F-property");
    EXPECT_EQ(expected, actual);
}


TEST_F( TestProperties, InvalidBooleanPropertyShouldThrow) {
    Properties p{"values_with_comments.properties"};
    try {
        p.getBooleanProperty("invalid-boolean-property");
        FAIL() << "Expected std::runtime_error";
    } catch( std::runtime_error const & err ) {
        EXPECT_EQ( err.what(), std::string( "Unrecognised boolean value [aye] in properties file") );
    } catch ( ... ) {
        FAIL( ) << "Expected std::runtime_error";
    }
}

TEST_F( TestProperties, InitialiseWithMapShouldWork) {
    std::map<std::string, std::string> props = {
            {"rho", "1.0"},
            {"text", "a string"},
            {"flag", "true"}
    };
    Properties p{props};
    EXPECT_EQ( p.getFloatProperty("rho"),1.0);
    EXPECT_EQ( p.getProperty("text"),"a string");
    EXPECT_EQ( p.getBooleanProperty("flag"),true);
}
