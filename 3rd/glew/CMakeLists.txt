cmake_minimum_required(VERSION 2.8)
project(glew)


set (GLEW_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# get version from config/version
file (STRINGS ${GLEW_DIR}/config/version  _VERSION_MAJOR_STRING REGEX "GLEW_MAJOR[ ]*=[ ]*[0-9]+.*")
string (REGEX REPLACE "GLEW_MAJOR[ ]*=[ ]*([0-9]+)" "\\1" CPACK_PACKAGE_VERSION_MAJOR ${_VERSION_MAJOR_STRING})
file (STRINGS ${GLEW_DIR}/config/version  _VERSION_MINOR_STRING REGEX "GLEW_MINOR[ ]*=[ ]*[0-9]+.*")
string (REGEX REPLACE "GLEW_MINOR[ ]*=[ ]*([0-9]+)" "\\1" CPACK_PACKAGE_VERSION_MINOR ${_VERSION_MINOR_STRING})
file (STRINGS ${GLEW_DIR}/config/version  _VERSION_PATCH_STRING REGEX "GLEW_MICRO[ ]*=[ ]*[0-9]+.*")
string (REGEX REPLACE "GLEW_MICRO[ ]*=[ ]*([0-9]+)" "\\1" CPACK_PACKAGE_VERSION_PATCH ${_VERSION_PATCH_STRING})
set (GLEW_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})

find_package (OpenGL REQUIRED)
find_package (X11)

set (GLEW_LIBRARIES ${OPENGL_LIBRARIES})
add_definitions (-DGLEW_NO_GLU)
set (GLEW_PUBLIC_HEADERS_FILES ${GLEW_DIR}/include/GL/wglew.h ${GLEW_DIR}/include/GL/glew.h ${GLEW_DIR}/include/GL/glxew.h)
set (GLEW_SRC_FILES ${GLEW_DIR}/src/glew.c)
add_library (glew ${GLEW_PUBLIC_HEADERS_FILES} ${GLEW_SRC_FILES})
target_include_directories(glew PUBLIC ${GLEW_DIR}/include)
target_compile_definitions(glew PUBLIC GLEW_NO_GLU)
target_link_libraries (glew LINK_PUBLIC ${GLEW_LIBRARIES})

if(MSVC)
	if(BUILD_SHARED_LIBS)
		target_compile_definitions (glew PRIVATE "GLEW_BUILD;VC_EXTRALEAN")
	else()
		target_compile_definitions (glew PRIVATE "GLEW_STATIC;VC_EXTRALEAN")
	endif()
	target_compile_options (glew PRIVATE -GS-)
elseif (WIN32 AND ((CMAKE_C_COMPILER_ID MATCHES "GNU") OR (CMAKE_C_COMPILER_ID MATCHES "Clang")))
  # remove stdlib dependency on windows with GCC and Clang (for similar reasons
  # as to MSVC - to allow it to be used with any Windows compiler)
  target_compile_options (glew PRIVATE -fno-builtin -fno-stack-protector)
  target_link_libraries (glew LINK_PRIVATE -nostdlib)
endif ()

if (BUILD_FRAMEWORK AND BUILD_SHARED_LIBS)
  set_target_properties(glew PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION ${GLEW_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER net.sourceforge.glew
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${GLEW_VERSION}
    MACOSX_FRAMEWORK_BUNDLE_VERSION ${GLEW_VERSION}
    XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
    PUBLIC_HEADER "${GLEW_PUBLIC_HEADERS_FILES}"
    OUTPUT_NAME GLEW
  )
endif()

set (GLEWINFO_SRC_FILES ${GLEW_DIR}/src/glewinfo.c)
add_executable (glewinfo ${GLEWINFO_SRC_FILES})
target_link_libraries (glewinfo glew)
if (NOT WIN32)
target_link_libraries(glewinfo ${X11_LIBRARIES})
endif ()

set (VISUALINFO_SRC_FILES ${GLEW_DIR}/src/visualinfo.c)
add_executable (visualinfo ${VISUALINFO_SRC_FILES})
target_link_libraries (visualinfo glew)
if (NOT WIN32)
target_link_libraries(visualinfo ${X11_LIBRARIES})
endif ()
