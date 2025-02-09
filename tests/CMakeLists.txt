enable_testing()

# Add test executable
add_executable(unit_tests
    TankerkoenigAPITest.cpp
    TeamsNotificationTest.cpp
    ConfigTest.cpp
)

# Link test dependencies
target_link_libraries(unit_tests
    PRIVATE
    Catch2::Catch2WithMain
    CURL::libcurl
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# Add tests to CTest
add_test(NAME unit_tests COMMAND unit_tests) 