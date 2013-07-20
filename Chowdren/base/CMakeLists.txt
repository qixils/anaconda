cmake_minimum_required (VERSION 2.6)

project(Chowdren)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include_directories("${PROJECT_SOURCE_DIR}/include")
set(CMAKE_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/include" ${CMAKE_INCLUDE_PATH})

FILE(GLOB FRAMESRCS "frame*.cpp")

set(SRCS run.cpp image.cpp glew.c desktop/platform.cpp font.cpp 
    desktop/shaders.cpp ${FRAMESRCS} movement.cpp common.cpp media.cpp
    objects.cpp fonts.cpp include/tinythread/tinythread.cpp)

option(USE_BOX2D OFF)
option(USE_PYTHON ON)

add_definitions(-DCHOWDREN_IS_DESKTOP)

if (USE_PYTHON)
    add_definitions(-DCHOWDREN_USE_PYTHON)
    set(SRCS
        ${SRCS}
        pythonext.cpp
    )
endif()

if (USE_BOX2D)
    add_definitions(-DCHOWDREN_USE_BOX2D)
    set(BOX2D_DIR "${PROJECT_SOURCE_DIR}/include/Box2D")
    set(SRCS
        ${SRCS}
        ${BOX2D_DIR}/Collision/b2BroadPhase.cpp
        ${BOX2D_DIR}/Collision/b2CollideCircle.cpp
        ${BOX2D_DIR}/Collision/b2CollidePoly.cpp
        ${BOX2D_DIR}/Collision/b2Collision.cpp
        ${BOX2D_DIR}/Collision/b2Distance.cpp
        ${BOX2D_DIR}/Collision/b2PairManager.cpp
        ${BOX2D_DIR}/Collision/b2TimeOfImpact.cpp
        ${BOX2D_DIR}/Collision/Shapes/b2CircleShape.cpp
        ${BOX2D_DIR}/Collision/Shapes/b2EdgeShape.cpp
        ${BOX2D_DIR}/Collision/Shapes/b2PolygonShape.cpp
        ${BOX2D_DIR}/Collision/Shapes/b2Shape.cpp
        ${BOX2D_DIR}/Common/b2BlockAllocator.cpp
        ${BOX2D_DIR}/Common/b2Math.cpp
        ${BOX2D_DIR}/Common/b2Settings.cpp
        ${BOX2D_DIR}/Common/b2StackAllocator.cpp
        ${BOX2D_DIR}/Dynamics/b2Body.cpp
        # ${BOX2D_DIR}/Dynamics/b2Body2.cpp
        ${BOX2D_DIR}/Dynamics/b2ContactManager.cpp
        ${BOX2D_DIR}/Dynamics/b2Island.cpp
        ${BOX2D_DIR}/Dynamics/b2World.cpp
        ${BOX2D_DIR}/Dynamics/b2WorldCallbacks.cpp
        ${BOX2D_DIR}/Dynamics/Contacts/b2CircleContact.cpp
        ${BOX2D_DIR}/Dynamics/Contacts/b2Contact.cpp
        ${BOX2D_DIR}/Dynamics/Contacts/b2ContactSolver.cpp
        ${BOX2D_DIR}/Dynamics/Contacts/b2EdgeAndCircleContact.cpp
        ${BOX2D_DIR}/Dynamics/Contacts/b2PolyAndCircleContact.cpp
        ${BOX2D_DIR}/Dynamics/Contacts/b2PolyAndEdgeContact.cpp
        ${BOX2D_DIR}/Dynamics/Contacts/b2PolyContact.cpp
        ${BOX2D_DIR}/Dynamics/Controllers/b2BuoyancyController.cpp
        ${BOX2D_DIR}/Dynamics/Controllers/b2ConstantAccelController.cpp
        ${BOX2D_DIR}/Dynamics/Controllers/b2ConstantForceController.cpp
        ${BOX2D_DIR}/Dynamics/Controllers/b2Controller.cpp
        ${BOX2D_DIR}/Dynamics/Controllers/b2GravityController.cpp
        ${BOX2D_DIR}/Dynamics/Controllers/b2TensorDampingController.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2DistanceJoint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2GearJoint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2Joint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2LineJoint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2MaxMinJoint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2MouseJoint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2PrismaticJoint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2PulleyJoint.cpp
        ${BOX2D_DIR}/Dynamics/Joints/b2RevoluteJoint.cpp
    )
endif()

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
    # /bigobj is required for MSVC because objects.cpp can get too big on
    # some applications
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /bigobj")
    # silence some stupid warnings
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

# for Linux (set RPATH properly)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_NAME_DIR "@executable_path")

add_executable(Chowdren WIN32 MACOSX_BUNDLE ${SRCS})

find_package(GLFW3 REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenALSoft REQUIRED)
find_package(PythonLibs 2.7 REQUIRED)

include_directories(${OPENALSOFT_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS} ${GLFW_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR} ${BOX2D_INCLUDE_DIR})
target_link_libraries(Chowdren ${GLFW_LIBRARY} ${OPENGL_LIBRARY} 
    ${FREETYPE_LIBRARIES} ${OPENALSOFT_LIBRARY} 
    ${PYTHON_LIBRARIES} ${VORBIS_LIBRARIES} ${PLATFORM_LIBRARIES}
    ${BOX2D_LIBRARY})

include(CPack)