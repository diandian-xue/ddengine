
set(SRC_PATH ${PROJECT_SOURCE_DIR}/lddcl)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_library(lddcl
    ${SOURCES}
    ${HEADERS}
    )


if(${BUILD_SHARED_LIBS})
    target_compile_definitions(lddcl PUBLIC DDCLLUA_EXPORTS)
endif()

target_link_libraries(lddcl PUBLIC ddcl)

if(WIN32)
else()
    set(DDCLLUA_COMPILE_FLAGS -std=gnu99)
    set_target_properties(lddcl PROPERTIES COMPILE_FLAGS ${DDCLLUA_COMPILE_FLAGS})
endif()

if(APPLE)
    if(${BUILD_SHARED_LIBS})
        set_target_properties(lddcl PROPERTIES SUFFIX ".so")
    endif()
endif()

set_target_properties(lddcl PROPERTIES PREFIX "")
set_target_properties(lddcl PROPERTIES OUTPUT_NAME lddcl)

target_include_directories(lddcl PUBLIC ${PROJECT_SOURCE_DIR}/3rd/lua/src)
target_link_libraries(lddcl PUBLIC lualib)
