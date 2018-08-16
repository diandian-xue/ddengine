cmake_minimum_required(VERSION 2.8)
project(ddengine)

set(OUTPUT_PATH bin)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_PATH})
set(LIBRARY_OUTPUT_PATH ${OUTPUT_PATH})

include(3rd/cmake/lua.cmake)
include(3rd/cmake/lua-cjson.cmake)
include(3rd/cmake/glew.cmake)
include(3rd/cmake/zlib.cmake)
include(3rd/cmake/libpng.cmake)
include(3rd/cmake/freetype.cmake)

include(cmake/ddcl.cmake)
include(cmake/lddcl.cmake)

include(cmake/ddgl.cmake)

include(example/cmake/simple-httpd.cmake)

