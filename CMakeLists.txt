cmake_minimum_required(VERSION 3.16)

project(DMake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden) # This should typically result in setting `-fvisibility=hidden` flag on POSIX.

list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macros"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
)

# Since CMake normally uses static libraries and Qt normally uses dynamic libraries this is required to avoid
# LNK2038: mismatch detected for 'RuntimeLibrary': value 'MT_StaticRelease' doesn't match value 'MD_DynamicRelease'.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

add_subdirectory(subtree/CMake)

include(GenerateExportHeader)

include(recurse_subproject_dirs)
set(DMAKE_TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
set(DMAKE_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/bin)

# qt_policy macro is defined in Core and QTP0001 is defined in Qml component.
find_package(Qt6 REQUIRED COMPONENTS Core Qml)
qt_policy(SET QTP0001 NEW)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/bin)

recurse_subproject_dirs(SUBDIRS ${DMAKE_TOOLS_DIR})
foreach(SUBDIR ${SUBDIRS})
    message(STATUS "Adding subproject: ${SUBDIR}")
    add_subdirectory(${SUBDIR})
endforeach()

recurse_subproject_dirs(SUBDIRS ${DMAKE_MODULES_DIR})
foreach(SUBDIR ${SUBDIRS})
    message(STATUS "Adding subproject: ${SUBDIR}")
    add_subdirectory(${SUBDIR})
endforeach()

