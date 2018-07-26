cmake_minimum_required(VERSION 2.8)

set(LUA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/3rd/lua/src)

set(SRC_PATH ${PROJECT_SOURCE_DIR}/3rd/lua-cjson)
set(SOURCES
    ${SRC_PATH}/lua_cjson.c
    ${SRC_PATH}/strbuf.c
    )

option(USE_INTERNAL_FPCONV "Use internal strtod() / g_fmt() code for performance")
if(NOT USE_INTERNAL_FPCONV)
    # Use libc number conversion routines (strtod(), sprintf())
    set(FPCONV_SOURCES ${SRC_PATH}/fpconv.c)
else()
    # Use internal number conversion routines
    add_definitions(-DUSE_INTERNAL_FPCONV)
    set(FPCONV_SOURCES ${SRC_PATH}/g_fmt.c ${SRC_PATH}/dtoa.c)

    include(TestBigEndian)
    TEST_BIG_ENDIAN(IEEE_BIG_ENDIAN)
    if(IEEE_BIG_ENDIAN)
        add_definitions(-DIEEE_BIG_ENDIAN)
    endif()

    if(MULTIPLE_THREADS)
        set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
        find_package(Threads REQUIRED)
        if(NOT CMAKE_USE_PTHREADS_INIT)
            message(FATAL_ERROR
                    "Pthreads not found - required by MULTIPLE_THREADS option")
        endif()
        add_definitions(-DMULTIPLE_THREADS)
    endif()
endif()

add_library(lua-cjson ${SOURCES} ${FPCONV_SOURCES})
set_target_properties(lua-cjson PROPERTIES PREFIX "")
set_target_properties(lua-cjson PROPERTIES OUTPUT_NAME cjson)
target_include_directories(lua-cjson PUBLIC ${LUA_INCLUDE_DIR})

if(${BUILD_SHARED_LIBS})
    target_compile_definitions(lua-cjson PUBLIC LUA_CJSON_EXPORTS)
endif()


# Handle platforms missing isinf() macro (Eg, some Solaris systems).
include(CheckSymbolExists)
CHECK_SYMBOL_EXISTS(isinf math.h HAVE_ISINF)
if(NOT HAVE_ISINF)
    add_definitions(-DUSE_INTERNAL_ISINF)
endif()

if(APPLE)
    set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
        "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -undefined dynamic_lookup")
endif()

if(WIN32)
    # Win32 modules need to be linked to the Lua library.
    target_link_libraries(lua-cjson lualib)
    # Windows sprintf()/strtod() handle NaN/inf differently. Not supported.
    add_definitions(-DDISABLE_INVALID_NUMBERS)
endif()
