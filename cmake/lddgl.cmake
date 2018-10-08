
set(SRC_PATH ${PROJECT_SOURCE_DIR}/lddgl)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_library(lddgl
    ${SOURCES}
    ${HEADERS}
    )


if(${BUILD_SHARED_LIBS})
    target_compile_definitions(lddgl PUBLIC DDGLLUA_EXPORTS)
endif()

target_link_libraries(lddgl PUBLIC ddgl)

if(WIN32)
else()
    set(DDCLLUA_COMPILE_FLAGS -std=gnu99)
    set_target_properties(lddgl PROPERTIES COMPILE_FLAGS ${DDCLLUA_COMPILE_FLAGS})
endif()

if(APPLE)
    if(${BUILD_SHARED_LIBS})
        set_target_properties(lddgl PROPERTIES SUFFIX ".so")
    endif()
endif()

set_target_properties(lddgl PROPERTIES PREFIX "")
set_target_properties(lddgl PROPERTIES OUTPUT_NAME lddgl)

target_include_directories(lddgl PUBLIC ${PROJECT_SOURCE_DIR}/3rd/lua/src)
target_link_libraries(lddgl PUBLIC lualib)
