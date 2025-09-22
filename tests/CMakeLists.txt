cmake_minimum_required(VERSION 3.16)
project(gcs_tests LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Test Network)

add_executable(gcs_tests
    LinkTests.cpp
)

target_link_libraries(gcs_tests
    PRIVATE Qt6::Core Qt6::Test Qt6::Network
)

enable_testing()
add_test(NAME gcs_tests COMMAND gcs_tests)
