cmake_minimum_required(VERSION 3.16)

project(gcs_tests LANGUAGES CXX)
enable_testing()

add_executable(gcs_tests
    test_udp_mavlink.cpp
)

target_link_libraries(gcs_tests PRIVATE
    Qt6::Core
    Qt6::Test
    Qt6::Network
    Qt6::Positioning        # <-- add this
)

# Include app sources used by tests
target_sources(gcs_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src/mav/MavlinkCodec.cpp
    ${CMAKE_SOURCE_DIR}/src/vehicle/MultiVehicleManager.cpp
    ${CMAKE_SOURCE_DIR}/src/vehicle/Vehicle.cpp
    ${CMAKE_SOURCE_DIR}/src/link/Link.cpp
)

target_include_directories(gcs_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${mavlink_SOURCE_DIR}
    ${mavlink_SOURCE_DIR}/common
    ${mavlink_SOURCE_DIR}/all
)

add_test(NAME gcs_tests COMMAND gcs_tests)
