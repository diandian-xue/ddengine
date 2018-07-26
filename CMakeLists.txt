cmake_minimum_required(VERSION 2.8)
project(ddengine)

set(OUTPUT_PATH bin)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH})
set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/3rd/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/example/cmake/")

include(lua)
include(lua-cjson)
include(ddcl)


include(lddcl)

include(ddengine-test)
include(simple-httpd)

#include(ddcl_luabind)
