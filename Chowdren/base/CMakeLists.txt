cmake_minimum_required (VERSION 2.6)
project (Chowdren)

add_executable(Chowdren run.cpp)
include_directories("${PROJECT_SOURCE_DIR}/include")
find_library(GLFW_LIBRARY GLFW lib)
find_library(SOIL_LIBRARY SOIL lib)
find_package(OpenGL)
target_link_libraries (Chowdren ${GLFW_LIBRARY} ${OPENGL_LIBRARY} 
    ${SOIL_LIBRARY})