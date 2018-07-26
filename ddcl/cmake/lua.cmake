cmake_minimum_required(VERSION 2.8)
project(lua)


set(SRC_PATH ${PROJECT_SOURCE_DIR}/bind/lua)

file(GLOB HEADERS ${SRC_PATH}/src/*.h)
set(SOURCES
    ${SRC_PATH}/src/lapi.c
    ${SRC_PATH}/src/lcode.c
    ${SRC_PATH}/src/ldebug.c
    ${SRC_PATH}/src/lgc.c
    ${SRC_PATH}/src/lmathlib.c
    ${SRC_PATH}/src/lopcodes.c
    ${SRC_PATH}/src/lstring.c
    ${SRC_PATH}/src/ltm.c
    ${SRC_PATH}/src/lutf8lib.c
    ${SRC_PATH}/src/lauxlib.c
    ${SRC_PATH}/src/lcorolib.c
    ${SRC_PATH}/src/ldo.c
    ${SRC_PATH}/src/linit.c
    ${SRC_PATH}/src/lmem.c
    ${SRC_PATH}/src/loslib.c
    ${SRC_PATH}/src/lstrlib.c
    ${SRC_PATH}/src/lvm.c
    ${SRC_PATH}/src/lbaselib.c
    ${SRC_PATH}/src/lctype.c
    ${SRC_PATH}/src/ldump.c
    ${SRC_PATH}/src/liolib.c
    ${SRC_PATH}/src/loadlib.c
    ${SRC_PATH}/src/lparser.c
    ${SRC_PATH}/src/ltable.c
    ${SRC_PATH}/src/lzio.c
    ${SRC_PATH}/src/lbitlib.c
    ${SRC_PATH}/src/ldblib.c
    ${SRC_PATH}/src/lfunc.c
    ${SRC_PATH}/src/llex.c
    ${SRC_PATH}/src/lobject.c
    ${SRC_PATH}/src/lstate.c
    ${SRC_PATH}/src/ltablib.c
    ${SRC_PATH}/src/lundump.c
)
add_library(lualib
    ${HEADERS}
    ${SOURCES}
    )
target_include_directories(lualib PUBLIC src)
set_target_properties(lualib
    PROPERTIES
    OUTPUT_NAME lua)

if(WIN32)
    if(${BUILD_SHARED_LIBS})
        target_compile_definitions(lualib PUBLIC LUA_BUILD_AS_DLL)
    endif()
    #target_compile_definitions(lualib PUBLIC LUA_DL_DLL)
else()
    target_link_libraries(lualib PUBLIC dl)
    target_compile_definitions(lualib PUBLIC LUA_USE_DLOPEN)
endif()

add_executable(lua
    ${SRC_PATH}/src/lua.c
    )
target_link_libraries(lua lualib)
    
add_executable(luac
    ${SRC_PATH}/src/luac.c
    ${SOURCES}
    )
if(WIN32)
else()
    target_compile_definitions(lualib PUBLIC LUA_USE_DLOPEN)
    target_link_libraries(lualib PUBLIC dl)
    target_link_libraries(lualib PUBLIC m)
    target_link_libraries(luac PUBLIC m)
endif()
