
set(VERSION "1.2.11")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
#enable_testing()

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)

set(ZLIB_PRIVATE_DEFINITIONS "")

#
# Check to see if we have large file support
#
set(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE=1)
# We add these other definitions here because CheckTypeSize.cmake
# in CMake 2.4.x does not automatically do so and we want
# compatibility with CMake 2.4.x.
if(HAVE_SYS_TYPES_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_SYS_TYPES_H)
endif()
if(HAVE_STDINT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDINT_H)
endif()
if(HAVE_STDDEF_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_STDDEF_H)
endif()
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   #add_definitions(-D_LARGEFILE64_SOURCE=1)
   list(APPEND ZLIB_PRIVATE_DEFINITIONS _LARGEFILE64_SOURCE=1)
endif()
set(CMAKE_REQUIRED_DEFINITIONS) # clear variable

#
# Check for fseeko
#
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    #add_definitions(-DNO_FSEEKO)
	list(APPEND ZLIB_PRIVATE_DEFINITIONS NO_FSEEKO)
endif()

#
# Check for unistd.h
#
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    #set(CMAKE_DEBUG_POSTFIX "d")
    #add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    #add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    #include_directories(${CMAKE_CURRENT_SOURCE_DIR})
	
	list(APPEND ZLIB_PRIVATE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
	list(APPEND ZLIB_PRIVATE_DEFINITIONS _CRT_NONSTDC_NO_DEPRECATE)
endif()

set(ZLIB_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rd/zlib)

set(ZLIB_PUBLIC_HDRS
    ${ZLIB_ROOT}/zconf.h
    ${ZLIB_ROOT}/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    ${ZLIB_ROOT}/crc32.h
    ${ZLIB_ROOT}/deflate.h
    ${ZLIB_ROOT}/gzguts.h
    ${ZLIB_ROOT}/inffast.h
    ${ZLIB_ROOT}/inffixed.h
    ${ZLIB_ROOT}/inflate.h
    ${ZLIB_ROOT}/inftrees.h
    ${ZLIB_ROOT}/trees.h
    ${ZLIB_ROOT}/zutil.h
)
set(ZLIB_SRCS
    ${ZLIB_ROOT}/adler32.c
    ${ZLIB_ROOT}/compress.c
    ${ZLIB_ROOT}/crc32.c
    ${ZLIB_ROOT}/deflate.c
    ${ZLIB_ROOT}/gzclose.c
    ${ZLIB_ROOT}/gzlib.c
    ${ZLIB_ROOT}/gzread.c
    ${ZLIB_ROOT}/gzwrite.c
    ${ZLIB_ROOT}/inflate.c
    ${ZLIB_ROOT}/infback.c
    ${ZLIB_ROOT}/inftrees.c
    ${ZLIB_ROOT}/inffast.c
    ${ZLIB_ROOT}/trees.c
    ${ZLIB_ROOT}/uncompr.c
    ${ZLIB_ROOT}/zutil.c
)

if(NOT MINGW)
    set(ZLIB_DLL_SRCS
        ${ZLIB_ROOT}/win32/zlib1.rc # If present will override custom build rule below.
    )
endif()

if(CMAKE_COMPILER_IS_GNUCC)
    if(ASM686)
        set(ZLIB_ASMS ${ZLIB_ROOT}/contrib/asm686/match.S)
    elseif (AMD64)
        set(ZLIB_ASMS ${ZLIB_ROOT}/contrib/amd64/amd64-match.S)
    endif ()

    if(ZLIB_ASMS)
        #add_definitions(-DASMV)
		list(APPEND ZLIB_PRIVATE_DEFINITIONS ASMV)
        set_source_files_properties(${ZLIB_ASMS} PROPERTIES LANGUAGE C COMPILE_FLAGS -DNO_UNDERLINE)
    endif()
endif()

if(MSVC)
    if(ASM686)
        ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
            ${ZLIB_ROOT}/contrib/masmx86/inffas32.asm
            ${ZLIB_ROOT}/contrib/masmx86/match686.asm
        )
    elseif (AMD64)
        ENABLE_LANGUAGE(ASM_MASM)
        set(ZLIB_ASMS
            ${ZLIB_ROOT}/contrib/masmx64/gvmat64.asm
            ${ZLIB_ROOT}/contrib/masmx64/inffasx64.asm
        )
    endif()

    if(ZLIB_ASMS)
        #add_definitions(-DASMV -DASMINF)
		list(APPEND ZLIB_PRIVATE_DEFINITIONS ASMV ASMINF)
    endif()
endif()

# parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
file(READ ${ZLIB_ROOT}/zlib.h _zlib_h_contents)
string(REGEX REPLACE ".*#define[ \t]+ZLIB_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*"
    "\\1" ZLIB_FULL_VERSION ${_zlib_h_contents})

add_library(zlib
    ${ZLIB_SRCS} ${ZLIB_ASMS} ${ZLIB_DLL_SRCS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})

target_include_directories(zlib PUBLIC  ${ZLIB_ROOT})
target_compile_definitions(zlib PRIVATE ${ZLIB_PRIVATE_DEFINITIONS})
	

if(${BUILD_SHARED_LIBS})
    target_compile_definitions(zlib PUBLIC ZLIB_DLL)
endif()

if(UNIX)
    # On unix-like platforms the library is almost always called libz
    #set_target_properties(zlib zlibstatic PROPERTIES OUTPUT_NAME z)
   if(NOT APPLE)
       set_target_properties(zlib PROPERTIES LINK_FLAGS "-Wl,--version-script,\"${ZLIB_ROOT}/zlib.map\"")
   endif()
elseif(BUILD_SHARED_LIBS AND WIN32)
    # Creates zlib1.dll when building shared library version
    #set_target_properties(zlib PROPERTIES SUFFIX "1.dll")
endif()
