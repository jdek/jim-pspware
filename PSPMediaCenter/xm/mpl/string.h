#ifndef string_h_n42378789234
#define string_h_n42378789234

#include "../sys_cfg.h"

#define MPL__STRING_EQUAL     1
#define MPL__STRING_UNEQUAL   0

int mpl__string_cmp_char(char *str1, char *str2);
int mpl__string_cmp_wide_char(wide_char *str1, wide_char *str2);

#endif
