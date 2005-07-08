#ifndef critsect_h_n423784238
#define critsect_h_n423784238

// #include <windows.h>
#include "../../sys_cfg.h"
#include "../../mpl/error.h"

typedef int /*CRITICAL_SECTION */ mpl__critical_section_t;

int mpl__cs_create(mpl__critical_section_t ** cs);
int mpl__cs_destroy(mpl__critical_section_t * cs);
int mpl__cs_construct(mpl__critical_section_t * cs);
int mpl__cs_destruct(mpl__critical_section_t * cs);
int mpl__cs_enter(mpl__critical_section_t * cs);
int mpl__cs_leave(mpl__critical_section_t * cs);

#endif
