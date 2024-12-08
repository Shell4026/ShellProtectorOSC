cmake_minimum_required (VERSION 3.8)

project ("ShellProtectorOSC" LANGUAGES CXX)
file(GLOB ShellSRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable (ShellProtectorOSC WIN32 ${ShellSRC})
target_precompile_headers(ShellProtectorOSC PUBLIC "${CMAKE_SOURCE_DIR}/include/PCH.h")
target_include_directories(ShellProtectorOSC PRIVATE "${CMAKE_SOURCE_DIR}/include")
message(STATUS "Include directories: ${CMAKE_SOURCE_DIR}/include")
message(STATUS "SRC: ${ShellSRC}")

include(FetchContent)
FetchContent_Declare(
  sfml
  GIT_REPOSITORY "https://github.com/SFML/SFML"
  GIT_TAG "2.6.2"
)
FetchContent_MakeAvailable(sfml)
target_link_libraries(ShellProtectorOSC PUBLIC sfml-system sfml-graphics sfml-window sfml-network)

FetchContent_Declare(
  traypp
  GIT_REPOSITORY "https://github.com/Soundux/traypp"
)
FetchContent_MakeAvailable(traypp)
target_link_libraries(ShellProtectorOSC PUBLIC tray)

add_subdirectory(${CMAKE_SOURCE_DIR}/external/imgui)
target_link_libraries(ShellProtectorOSC PUBLIC ImGUI)