cmake_minimum_required(VERSION 3.14)

project(
    raycaster
    DESCRIPTION "A reimplementation of the wolfenstein3d renderer."
    VERSION 0.0.1
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

include_directories(include ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS})

add_executable(
    raycaster
    src/main.cpp
)

target_link_libraries(raycaster ${OPENGL_LIBRARIES} ${GLUT_LIBRARY})
