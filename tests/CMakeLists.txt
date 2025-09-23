cmake_minimum_required(VERSION 3.16)

find_package(Qt6 REQUIRED COMPONENTS Test Network)

add_executable(udp_mavlink_test
    test_udp_mavlink.cpp
    ../src/link/UdpLink.cpp
    ../src/mav/MavlinkCodec.cpp
)

target_include_directories(udp_mavlink_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${mavlink_SOURCE_DIR}
    ${mavlink_SOURCE_DIR}/all
    ${mavlink_SOURCE_DIR}/common
)

target_link_libraries(udp_mavlink_test PRIVATE Qt6::Test Qt6::Network)

add_test(NAME udp_mavlink COMMAND udp_mavlink_test)
