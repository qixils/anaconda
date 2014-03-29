if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(LINUX TRUE)
endif()

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.7" CACHE STRING "")
    set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk" CACHE STRING "")
endif()

set(CMAKE_MODULE_PATH "${CHOWDREN_BASE_DIR}/cmake")

if (NOT EMSCRIPTEN)
    include_directories("${CHOWDREN_BASE_DIR}/include/desktop")
endif()
include_directories("${CHOWDREN_BASE_DIR}/include")
include_directories("${CHOWDREN_BASE_DIR}/include/desktop")
include_directories("${CHOWDREN_BASE_DIR}/desktop")
include_directories("${CHOWDREN_BASE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}")
set(CMAKE_INCLUDE_PATH "${CHOWDREN_BASE_DIR}/include"
    "${CHOWDREN_BASE_DIR}/include/desktop" ${CMAKE_INCLUDE_PATH})

option(USE_BOX2D OFF)
option(USE_PYTHON ON)
option(USE_STEAM OFF)
option(ENABLE_STEAM ON)
option(EMULATE_WIIU OFF)

if (EMSCRIPTEN)
    set(USE_GLES2 TRUE)
else()
    option(USE_GL TRUE)
    option(USE_GLES1 FALSE)
    option(USE_GLES2 FALSE)
endif()

if(EMSCRIPTEN)
    add_definitions(-DCHOWDREN_IS_EMSCRIPTEN)
endif()

add_definitions(-DCHOWDREN_IS_DESKTOP)

if (USE_GL)
    add_definitions(-DCHOWDREN_USE_GL)
elseif (USE_GLES1)
    add_definitions(-DCHOWDREN_USE_GLES1)
else()
    add_definitions(-DCHOWDREN_USE_GLES2)
endif()

FILE(GLOB FRAMESRCS "frame*.cpp")

if (EMSCRIPTEN)
    set(PLATFORM_CPP ${CHOWDREN_BASE_DIR}/desktop/emscriptenplatform.cpp)
else()
    set(PLATFORM_CPP ${CHOWDREN_BASE_DIR}/desktop/platform.cpp)
    if (USE_GL)
        set(GLEW_C ${CHOWDREN_BASE_DIR}/glew.c)
    endif()
endif()

set(SRCS
    objects.cpp
    fonts.cpp
    ${CHOWDREN_BASE_DIR}/run.cpp
    ${CHOWDREN_BASE_DIR}/image.cpp
    ${CHOWDREN_BASE_DIR}/desktop/glc.cpp
    ${GLEW_C}
    ${PLATFORM_CPP}
    ${CHOWDREN_BASE_DIR}/shader.cpp
    ${CHOWDREN_BASE_DIR}/font.cpp
    ${CHOWDREN_BASE_DIR}/desktop/glslshader.cpp
    ${FRAMESRCS}
    ${CHOWDREN_BASE_DIR}/movement.cpp
    ${CHOWDREN_BASE_DIR}/common.cpp
    ${CHOWDREN_BASE_DIR}/media.cpp
    ${CHOWDREN_BASE_DIR}/fpslimit.cpp
    ${CHOWDREN_BASE_DIR}/include/tinythread/tinythread.cpp
    ${CHOWDREN_BASE_DIR}/coltree.cpp)

if (USE_STEAM)
    add_definitions(-DCHOWDREN_USE_STEAM)
    if (ENABLE_STEAM)
        add_definitions(-DCHOWDREN_ENABLE_STEAM)
        include_directories("${CHOWDREN_BASE_DIR}/steam")
        if (WIN32)
            set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES}
                "${CHOWDREN_BASE_DIR}/steam/sdk/redistributable_bin/steam_api.lib")
        elseif (APPLE)
            set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES}
                "${CHOWDREN_BASE_DIR}/steam/sdk/redistributable_bin/osx32/libsteam_api.dylib")
        else()
            set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES}
                "${CHOWDREN_BASE_DIR}/steam/sdk/redistributable_bin/linux32/libsteam_api.so")
        endif()
    endif()
    set(SRCS
        ${SRCS}
        steamext.cpp
    )
endif()

if (USE_BOX2D)
    add_definitions(-DCHOWDREN_USE_BOX2D)
    set(BOX2D_DIR "${CHOWDREN_BASE_DIR}/include/Box2D")
    set(SRCS
        ${SRCS}
        box2d/box2dext.cpp
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

if (EMULATE_WIIU)
    add_definitions(-DCHOWDREN_EMULATE_WIIU)
endif()

if(WIN32)
    set(CMAKE_LIBRARY_PATH "${CHOWDREN_BASE_DIR}/lib/win32"
        ${CMAKE_LIBRARY_PATH})
    set(CMAKE_INCLUDE_PATH "${CHOWDREN_BASE_DIR}/include/win32"
        ${CMAKE_INCLUDE_PATH})
    set(SRCS ${SRCS} resource.rc)
    set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} winmm)
