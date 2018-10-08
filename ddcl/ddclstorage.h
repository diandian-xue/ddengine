#pragma once

#include "ddclconfig.h"

typedef dduint32 ddcl_Handle;
struct tag_ddcl_Storage;
typedef struct tag_ddcl_Storage ddcl_Storage;

DDCLAPI ddcl_Storage *
ddcl_new_storage (unsigned ele_sz, unsigned init_count);

DDCLAPI void
ddcl_free_storage (ddcl_Storage * s);

DDCLAPI ddcl_Handle
ddcl_register_in_storage (ddcl_Storage * s, void ** p);

DDCLAPI void *
ddcl_find_in_storage (ddcl_Storage * hs, ddcl_Handle h);

DDCLAPI int
ddcl_del_in_storage (ddcl_Storage * hs, ddcl_Handle h);

DDCLAPI void
ddcl_expand_storage (ddcl_Storage * hs, dduint32 size);

DDCLAPI void
ddcl_begin_storage(ddcl_Storage * hs);

DDCLAPI int
ddcl_next_storage(ddcl_Storage * hs, ddcl_Handle * h, void ** p);
