
set(SRC_PATH ${PROJECT_SOURCE_DIR}/example/simple-gl)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_executable(simple-gl
    ${SOURCES}
    ${HEADERS}
    )

target_link_libraries(simple-gl PUBLIC ddcl)
target_link_libraries(simple-gl PUBLIC ddgl)
set_target_properties(simple-gl PROPERTIES OUTPUT_NAME simple-gl)

if(LINUX)
    set(COMPILE_FLAGS -std=gnu99)
    set_target_properties(simple-gl PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS})
endif()
