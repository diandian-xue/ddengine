

set(SRC_PATH ${PROJECT_SOURCE_DIR}/test)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)

add_executable(ddengine-test
    ${SOURCES}
    ${HEADERS}
    )

target_link_libraries(ddengine-test PUBLIC ddcl)
set_target_properties(ddengine-test PROPERTIES OUTPUT_NAME ddengine_test)

if(LINUX)
    set(DDCLTEST_COMPILE_FLAGS -std=gnu99)
    set_target_properties(ddengine-test PROPERTIES COMPILE_FLAGS ${DDCLTEST_COMPILE_FLAGS})
endif()
