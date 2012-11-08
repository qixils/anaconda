cmake_minimum_required (VERSION 2.6)

project(Chowdren)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${PROJECT_SOURCE_DIR}/lib")
include_directories("${PROJECT_SOURCE_DIR}/include")

if(MSVC)
    include_directories(${PROJECT_SOURCE_DIR}/include/win32)
    # /bigobj is required for Windows because images.h can get too big on
    # some applications
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    add_definitions("/wd4244 /wd4102 /wd4305")
    set(PLATFORM_LIBRARIES winmm)
endif()

add_definitions(-DGLEW_STATIC)
add_executable(Chowdren run.cpp images.cpp image.cpp glew.c 
               include/tinythread/tinythread.cpp assets.cpp)
find_library(PYTHON_DEBUG_LIB python27_d lib)
find_package(GLFW REQUIRED)
find_package(SndFile REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)

include_directories(${OPENAL_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS} ${SNDFILE_INCLUDE_DIRS} ${GLFW_INCLUDE_DIR})
target_link_libraries(Chowdren ${GLFW_LIBRARY} ${OPENGL_LIBRARY} 
    ${FREETYPE_LIBRARIES} ${SNDFILE_LIBRARIES} ${OPENAL_LIBRARY} 
    ${PYTHON_LIBRARIES} ${PLATFORM_LIBRARIES})