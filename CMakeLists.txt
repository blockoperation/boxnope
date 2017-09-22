project(boxnope)
cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -ffunction-sections -fdata-sections -Wl,--gc-sections -O2 -W -Wall -Wextra -pedantic")

set(boxnope_source src/main.cpp src/background.cpp src/menu.cpp)
set(boxnope_link)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets X11Extras XmlPatterns)
qt5_add_resources(boxnope_resources resources/boxnope.qrc)
list(APPEND boxnope_source ${boxnope_resources})
list(APPEND boxnope_link Qt5::Core Qt5::Widgets Qt5::X11Extras Qt5::XmlPatterns)

option(WITH_KGLOBALACCEL "Enable keyboard shortcuts using KGlobalAccel" ON)
if(WITH_KGLOBALACCEL)
    find_package(KF5GlobalAccel REQUIRED)
    add_definitions("-DWITH_KGLOBALACCEL")
    list(APPEND boxnope_link KF5::GlobalAccel)
endif(WITH_KGLOBALACCEL)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PKG_XCB REQUIRED xcb)
find_path(XCB_INCLUDE_DIR xcb/xcb.h ${PKG_XCB_INCLUDE_DIRS})
find_library(XCB_LIBRARY NAMES xcb PATHS ${PKG_XCB_LIBRARY_DIRS})
list(APPEND boxnope_link ${XCB_LIBRARY})

add_executable(boxnope ${boxnope_source})
target_link_libraries(boxnope ${boxnope_link})

include(GNUInstallDirs)
install(TARGETS boxnope DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT prog)
