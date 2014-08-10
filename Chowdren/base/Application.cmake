cmake_minimum_required (VERSION 2.6)
project(Chowdren)

set(CHOWDREN_BASE_DIR "%(base_path)s")
set(APP_NAME "%(name)s")
set(APP_VERSION "%(version)s")
set(APP_COPYRIGHT "%(copyright)s")

%(frame_srcs)s

%(event_srcs)s

%(object_srcs)s

include(${CHOWDREN_BASE_DIR}/CMakeLists.txt)
