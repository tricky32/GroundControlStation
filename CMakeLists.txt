cmake_minimum_required(VERSION 3.16)

project(gcs_application VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# + Svg here
find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml QuickControls2 Network Positioning Location Svg)

qt_policy(SET QTP0004 NEW)
qt_standard_project_setup(REQUIRES 6.5)

# ---- App target (.cpp here) ----
qt_add_executable(appgcs_application
    src/main.cpp
    src/link/UdpLink.cpp
    src/mav/MavlinkCodec.cpp
    src/vehicle/Vehicle.cpp
    src/vehicle/MultiVehicleManager.cpp
    src/ui/QmlGlobals.cpp
)

# ---- Show headers in Qt Creator ----
target_sources(appgcs_application PRIVATE
    src/link/UdpLink.h
    src/mav/MavlinkCodec.h
    src/vehicle/Vehicle.h
    src/vehicle/MultiVehicleManager.h
    src/ui/QmlGlobals.h
)

# ---- QML module (no .qrc needed) ----
set(qml_files
    src/ui/Main.qml
    src/ui/MainView.qml
    src/ui/MapView.qml
    src/ui/VehicleMarker.qml
)
qt_add_qml_module(appgcs_application
    URI gcs_application
    VERSION 1.0
    QML_FILES ${qml_files}
    # + include SVG icons in the module's resource namespace
    RESOURCES
        res/example.svg
        res/goto.svg
        res/home.svg
)

# ---- MAVLink header-only (c_library_v2) ----
include(FetchContent)
FetchContent_Declare(
    mavlink
    GIT_REPOSITORY https://github.com/mavlink/c_library_v2.git
    GIT_TAG master
)
FetchContent_MakeAvailable(mavlink)
target_include_directories(appgcs_application
    PUBLIC
        ${mavlink_SOURCE_DIR}
        ${mavlink_SOURCE_DIR}/all
        ${mavlink_SOURCE_DIR}/common
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(appgcs_application PRIVATE
    MAVLINK_NO_CONVENIENCE_FUNCTIONS
    MAVLINK_USE_MESSAGE_INFO
)

# + Link Svg so the SVG image plugin is deployed with the app
target_link_libraries(appgcs_application PRIVATE
    Qt6::Core Qt6::Quick Qt6::Qml Qt6::QuickControls2
    Qt6::Network Qt6::Positioning Qt6::Location
    Qt6::Svg
)

set_target_properties(appgcs_application PROPERTIES WIN32_EXECUTABLE TRUE)

# Tests OFF for now
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
