include(CheckIncludeFile)

set(VERSION_MAJOR "2")
set(VERSION_MINOR "9")
set(VERSION_PATCH "1")
set(FREETYPE_ROOT ${PROJECT_SOURCE_DIR}/3rd/freetype)


set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(SHARED_LIBRARY_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})



file(GLOB PUBLIC_HEADERS 
    ${FREETYPE_ROOT}/include/ft2build.h 
    ${FREETYPE_ROOT}/include/freetype/*.h)
file(GLOB PUBLIC_CONFIG_HEADERS ${FREETYPE_ROOT}/include/freetype/config/*.h)
file(GLOB PRIVATE_HEADERS ${FREETYPE_ROOT}/include/freetype/internal/*.h)

set(BASE_SRCS
    ${FREETYPE_ROOT}/src/autofit/autofit.c
    ${FREETYPE_ROOT}/src/base/ftbase.c
    ${FREETYPE_ROOT}/src/base/ftbbox.c
    ${FREETYPE_ROOT}/src/base/ftbdf.c
    ${FREETYPE_ROOT}/src/base/ftbitmap.c
    ${FREETYPE_ROOT}/src/base/ftcid.c
    ${FREETYPE_ROOT}/src/base/ftfstype.c
    ${FREETYPE_ROOT}/src/base/ftgasp.c
    ${FREETYPE_ROOT}/src/base/ftglyph.c
    ${FREETYPE_ROOT}/src/base/ftgxval.c
    ${FREETYPE_ROOT}/src/base/ftinit.c
    ${FREETYPE_ROOT}/src/base/ftmm.c
    ${FREETYPE_ROOT}/src/base/ftotval.c
    ${FREETYPE_ROOT}/src/base/ftpatent.c
    ${FREETYPE_ROOT}/src/base/ftpfr.c
    ${FREETYPE_ROOT}/src/base/ftstroke.c
    ${FREETYPE_ROOT}/src/base/ftsynth.c
    ${FREETYPE_ROOT}/src/base/ftsystem.c
    ${FREETYPE_ROOT}/src/base/fttype1.c
    ${FREETYPE_ROOT}/src/base/ftwinfnt.c
    ${FREETYPE_ROOT}/src/bdf/bdf.c
    ${FREETYPE_ROOT}/src/bzip2/ftbzip2.c
    ${FREETYPE_ROOT}/src/cache/ftcache.c
    ${FREETYPE_ROOT}/src/cff/cff.c
    ${FREETYPE_ROOT}/src/cid/type1cid.c
    ${FREETYPE_ROOT}/src/gzip/ftgzip.c
    ${FREETYPE_ROOT}/src/lzw/ftlzw.c
    ${FREETYPE_ROOT}/src/pcf/pcf.c
    ${FREETYPE_ROOT}/src/pfr/pfr.c
    ${FREETYPE_ROOT}/src/psaux/psaux.c
    ${FREETYPE_ROOT}/src/pshinter/pshinter.c
    ${FREETYPE_ROOT}/src/psnames/psnames.c
    ${FREETYPE_ROOT}/src/raster/raster.c
    ${FREETYPE_ROOT}/src/sfnt/sfnt.c
    ${FREETYPE_ROOT}/src/smooth/smooth.c
    ${FREETYPE_ROOT}/src/truetype/truetype.c
    ${FREETYPE_ROOT}/src/type1/type1.c
    ${FREETYPE_ROOT}/src/type42/type42.c
    ${FREETYPE_ROOT}/src/winfonts/winfnt.c
)

if (WIN32)
  enable_language(RC)
  list(APPEND BASE_SRCS ${FREETYPE_ROOT}/builds/windows/ftdebug.c
                        ${FREETYPE_ROOT}/src/base/ftver.rc)
elseif (WINCE)
  list(APPEND BASE_SRCS ${FREETYPE_ROOT}/builds/wince/ftdebug.c)
else ()
  list(APPEND BASE_SRCS ${FREETYPE_ROOT}/src/base/ftdebug.c)
endif ()

if (BUILD_FRAMEWORK)
  list(APPEND BASE_SRCS ${FREETYPE_ROOT}/builds/mac/freetype-Info.plist)
endif ()

add_library(freetype
  ${PUBLIC_HEADERS}
  ${PUBLIC_CONFIG_HEADERS}
  ${PRIVATE_HEADERS}
  ${BASE_SRCS}
)
set_target_properties(freetype PROPERTIES C_VISIBILITY_PRESET hidden)

target_compile_definitions(freetype PRIVATE FT2_BUILD_LIBRARY)
target_include_directories(freetype PUBLIC ${FREETYPE_ROOT}/include)

 if (WIN32)
  target_compile_definitions(
    freetype PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
    if(${BUILD_SHARED_LIBS})
        target_compile_definitions(freetype PRIVATE DLL_EXPORT)
    endif()
    target_compile_definitions(freetype PUBLIC FT2_DLLIMPORT)
endif ()

target_link_libraries(freetype PUBLIC zlib)
target_link_libraries(freetype PUBLIC libpng)