elseif(APPLE)
    set(CMAKE_LIBRARY_PATH "${CHOWDREN_BASE_DIR}/lib/osx"
        ${CMAKE_LIBRARY_PATH})
    set(CMAKE_INCLUDE_PATH "${CHOWDREN_BASE_DIR}/include/osx"
        ${CMAKE_INCLUDE_PATH})
    find_library(CARBON_LIBRARY Carbon)
    set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${CARBON_LIBRARY})
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
    set(MACOSX_BUNDLE_BUNDLE_NAME ${APP_NAME})
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_NAME})
    set(MACOSX_BUNDLE_INFO_STRING "${APP_NAME} - ${APP_VERSION}")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${APP_VERSION}")
    set(MACOSX_BUNDLE_COPYRIGHT "${APP_COPYRIGHT}")
    set(DATA_SRCS ${DATA_SRCS} "${CMAKE_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
    set(DATA_SRCS ${DATA_SRCS} "${CMAKE_SOURCE_DIR}/alsoft.conf")
endif()

# include alsoft.conf
set(DATA_SRCS ${DATA_SRCS} "${CMAKE_SOURCE_DIR}/alsoft.conf")

if (USE_PYTHON)
    add_definitions(-DCHOWDREN_USE_PYTHON)
    set(SRCS
        ${SRCS}
        ${CHOWDREN_BASE_DIR}/pythonext.cpp
    )
    find_package(PythonLibs 2.7 REQUIRED)
endif()

# for Linux (set RPATH properly)
set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
set(CMAKE_INSTALL_NAME_DIR "@executable_path")

if(APPLE)
    set_source_files_properties(${DATA_SRCS} PROPERTIES 
        MACOSX_PACKAGE_LOCATION Resources)
    set(SRCS ${SRCS} ${DATA_SRCS})
endif()

if(EMSCRIPTEN)
    # libogg
    file(GLOB LIBOGG_C "${CHOWDREN_BASE_DIR}/staticlibs/libogg/src/*.c")
    file(GLOB LIBVORBIS_C "${CHOWDREN_BASE_DIR}/staticlibs/libvorbis/lib/*.c")
    set(FREETYPE_DIR "${CHOWDREN_BASE_DIR}/staticlibs/freetype")
    set(SRCS ${SRCS} 
        ${LIBOGG_C} 
        ${LIBVORBIS_C}
        ${FREETYPE_DIR}/base/ftsystem.c
        ${FREETYPE_DIR}/base/ftinit.c
        ${FREETYPE_DIR}/base/ftdebug.c
        ${FREETYPE_DIR}/base/ftbase.c
        ${FREETYPE_DIR}/base/ftbbox.c
        ${FREETYPE_DIR}/base/ftglyph.c
        ${FREETYPE_DIR}/base/ftstroke.c
        ${FREETYPE_DIR}/base/ftbitmap.c
        ${FREETYPE_DIR}/sfnt/sfnt.c
        ${FREETYPE_DIR}/truetype/truetype.c
        ${FREETYPE_DIR}/smooth/smooth.c
        ${FREETYPE_DIR}/autofit/autofit.c
        ${FREETYPE_DIR}/psnames/psnames.c
    )
    add_definitions(-DFT2_BUILD_LIBRARY)
    set(VORBIS_INCLUDE_DIR "${CHOWDREN_BASE_DIR}/staticlibs/libvorbis/lib")
    include_directories(${CHOWDREN_BASE_DIR}/include/staticlibs)
else()
    find_package(SDL2 REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(OpenALSoft REQUIRED)
    find_package(Vorbis REQUIRED)
    if (USE_GL)
        find_package(OpenGL REQUIRED)
    else()
        find_package(OpenGLES2 REQUIRED)
    endif()
endif()

set(EXE_NAME "Chowdren")
add_executable(${EXE_NAME} WIN32 MACOSX_BUNDLE ${SRCS})

include_directories(${OPENALSOFT_INCLUDE_DIR} ${PYTHON_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR}
    ${BOX2D_INCLUDE_DIR})
target_link_libraries(${EXE_NAME} ${SDL2_LIBRARY} ${OPENGL_LIBRARY}
    ${FREETYPE_LIBRARIES} ${OPENALSOFT_LIBRARY} 
    ${PYTHON_LIBRARIES} ${VORBIS_LIBRARIES} ${PLATFORM_LIBRARIES}
    ${BOX2D_LIBRARY} ${OPENGLES2_LIBRARIES} ${EGL_LIBRARIES})


set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

set(BIN_DIR ".")
set(DATA_PATH ".")
if(APPLE)
    set(DATA_PATH "/${EXE_NAME}.app/Contents/Resources")
    set(EXE_PATH "\${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.app")
    set()
elseif(WIN32)
    set(EXE_PATH "\${CMAKE_INSTALL_PREFIX}/${EXE_NAME}.exe")
    set(DLL_PATH "${CMAKE_SOURCE_DIR}/bin/win32")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win32/Microsoft.VC90.CRT"
            DESTINATION .)
else()
    set(BIN_DIR "bin")
    set(EXE_PATH "\${CMAKE_INSTALL_PREFIX}/bin/${EXE_NAME}")
endif()

if(NOT WIN32)
    include(InstallRequiredSystemLibraries)
endif()
install(TARGETS ${EXE_NAME} DESTINATION ${BIN_DIR})
install(CODE "
    function(gp_item_default_embedded_path_override item default_embedded_path_var)
        set(path \"\${\${default_embedded_path_var}}\")
        if (${LINUX})
            set(path \"@executable_path/../lib\")
        endif()
        set(\${default_embedded_path_var} \"\${path}\" PARENT_SCOPE)
    endfunction(gp_item_default_embedded_path_override)

   set(BU_CHMOD_BUNDLE_ITEMS TRUE)
   include(BundleUtilities)
   fixup_bundle(\"${EXE_PATH}\"   \"\"   \"${DLL_PATH}\")
   " COMPONENT Runtime)

if(NOT APPLE)
    install(FILES ${DATA_SRCS} DESTINATION ".")
endif()
