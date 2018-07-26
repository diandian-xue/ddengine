

project(ddcl)

set(SRC_PATH ${PROJECT_SOURCE_DIR}/src)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)
add_library(ddcl
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(ddcl PUBLIC src)
if(${BUILD_SHARED_LIBS})
    target_compile_definitions(ddcl PUBLIC DDCL_EXPORTS)
endif()

#设置 socket 用 select 模型
if(${DDSOCKETPOLL_USE_SELECT})
    target_compile_definitions(ddcl PUBLIC DDSOCKETPOLL_USE_SELECT)
endif()
if(${DDMALLOC_RECORD})
    target_compile_definitions(ddcl PUBLIC DDMALLOC_RECORD)
endif()

if(WIN32)
    target_compile_definitions(ddcl PUBLIC WIN32)
	target_compile_definitions(ddcl PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()

if(LINUX)
    target_compile_definitions(ddcl PUBLIC LINUX)
    set(DDCL_COMPILE_FLAGS -std=gnu99)
    set_target_properties(ddcl PROPERTIES COMPILE_FLAGS ${DDCL_COMPILE_FLAGS})
    target_link_libraries(ddcl PUBLIC pthread)
    target_link_libraries(ddcl PUBLIC m)
endif()

if(APPLE)
    target_compile_definitions(ddcl PUBLIC APPLE)
    set(DDCL_COMPILE_FLAGS -std=c99)
    set_target_properties(ddcl PROPERTIES COMPILE_FLAGS ${DDCL_COMPILE_FLAGS})
    target_link_libraries(ddcl PUBLIC pthread)
    #target_link_libraries(ddcl PUBLIC m)

endif()