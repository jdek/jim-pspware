#include "../mpl/string.h"

int mpl__string_cmp_char(char *str1, char *str2)
{
    while (*str2 != '\0' && *str1 == *str2) {
	str1++;
	str2++;
    }

    if (*str1 == *str2) {
	return MPL__STRING_EQUAL;
    }

    return MPL__STRING_UNEQUAL;
}

int mpl__string_cmp_wide_char(wide_char * str1, wide_char * str2)
{
    while (*str2 != '\0' && *str1 == *str2) {
	str1++;
	str2++;
    }

    if (*str1 == *str2) {
	return MPL__STRING_EQUAL;
    }

    return MPL__STRING_UNEQUAL;
}
