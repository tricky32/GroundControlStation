cmake_minimum_required(VERSION 3.16)

project(gcs_application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Network Positioning Location)
qt_policy(SET QTP0004 NEW)
qt_standard_project_setup(REQUIRES 6.5)

# ---- Executable shell
qt_add_executable(gcs_application
    src/main.cpp
)

# ---- QML module (NO qml.qrc)
set(qml_files
    src/ui/Main.qml
    src/ui/MainView.qml
    src/ui/MapView.qml
    src/ui/TopBar.qml
    src/ui/CommandBar.qml
    src/ui/vehicleMarker.qml
)

# ---- C++ sources from the new architecture
set(cpp_sources
    src/link/UdpLink.cpp        src/link/UdpLink.h
    src/link/UdpWorker.cpp      src/link/UdpWorker.h
    src/mav/MavRouter.cpp       src/mav/MavRouter.h
    src/mav/MavCommandSender.cpp src/mav/MavCommandSender.h
    src/vehicle/Vehicle.cpp     src/vehicle/Vehicle.h
    src/vehicle/MultiVehicleManager.cpp src/vehicle/MultiVehicleManager.h
)

qt_add_qml_module(gcs_application
    URI gcs_application
    VERSION 1.0
    QML_FILES
        ${qml_files}
    SOURCES
        ${cpp_sources}
)

target_include_directories(gcs_application PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- MAVLink headers only (pick ONE option)

# Option A: FetchContent (recommended)
include(FetchContent)
FetchContent_Declare(
    mavlink
    GIT_REPOSITORY https://github.com/mavlink/c_library_v2.git
    GIT_TAG master
)
FetchContent_MakeAvailable(mavlink)

add_library(MavlinkHeaders INTERFACE)
target_include_directories(MavlinkHeaders INTERFACE ${mavlink_SOURCE_DIR})



target_link_libraries(gcs_application
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Qml
        Qt6::Network
        Qt6::Positioning
        Qt6::Location
        MavlinkHeaders
)

# Install (optional)
include(GNUInstallDirs)
install(TARGETS gcs_application
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Tests (optional)
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
