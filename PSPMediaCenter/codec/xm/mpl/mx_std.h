#ifndef mx_std_h_n37842789342789
#define mx_std_h_37842789342789

// standard mixer

#include "../mpl/mx.h"

typedef struct {
    int exists;
    mpl__snd_mixer_smp_t smp;
    int sus_loop_beg;
    int sus_loop_end;
    int sus_loop_mode;
//      int                                     locked;
} mpl__mx_std_smp_t;

typedef struct {
    int playing;
    int smp;
    float32 patch[8];
    float32 vol_left, vol_right, vol_center;
    float32 vol_left_ramp, vol_right_ramp, vol_center_ramp;
    int vol_left_ramp_num, vol_right_ramp_num, vol_center_ramp_num;
    float32 vol_left_dst, vol_right_dst, vol_center_dst;
    float32 vol, pan_lr;
    float32 freq;
    float64 pos;
    int dir;
    int loop_beg;
    int loop_end;
    int loop_mode;
} mpl__mx_std_ch_t;

typedef struct {
    mpl__mx_std_smp_t *smp;
    int smp_num;
    mpl__mx_std_ch_t *ch;
    int ch_num;

    int latency;
    int paused;
    int mem_usage;
    int qual;
    mpl__snd_dev_output_t output;
    float32 vol_local;
    float32 pan_lr_local;
    float32 pan_fb_local;
    float32 vol_global;
    float32 pan_lr_global;
    float32 pan_fb_global;

    float32 ramp;
    float32 dc_spd[2], dc_pos[2];
//      float32                         quant[2];


    mpl__snd_mixer_t iface;
    mpl__snd_call_back_t call_back;
    float64 count_down_frac;
    int count_down;
} mpl__mx_std_t;

int mpl__mx_std_create(mpl__mixer_t ** mixer);
void mpl__mx_std_destroy(mpl__mixer_t * mixer);

#endif
