cmake_minimum_required (VERSION 2.6)

project(Chowdren)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include_directories("${PROJECT_SOURCE_DIR}/include")
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${PROJECT_SOURCE_DIR}/include")

set(SRCS run.cpp images.cpp image.cpp glew.c
    include/tinythread/tinythread.cpp assets.cpp)

if(WIN32)
    set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} 
        "${PROJECT_SOURCE_DIR}/lib/win32")
    set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} 
        "${PROJECT_SOURCE_DIR}/include/win32")
    set(SRCS ${SRCS} chowdren.rc)
    set(PLATFORM_LIBRARIES winmm)
endif(WIN32)

if(MSVC)
    # /bigobj is required for MSVC because images.cpp can get too big on
    # some applications
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    add_definitions("/wd4244 /wd4102 /wd4305")
endif()

add_definitions(-DGLEW_STATIC)
add_executable(Chowdren ${SRCS})
find_package(GLFW REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(PNG REQUIRED)

add_definitions(${PNG_DEFINITIONS})

if(NOT PYTHON_DEBUG_LIBRARIES)
    add_definitions(-DFORCE_PYTHON_RELEASE)
endif()

include_directories(${OPENAL_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS} ${SNDFILE_INCLUDE_DIRS} ${GLFW_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
target_link_libraries(Chowdren ${GLFW_LIBRARY} ${OPENGL_LIBRARY} 
    ${FREETYPE_LIBRARIES} ${SNDFILE_LIBRARIES} ${OPENAL_LIBRARY} 
    ${PYTHON_LIBRARIES} ${VORBIS_LIBRARIES} ${PLATFORM_LIBRARIES}
    ${PNG_LIBRARIES})