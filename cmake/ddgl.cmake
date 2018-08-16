
set(SRC_PATH ${PROJECT_SOURCE_DIR}/ddgl)
file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)
add_library(ddgl
    ${SOURCES}
    ${HEADERS}
    )

if(${BUILD_SHARED_LIBS})
    target_compile_definitions(ddgl PUBLIC DDGL_EXPORTS)
endif()

if(WIN32)
    target_link_libraries(ddgl PUBLIC opengl32)
else()
    set_target_properties(ddgl PROPERTIES COMPILE_FLAGS -std=gnu99)
endif()

if(LINUX)
    target_compile_definitions(ddgl PUBLIC LINUX)
    target_link_libraries(ddgl PUBLIC X11)
    target_link_libraries(ddgl PUBLIC GL)
endif()


target_include_directories(ddgl PUBLIC ${SRC_PATH})

target_link_libraries(ddgl PUBLIC ddcl)
target_link_libraries(ddgl PUBLIC freetype)
target_link_libraries(ddgl PUBLIC glew)