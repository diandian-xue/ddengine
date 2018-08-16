

set(SRC_PATH ${PROJECT_SOURCE_DIR}/ddcl)

file(GLOB HEADERS ${SRC_PATH}/*.h)
file(GLOB SOURCES ${SRC_PATH}/*.c)
add_library(ddcl
    ${SOURCES}
    ${HEADERS}
    )

target_include_directories(ddcl PUBLIC ${SRC_PATH})
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
    target_compile_definitions(ddcl PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(ddcl PRIVATE _CRT_NONSTDC_NO_DEPRECATE)
else()
	set_target_properties(ddcl PROPERTIES COMPILE_FLAGS -std=gnu99)
endif()

if(LINUX)
    target_compile_definitions(ddcl PUBLIC LINUX)
    target_link_libraries(ddcl PUBLIC pthread)
    target_link_libraries(ddcl PUBLIC m)
endif()

if(APPLE)
    target_compile_definitions(ddcl PUBLIC APPLE)
    target_link_libraries(ddcl PUBLIC pthread)
    #target_link_libraries(ddcl PUBLIC m)
endif()
