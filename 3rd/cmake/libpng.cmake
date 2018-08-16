
set(VERSION 1.6.35)

set(LIBPNG_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rd/libpng)
set(LIBPNG_PRIVATE_DEFINITIONS "")
option(PNG_HARDWARE_OPTIMIZATIONS "Enable Hardware Optimizations" ON)

if(PNG_HARDWARE_OPTIMIZATIONS)
# set definitions and sources for arm
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm" OR
  CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
  set(PNG_ARM_NEON_POSSIBLE_VALUES check on off)
  set(PNG_ARM_NEON "check" CACHE STRING "Enable ARM NEON optimizations:
     check: (default) use internal checking code;
     off: disable the optimizations;
     on: turn on unconditionally.")
  set_property(CACHE PNG_ARM_NEON PROPERTY STRINGS
     ${PNG_ARM_NEON_POSSIBLE_VALUES})
  list(FIND PNG_ARM_NEON_POSSIBLE_VALUES ${PNG_ARM_NEON} index)
  if(index EQUAL -1)
    message(FATAL_ERROR
      " PNG_ARM_NEON must be one of [${PNG_ARM_NEON_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_ARM_NEON} STREQUAL "no")
    set(libpng_arm_sources
      ${LIBPNG_ROOT}/arm/arm_init.c
      ${LIBPNG_ROOT}/arm/filter_neon.S
      ${LIBPNG_ROOT}/arm/filter_neon_intrinsics.c)

    if(${PNG_ARM_NEON} STREQUAL "on")
      #add_definitions(-DPNG_ARM_NEON_OPT=2)
	  list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_ARM_NEON_OPT=2)
    elseif(${PNG_ARM_NEON} STREQUAL "check")
      #add_definitions(-DPNG_ARM_NEON_CHECK_SUPPORTED)
	  list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_ARM_NEON_CHECK_SUPPORTED)
    endif()
  else()
    #add_definitions(-DPNG_ARM_NEON_OPT=0)
	list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_ARM_NEON_OPT=0)
  endif()
endif()

# set definitions and sources for powerpc
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^powerpc*" OR
	CMAKE_SYSTEM_PROCESSOR MATCHES "^ppc64*" )
  set(PNG_POWERPC_VSX_POSSIBLE_VALUES on off)
  set(PNG_POWERPC_VSX "on" CACHE STRING "Enable POWERPC VSX optimizations:
     off: disable the optimizations.")
  set_property(CACHE PNG_POWERPC_VSX PROPERTY STRINGS
     ${PNG_POWERPC_VSX_POSSIBLE_VALUES})
  list(FIND PNG_POWERPC_VSX_POSSIBLE_VALUES ${PNG_POWERPC_VSX} index)
  if(index EQUAL -1)
    message(FATAL_ERROR
      " PNG_POWERPC_VSX must be one of [${PNG_POWERPC_VSX_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_POWERPC_VSX} STREQUAL "no")
    set(libpng_powerpc_sources
      ${LIBPNG_ROOT}/powerpc/powerpc_init.c
      ${LIBPNG_ROOT}/powerpc/filter_vsx_intrinsics.c)
    if(${PNG_POWERPC_VSX} STREQUAL "on")
      #add_definitions(-DPNG_POWERPC_VSX_OPT=2)
	  list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_POWERPC_VSX_OPT=2)
    endif()
  else()
    #add_definitions(-DPNG_POWERPC_VSX_OPT=0)
	list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_POWERPC_VSX_OPT=0)
  endif()
endif()

# set definitions and sources for intel
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^i?86" OR
	CMAKE_SYSTEM_PROCESSOR MATCHES "^x86_64*" )
  set(PNG_INTEL_SSE_POSSIBLE_VALUES on off)
  set(PNG_INTEL_SSE "on" CACHE STRING "Enable INTEL_SSE optimizations:
     off: disable the optimizations")
  set_property(CACHE PNG_INTEL_SSE PROPERTY STRINGS
     ${PNG_INTEL_SSE_POSSIBLE_VALUES})
  list(FIND PNG_INTEL_SSE_POSSIBLE_VALUES ${PNG_INTEL_SSE} index)
  if(index EQUAL -1)
    message(FATAL_ERROR
      " PNG_INTEL_SSE must be one of [${PNG_INTEL_SSE_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_INTEL_SSE} STREQUAL "no")
    set(libpng_intel_sources
      ${LIBPNG_ROOT}/intel/intel_init.c
      ${LIBPNG_ROOT}/intel/filter_sse2_intrinsics.c)
    if(${PNG_INTEL_SSE} STREQUAL "on")
      #add_definitions(-DPNG_INTEL_SSE_OPT=1)
	  list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_INTEL_SSE_OPT=1)
    endif()
  else()
    #add_definitions(-DPNG_INTEL_SSE_OPT=0)
	list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_INTEL_SSE_OPT=0)
  endif()
