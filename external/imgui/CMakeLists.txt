file(GLOB SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(ImGUI STATIC ${SRC})

find_package(OpenGL REQUIRED)
target_link_libraries(ImGUI PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(ImGUI PUBLIC sfml-system sfml-graphics sfml-window)

target_include_directories(ImGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})