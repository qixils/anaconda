cmake_minimum_required (VERSION 2.6)

project(Chowdren)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include_directories("${PROJECT_SOURCE_DIR}/include")
set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include" ${CMAKE_INCLUDE_PATH})

set(SRCS run.cpp images.cpp image.cpp glew.c
    include/tinythread/tinythread.cpp)

if(WIN32)
    set(CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}/lib/win32"
        ${CMAKE_LIBRARY_PATH})
    set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/win32"
        ${CMAKE_INCLUDE_PATH})
    set(SRCS ${SRCS} resource.rc)
    set(PLATFORM_LIBRARIES winmm)
elseif(APPLE)
    set(CMAKE_LIBRARY_PATH "${PROJECT_SOURCE_DIR}/lib/osx"
        ${CMAKE_LIBRARY_PATH})
    set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include/osx"
        ${CMAKE_INCLUDE_PATH})
    find_library(CARBON_LIBRARY Carbon)
    set(PLATFORM_LIBRARIES ${CARBON_LIBRARY})
endif()

if(MSVC)
    # /bigobj is required for MSVC because images.cpp can get too big on
    # some applications
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    add_definitions("/wd4244 /wd4102 /wd4305")
endif()

add_definitions(-DGLEW_STATIC)

if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set(MACOSX_BUNDLE_BUNDLE_NAME "%(name)s")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "%(name)s")
    set(MACOSX_BUNDLE_INFO_STRING "%(name)s - %(version)s")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "%(version)s")
    set(MACOSX_BUNDLE_COPYRIGHT "%(copyright)s")
    # This tells cmake where to place files inside the bundle
    set(OSX_ICON_FILES "${CMAKE_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
    # set where in the bundle to put the icns files
    set_source_files_properties(${OSX_ICON_FILES} PROPERTIES 
        MACOSX_PACKAGE_LOCATION Resources)
    set(SRCS ${SRCS} ${OSX_ICON_FILES})
endif()

if(UNIX)
    # for all binaries created in a CMake project
    SET(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/")
    # for certain targets
    SET_TARGET_PROPERTIES(Chowdren 
        PROPERTIES INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/")
endif()

add_executable(Chowdren WIN32 MACOSX_BUNDLE ${SRCS})

find_package(GLFW REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenALSoft REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)
find_package(PNG REQUIRED)

add_definitions(${PNG_DEFINITIONS})

if(NOT PYTHON_DEBUG_LIBRARIES)
    add_definitions(-DFORCE_PYTHON_RELEASE)
endif()

include_directories(${OPENALSOFT_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS} ${SNDFILE_INCLUDE_DIRS} ${GLFW_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR} ${PNG_INCLUDE_DIRS})
target_link_libraries(Chowdren ${GLFW_LIBRARY} ${OPENGL_LIBRARY} 
    ${FREETYPE_LIBRARIES} ${SNDFILE_LIBRARIES} ${OPENALSOFT_LIBRARY} 
    ${PYTHON_LIBRARIES} ${VORBIS_LIBRARIES} ${PLATFORM_LIBRARIES}
    ${PNG_LIBRARIES})

include(CPack)