endif()

# set definitions and sources for MIPS
if(CMAKE_SYSTEM_PROCESSOR MATCHES "mipsel*" OR
	CMAKE_SYSTEM_PROCESSOR MATCHES "mips64el*" )
  set(PNG_MIPS_MSA_POSSIBLE_VALUES on off)
  set(PNG_MIPS_MSA "on" CACHE STRING "Enable MIPS_MSA optimizations:
     off: disable the optimizations")
  set_property(CACHE PNG_MIPS_MSA PROPERTY STRINGS
     ${PNG_MIPS_MSA_POSSIBLE_VALUES})
  list(FIND PNG_MIPS_MSA_POSSIBLE_VALUES ${PNG_MIPS_MSA} index)
  if(index EQUAL -1)
    message(FATAL_ERROR
      " PNG_MIPS_MSA must be one of [${PNG_MIPS_MSA_POSSIBLE_VALUES}]")
  elseif(NOT ${PNG_MIPS_MSA} STREQUAL "no")
    set(libpng_mips_sources
      ${LIBPNG_ROOT}/mips/mips_init.c
      ${LIBPNG_ROOT}/mips/filter_msa_intrinsics.c)
    if(${PNG_MIPS_MSA} STREQUAL "on")
      #add_definitions(-DPNG_MIPS_MSA_OPT=2)
	  list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_MIPS_MSA_OPT=2)
    endif()
  else()
    #add_definitions(-DPNG_MIPS_MSA_OPT=0)
	list(APPEND LIBPNG_PRIVATE_DEFINITIONS PNG_MIPS_MSA_OPT=0)
  endif()
endif()
endif(PNG_HARDWARE_OPTIMIZATIONS)


# OUR SOURCES
set(libpng_public_hdrs
  ${LIBPNG_ROOT}/png.h
  ${LIBPNG_ROOT}/pngconf.h
  ${LIBPNG_ROOT}/pnglibconf.h
)

set(libpng_private_hdrs
  ${LIBPNG_ROOT}/pngpriv.h
  ${LIBPNG_ROOT}/pngdebug.h
  ${LIBPNG_ROOT}/pnginfo.h
  ${LIBPNG_ROOT}/pngstruct.h
)

set(libpng_sources
  ${libpng_public_hdrs}
  ${libpng_private_hdrs}
  ${LIBPNG_ROOT}/png.c
  ${LIBPNG_ROOT}/pngerror.c
  ${LIBPNG_ROOT}/pngget.c
  ${LIBPNG_ROOT}/pngmem.c
  ${LIBPNG_ROOT}/pngpread.c
  ${LIBPNG_ROOT}/pngread.c
  ${LIBPNG_ROOT}/pngrio.c
  ${LIBPNG_ROOT}/pngrtran.c
  ${LIBPNG_ROOT}/pngrutil.c
  ${LIBPNG_ROOT}/pngset.c
  ${LIBPNG_ROOT}/pngtrans.c
  ${LIBPNG_ROOT}/pngwio.c
  ${LIBPNG_ROOT}/pngwrite.c
  ${LIBPNG_ROOT}/pngwtran.c
  ${LIBPNG_ROOT}/pngwutil.c
  ${libpng_arm_sources}
  ${libpng_intel_sources}
  ${libpng_mips_sources}
  ${libpng_powerpc_sources}
)

if(MSVC)
  #add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  list(APPEND LIBPNG_PRIVATE_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
  list(APPEND LIBPNG_PRIVATE_DEFINITIONS _CRT_NONSTDC_NO_DEPRECATE)
endif(MSVC)

add_library(libpng ${libpng_sources})
target_link_libraries(libpng PUBLIC zlib)
target_include_directories(libpng PUBLIC  ${LIBPNG_ROOT})
target_compile_definitions(libpng PRIVATE ${LIBPNG_PRIVATE_DEFINITIONS})