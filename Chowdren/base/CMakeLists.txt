cmake_minimum_required (VERSION 2.6)
project (Chowdren)

add_definitions(-DGLEW_STATIC)
add_executable(Chowdren run.cpp images.cpp image.cpp glew.c 
               include/tinythread/tinythread.cpp assets.cpp)
find_library(GLFW_LIBRARY GLFW lib)
find_library(FT_LIBRARY freetype lib)
find_library(SNDFILE_LIBRARY libsndfile-1 lib)
find_library(PYTHON_DEBUG_LIB python27_d lib)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)

if(MSVC)
    # /bigobj is required for Windows because images.h can get too big on
    # some applications
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    add_definitions("/wd4244 /wd4102 /wd4305")
    set(PLATFORM_LIBRARIES winmm)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include ${OPENAL_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS})
target_link_libraries(Chowdren ${GLFW_LIBRARY} ${OPENGL_LIBRARY} 
    ${FT_LIBRARY} ${SNDFILE_LIBRARY} ${OPENAL_LIBRARY} ${PYTHON_LIBRARIES}
    ${PLATFORM_LIBRARIES})