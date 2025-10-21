cmake_minimum_required(VERSION 3.13)

project(RailcomTests)

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
    railcom_tests
    test_dcc_message.cpp
    test_railcom.cpp
    ../src/Railcom.cpp
)

target_include_directories(
    railcom_tests
    PRIVATE
    ../src
)

target_link_libraries(
    railcom_tests
    PRIVATE
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(railcom_tests)
