
project(ddcl_luabind)

set(SRC_PATH ${PROJECT_SOURCE_DIR}/bind/src)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_library(lddcl
    ${SOURCES}
    ${HEADERS}
    )


if(${BUILD_SHARED_LIBS})
    target_compile_definitions(lddcl PUBLIC CLLUA_EXPORTS)
endif()

target_link_libraries(lddcl PUBLIC ddcl)
set(DDCLLUA_COMPILE_FLAGS -std=gnu99)
set_target_properties(lddcl PROPERTIES COMPILE_FLAGS ${DDCLLUA_COMPILE_FLAGS})

target_include_directories(lddcl PUBLIC ${PROJECT_SOURCE_DIR}/bind/lua/src)
target_link_libraries(lddcl PUBLIC lualib)
