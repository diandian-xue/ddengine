
set(SRC_PATH ${PROJECT_SOURCE_DIR}/example/simple-httpd)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_executable(simple-httpd
    ${SOURCES}
    ${HEADERS}
    )

target_link_libraries(simple-httpd PUBLIC ddcl)
set_target_properties(simple-httpd PROPERTIES OUTPUT_NAME simple-httpd)

if(LINUX)
    set(COMPILE_FLAGS -std=gnu99)
    set_target_properties(simple-httpd PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS})
endif()
