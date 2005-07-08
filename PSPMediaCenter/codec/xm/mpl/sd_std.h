#ifndef sd_std_h_n34782823478789234
#define sd_std_h_n34782823478789234

#include "../mpl/mx.h"

typedef struct mpl__sd_std_s {
    int (*mx_create) (mpl__mixer_t ** mixer);
    void (*mx_destroy) (mpl__mixer_t * mixer);

    mpl__mixer_t *mx[4];
} mpl__sd_std_t;

int mpl__sd_std_create(mpl__snd_dev_t ** sd, int (*mx_create) (mpl__mixer_t ** mixer),
		       void (*mx_destroy) (mpl__mixer_t * mixer));
void mpl__sd_std_destroy(mpl__snd_dev_t * sd);

#endif
