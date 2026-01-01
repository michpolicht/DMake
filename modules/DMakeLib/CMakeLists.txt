# CMakeLib pre-requisites
include_directories(SYSTEM ${CMake_SOURCE_DIR}/Utilities)
include_directories(SYSTEM ${CMake_SOURCE_DIR}/Utilities/std)
include_directories(SYSTEM "${CMake_BINARY_DIR}/Utilities")

# Tell CMake executable in the build tree where to find the source tree.
configure_file(
  CMakeSourceDir.txt.in
  "${CMAKE_BINARY_DIR}/out/CMakeFiles/CMakeSourceDir.txt"
  @ONLY
)

find_package(Qt6 6.8 COMPONENTS Qml REQUIRED)

qt_add_library(DMakeLib STATIC)
qt_add_qml_module(DMakeLib
  URI DMakeLib
  VERSION 1.0
  SOURCES
  cpp/DMakeLib/DMake.cpp
  cpp/DMakeLib/DMake.hpp
)

generate_export_header(DMakeLib EXPORT_MACRO_NAME DMAKE_LIB_API EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/cpp/DMakeLib/autogen/ApiMacro.hpp)

target_compile_definitions(DMakeLib PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(DMakeLib
  PUBLIC
  CMakeLib
  Qt6::Qml
)

target_include_directories(DMakeLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cpp/DMakeLib)
target_include_directories(DMakeLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/cpp)
