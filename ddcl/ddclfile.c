#define DDCL_CORE

#include "ddclfile.h"
#include "ddclerr.h"

#include <memory.h>
#include <stdio.h>

static char _cwd_path[DDCL_MAX_PATH];

enum {
    FIND_START = 0,
    FIND_NAME,
    FIND_CUR,
    FIND_LAST,
    FIND_DIR,
};

DDCLAPI int
ddcl_file_module_init (ddcl * conf){
    if (DDCL_GETCWD(_cwd_path, DDCL_MAX_PATH)) {
        return 0;
    }else{
        return DDCL_FILE_PATH_ERR;
    }
}

static int
repair_fullpath (char * path){
    size_t len = strlen(path);
    if (len < 1)
        return DDCL_FILE_PATH_ERR;
    unsigned stat = 0;
    unsigned i = 0;
    unsigned pos = 0;
    char clear[DDCL_MAX_PATH] = { 0 };
    while (path[i]) {
        switch (path[i]) {
        case '.': {
            if (stat == FIND_START)
                stat = FIND_CUR;
            else if (stat == FIND_CUR)
                stat = FIND_LAST;
            else{
                clear[pos] = path[i]; pos++;
                stat = FIND_CUR;
            }
            break;
        }
        case '\\':
            if (stat == FIND_START)
                return DDCL_FILE_PATH_ERR;
        case '/': {
            if (stat == FIND_CUR || stat == FIND_DIR) {
                clear[pos] = 0;
                pos--;
                stat = FIND_NAME;
            }
            else if (stat == FIND_START){
                clear[pos] = DDCL_PATH_SPCH; pos++;
                stat = FIND_DIR;
            }
            else if (stat == FIND_LAST) {
                unsigned tpos = pos - 3;
                while (tpos > 0) {
                    if (clear[tpos] == DDCL_PATH_SPCH)
                        break;
                    tpos--;
                }
                pos = tpos + 1;
                clear[pos + 1] = 0;
                stat = FIND_DIR;
            }
            else{
                 clear[pos] = DDCL_PATH_SPCH; pos++;
                 stat = FIND_DIR;
            }
            break;
        }
        default: {
            if (stat == FIND_LAST)
                return DDCL_FILE_PATH_ERR;
            else if (stat == FIND_CUR) {
                clear[pos] = path[i]; pos++;
                stat = FIND_NAME;
            }
            else {
                clear[pos] = path[i]; pos++;
                stat = FIND_NAME;
            }
        }
        }
        i++;
    }
    clear[pos] = 0;
    memcpy(path, clear, pos + 1);
    return 0;
}

DDCLAPI int
ddcl_full_path (const char * path, char out[DDCL_MAX_PATH]){
    if (ddcl_path_absolute(path)) {
        memcpy(out, path, strlen(path) + 1);
        return repair_fullpath(out);;
    }
    else {
        sprintf(out, "%s" DDCL_PATH_SP "%s", _cwd_path, path);
        return repair_fullpath(out);;
    }
}

DDCLAPI const char *
ddcl_cwd (){
    return _cwd_path;
}

DDCLAPI int
ddcl_path_absolute (const char * path){
#ifdef DDSYS_WIN
    if ((strlen(path) > 2
        && ((path[0] >= 'a' && path[0] <= 'z') || (path[0] >= 'A' && path[0] <= 'Z'))
        && path[1] == ':') || (path[0] == '/' && path[1] == '/')){
        return 1;
    }
    return 0;
#else
    return path[0] == '/';
#endif
}

DDCLAPI int
ddcl_file_exist(const char * file) {
    struct stat s;
    if (stat(file, &s))
        return 0;
    if (!s.st_mode)
        return 0;
    return 1;
}

DDCLAPI int
ddcl_is_dir (const char * path){
    struct stat s;
    stat(path, &s);
    if (stat(path, &s))
        return 0;
    if (!s.st_mode)
        return 0;
    return (s.st_mode & DDCL_IFMT) == DDCL_IFDIR;
}

DDCLAPI int
ddcl_file_size (const char * path, size_t * size){
    struct stat s;
    if (stat(path, &s))
        return DDCL_FILE_NOT_EXIST;
    if (!s.st_mode)
        return DDCL_FILE_NOT_EXIST;
    if ((s.st_mode & DDCL_IFMT) == DDCL_IFDIR)
        return DDCL_FILE_CANNOT_BE_DIR;
    *size = s.st_size;
    return 0;
}


DDCLAPI void
ddcl_init_file_search (ddcl_FileSearch * fs){
    memset(fs, 0, sizeof(ddcl_FileSearch));
    memcpy(fs->paths[0], _cwd_path, strlen(_cwd_path));
    fs->count = 1;
}

DDCLAPI int
ddcl_add_path (ddcl_FileSearch * fs, const char * path){
    const char * fp = path;
    if (!ddcl_path_absolute(path)) {
        char fpbuff[DDCL_MAX_PATH];
        ddcl_full_path(path, fpbuff);
        fp = fpbuff;
    }

    struct stat s;
    if(stat(fp, &s))
        return DDCL_FILE_NOT_EXIST;
    if (!s.st_mode)
        return DDCL_FILE_NOT_EXIST;
    if ((s.st_mode & DDCL_IFMT) != DDCL_IFDIR)
        return DDCL_FILE_MUST_BE_DIR;

    memcpy(fs->paths[fs->count], fp, strlen(fp) + 1);
    fs->count ++;
    return 0;
}

DDCLAPI int
ddcl_find_file (ddcl_FileSearch * fs, const char * file, char out[DDCL_MAX_PATH]){
    if (ddcl_path_absolute(file)) {
        memcpy(out, file, strlen(file) + 1);
        return repair_fullpath(out);
    }

    struct stat s;
    for (int i = fs->count; i >0 ; i--) {
        sprintf(out, "%s" DDCL_PATH_SP "%s", fs->paths[i - 1], file);
        if(stat(out, &s))
            continue;
        if(s.st_mode)
            return repair_fullpath(out);
    }
    return DDCL_FILE_NOT_EXIST;
}


