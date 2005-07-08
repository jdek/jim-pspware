#ifndef msg_box_h_n43789423798
#define msg_box_h_n43789423798

#include "../../sys_cfg.h"
#include "../../mpl/sys/critsect.h"
#include "../../mpl/error.h"

typedef struct mpl__msg_s {
    int msg;
    int param[8];
    void *data;
} mpl__msg_t;

typedef struct mpl__msg_box_s {
    mpl__msg_t *msg;
    int max;
    int cnt;
    int first;
    int last;
//      mpl__critical_section_t cs;
} mpl__msg_box_t;

int mpl__msg_box_create(mpl__msg_box_t ** mb, int max);
int mpl__msg_box_destroy(mpl__msg_box_t * mb);
int mpl__msg_box_construct(mpl__msg_box_t * mb, int max);
int mpl__msg_box_destruct(mpl__msg_box_t * mb);
int mpl__msg_box_send(mpl__msg_box_t * mb, mpl__msg_t * msg);
int mpl__msg_box_receive(mpl__msg_box_t * mb, mpl__msg_t * msg);

#endif
