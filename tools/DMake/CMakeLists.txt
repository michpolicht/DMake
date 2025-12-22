cmake_minimum_required(VERSION 3.16)

project(DMake LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

include_directories(SYSTEM ${CMake_SOURCE_DIR}/Utilities)
include_directories(SYSTEM ${CMake_SOURCE_DIR}/Utilities/std)
include_directories(SYSTEM "${CMake_BINARY_DIR}/Utilities")

add_executable(dmake
  main.cpp
)

target_link_libraries(dmake
  Qt${QT_VERSION_MAJOR}::Core

  CMakeLib
)

include(GNUInstallDirs)
install(TARGETS dmake
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
