#pragma once

#include "ddclconfig.h"
#include "ddcl.h"
#include <sys/stat.h>

#ifdef DDSYS_WIN
#include <direct.h>
#include <Windows.h>

#define DDCL_GETCWD  _getcwd
#define DDCL_MAX_PATH MAX_PATH
#define DDCL_PATH_SP "\\"
#define DDCL_PATH_SPCH '\\'

#define DDCL_IFMT _S_IFMT
#define DDCL_IFDIR _S_IFDIR
#define DDCL_IFCHR _S_IFCHR
#define DDCL_IFIFO _S_IFIFO
#define DDCL_IFREG _S_IFREG
#define DDCL_IREAD _S_IREAD
#define DDCL_IWRITE _S_IWRITE
#define DDGL_IEXEC  _S_IEXEC

#else

#define DDCL_GETCWD(b,f) getcwd(b, f)
#define DDCL_MAX_PATH 255
#define DDCL_PATH_SP "/"
#define DDCL_PATH_SPCH '/'

#endif


#if defined(DDSYS_LINUX)
#include <unistd.h>
#define DDCL_IFMT __S_IFMT
#define DDCL_IFDIR __S_IFDIR
#define DDCL_IFCHR __S_IFCHR
#define DDCL_IFIFO _S_IFIFO
#define DDCL_IFREG __S_IFREG
#define DDCL_IREAD __S_IREAD
#define DDCL_IWRITE __S_IWRITE
#define DDCL_IEXEC  __S_IEXEC
#endif

#if defined(DDSYS_APPLE) || defined(DDSYS_ANDROID)
#define DDCL_IFMT S_IFMT
#define DDCL_IFDIR S_IFDIR
#define DDCL_IFCHR S_IFCHR
#define DDCL_IFIFO S_IFIFO
#define DDCL_IFREG S_IFREG
#define DDCL_IREAD S_IREAD
#define DDCL_IWRITE S_IWRITE
#define DDCL_IEXEC  S_IEXEC
#endif


DDCLAPI int
ddcl_file_module_init (ddcl * conf);

DDCLAPI int
ddcl_full_path (const char * path, char out[DDCL_MAX_PATH]);

DDCLAPI const char *
ddcl_cwd ();

DDCLAPI int
ddcl_path_absolute (const char * path);

DDCLAPI int
ddcl_file_exist (const char * path);

DDCLAPI int
ddcl_is_dir (const char * path);

DDCLAPI int
ddcl_file_size (const char * path, size_t * size);

// file search
typedef struct tag_ddcl_FileSearch{
    char paths[DDCL_MAX_PATH][255];
    unsigned count;
}ddcl_FileSearch;

DDCLAPI void
ddcl_init_file_search (ddcl_FileSearch * fs);

DDCLAPI int
ddcl_add_path (ddcl_FileSearch * fs, const char * path);

DDCLAPI int
ddcl_find_file (ddcl_FileSearch * fs, const char * f, char out[DDCL_MAX_PATH]);
