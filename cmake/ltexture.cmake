
set(SRC_PATH ${PROJECT_SOURCE_DIR}/ltexture)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_library(ltexture
    ${SOURCES}
    ${HEADERS}
    )


if(${BUILD_SHARED_LIBS})
    target_compile_definitions(ltexture PUBLIC LTEXTURE_EXPORTS)
endif()

target_link_libraries(ltexture PUBLIC freetype)

if(WIN32)
else()
    set(DDCLLUA_COMPILE_FLAGS -std=gnu99)

endif()

if(LINUX)
    if(${BUILD_SHARED_LIBS})
        set_target_properties(ltexture PROPERTIES SUFFIX ".so")
    endif()
endif()

set_target_properties(ltexture PROPERTIES PREFIX "")
set_target_properties(ltexture PROPERTIES OUTPUT_NAME ltexture)

#target_include_directories(ltexture PUBLIC ${PROJECT_SOURCE_DIR}/3rd/lua/src)
target_include_directories(ltexture PUBLIC ${PROJECT_SOURCE_DIR}/ddcl)
target_link_libraries(ltexture PUBLIC lualib)
