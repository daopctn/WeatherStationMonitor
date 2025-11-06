# WeatherStationMonitor - Test Suite Configuration
# This CMakeLists.txt configures the Qt Test framework for unit and integration tests

# Find Qt Test module for testing infrastructure
find_package(Qt5 REQUIRED COMPONENTS Test Core Network Sql)

# Include directories from main project
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${Python3_INCLUDE_DIRS}
)

# Helper function to add weather station tests
# Usage: add_weather_test(test_name source_file.cpp [additional_sources...])
function(add_weather_test TEST_NAME)
    # Create test executable
    add_executable(${TEST_NAME} ${ARGN})

    # Link against Qt Test and other required libraries
    target_link_libraries(${TEST_NAME}
        Qt5::Test
        Qt5::Core
        Qt5::Network
        Qt5::Sql
        Python3::Python
    )

    # Add test to CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    # Set test properties
    set_tests_properties(${TEST_NAME} PROPERTIES
        TIMEOUT 30  # 30 second timeout per test
    )
endfunction()

# Individual test suites
# Note: Uncomment tests as dependencies become available in build environment

add_weather_test(test_WeatherData test_WeatherData.cpp)
# add_weather_test(test_DatabaseManager test_DatabaseManager.cpp)  # Requires MySQL
# add_weather_test(test_PythonBridge test_PythonBridge.cpp)       # TODO: Implement
# add_weather_test(test_WeatherWorker test_WeatherWorker.cpp)     # TODO: Implement
# add_weather_test(test_integration test_integration.cpp)         # TODO: Implement

message(STATUS "Test suite configured. Run 'ctest' to execute tests.")
message(STATUS "  - test_WeatherData: ENABLED")
message(STATUS "  - test_DatabaseManager: Available (requires MySQL)")
message(STATUS "  - Other tests: TODO (Phase 2 continuation)")
