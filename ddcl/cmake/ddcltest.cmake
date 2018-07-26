

project(ddcltest)

set(SRC_PATH ${PROJECT_SOURCE_DIR}/test)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_executable(ddcltest
    ${SOURCES}
    ${HEADERS}
    )

target_link_libraries(ddcltest PUBLIC ddcl)
set(DDCLTEST_COMPILE_FLAGS -std=gnu99)
set_target_properties(ddcltest PROPERTIES COMPILE_FLAGS ${DDCLTEST_COMPILE_FLAGS})

