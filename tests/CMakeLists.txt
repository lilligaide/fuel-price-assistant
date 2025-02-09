enable_testing()

# Find GTest package
find_package(GTest REQUIRED)

# Add test executable
add_executable(unit_tests
    StationServiceTest.cpp
    DatabaseTest.cpp
    GeocodingTest.cpp
)

# Link test dependencies
target_link_libraries(unit_tests
    PRIVATE
    GTest::GTest
    GTest::Main
    ${PROJECT_NAME}_lib  # Link to main project library
)

# Add tests to CTest
add_test(NAME unit_tests COMMAND unit_tests) 