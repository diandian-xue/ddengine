cmake_minimum_required(VERSION 2.8)
project(ddcl_solution)

set(OP ../bin)
set(EXECUTABLE_OUTPUT_PATH ${OP})
set(LIBRARY_OUTPUT_PATH ${OP})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")

include(ddcl)
include(lua)
include(ddcl_luabind)
include(ddcltest)
