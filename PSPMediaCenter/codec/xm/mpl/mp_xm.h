#ifndef mp_xm_h_n42378789234789342
#define mp_xm_h_n42378789234789342

#include "../vbf/vbf_util.h"
#include "../mpl/sys/msg_box.h"
#include "../mpl/mp.h"

#define MPL__XM_FREQ_TABLE_LINEAR             1

#define MPL__XM_SMP_LOOP_BITS                 0x03

#define MPL__XM_SMP_NO_LOOP                   0
#define MPL__XM_SMP_LOOP                      1
#define MPL__XM_SMP_BIDI_LOOP                 2

#define MPL__XM_SMP_16BIT                     16

// vol/pan envelope
#define MPL__XM_ENV_ON                        1
#define MPL__XM_ENV_SUSTAIN                   2
#define MPL__XM_ENV_LOOP                      4

#define MPL__XM_NO_NOTE                       98
#define MPL__XM_KEY_OFF                       97

#define MPL__XM_EFF_ARPEGGIO                  0x00
#define MPL__XM_EFF_PORTA_UP                  0x01
#define MPL__XM_EFF_PORTA_DOWN                0x02
#define MPL__XM_EFF_PORTA                     0x03
#define MPL__XM_EFF_VIB                       0x04
#define MPL__XM_EFF_PORTA_VOL_SLIDE           0x05
#define MPL__XM_EFF_VIB_VOL_SLIDE             0x06
#define MPL__XM_EFF_TREMOLO                   0x07
#define MPL__XM_EFF_PAN                       0x08
#define MPL__XM_EFF_SMP_OFFSET                0x09
#define MPL__XM_EFF_VOL_SLIDE                 0x0A
#define MPL__XM_EFF_POS_JUMP                  0x0B
#define MPL__XM_EFF_VOL                       0x0C
#define MPL__XM_EFF_PAT_BREAK                 0x0D
#define MPL__XM_EFF_MOD_EXT                   0x0E
#define MPL__XM_EFF_SPEED                     0x0F
#define MPL__XM_EFF_GLOB_VOL                  0x10
#define MPL__XM_EFF_GLOB_VOL_SLIDE            0x11
#define MPL__XM_EFF_KEY_OFF                   0x14
#define MPL__XM_EFF_ENV_POS                   0x15
#define MPL__XM_EFF_PAN_SLIDE                 0x19
#define MPL__XM_EFF_RETRIG_VOL_SLIDE          0x1B
#define MPL__XM_EFF_TREMOR                    0x1D
#define MPL__XM_EFF_EXTRA_FINE_PORTA          0x21

// Exx effects
#define MPL__XM_EXT_EFF_FINE_PORTA_UP         0x10
#define MPL__XM_EXT_EFF_FINE_PORTA_DOWN       0x20
#define MPL__XM_EXT_EFF_GLISSANDO             0x30
#define MPL__XM_EXT_EFF_VIB_WAVE              0x40
#define MPL__XM_EXT_EFF_FINE_TUNE             0x50
#define MPL__XM_EXT_EFF_PAT_LOOP              0x60
#define MPL__XM_EXT_EFF_TREMOLO_WAVE          0x70
#define MPL__XM_EXT_EFF_PAN                   0x80
#define MPL__XM_EXT_EFF_RETRIG                0x90
#define MPL__XM_EXT_EFF_FINE_VOL_SLIDE_UP     0xA0
#define MPL__XM_EXT_EFF_FINE_VOL_SLIDE_DOWN   0xB0
#define MPL__XM_EXT_EFF_NOTE_CUT              0xC0
#define MPL__XM_EXT_EFF_NOTE_DELAY            0xD0
#define MPL__XM_EXT_EFF_PAT_DELAY             0xE0

#define MPL__XM_VOL_EFF_VOL_SLIDE_DOWN        0x60
#define MPL__XM_VOL_EFF_VOL_SLIDE_UP          0x70
#define MPL__XM_VOL_EFF_FINE_VOL_SLIDE_DOWN   0x80
#define MPL__XM_VOL_EFF_FINE_VOL_SLIDE_UP     0x90
#define MPL__XM_VOL_EFF_VIB_SPEED             0xA0
#define MPL__XM_VOL_EFF_VIB                   0xB0
#define MPL__XM_VOL_EFF_PAN                   0xC0
#define MPL__XM_VOL_EFF_PAN_SLIDE_LEFT        0xD0
#define MPL__XM_VOL_EFF_PAN_SLIDE_RIGHT       0xE0
#define MPL__XM_VOL_EFF_PORTA                 0xF0

#define MPL__XM_CTRL_VOL                      1
#define MPL__XM_CTRL_PER                      2
#define MPL__XM_CTRL_START                    4
#define MPL__XM_CTRL_STOP                     8

#define MPL__XM_PL_CTRL_BPM                   1
#define MPL__XM_PL_CTRL_VOL                   2

typedef struct {
    int index;

    char name[23];

    int loop_begin;
    int loop_end;

    int vol;
    int pan;

    int rel_note;
    int finetune;

    int length;
    int format;
    void *data;
} mpl__xm_sample_t;

typedef struct {
    int x, y;
} mpl__xm_point_t;

typedef struct {
    char name[23];

    int note2smp[96];

    mpl__xm_point_t vol_env[12];
    int vol_num;
    int vol_sus;
    int vol_loop_beg;
    int vol_loop_end;
    int vol_type;

    int vol_fade_out;

    mpl__xm_point_t pan_env[12];
    int pan_num;
    int pan_sus;
    int pan_loop_beg;
    int pan_loop_end;
    int pan_type;

    int vib_type;
    int vib_sweep;
    int vib_depth;
    int vib_rate;

    int smp_num;
    mpl__xm_sample_t *smp;
} mpl__xm_inst_t;

typedef struct {
    u8 key;
    u8 inst;
    u8 volume;
    u8 effect;
    u8 param;
} mpl__xm_cell_t;

typedef struct {
    char title[21];
    char tracker[21];

    int restart_pos;
    int length;
    int order[256];

    int inst_num;
    mpl__xm_inst_t *inst;
    int smp_index_num;

    int pat_num;
    mpl__xm_cell_t *pat[256];
    int pat_length[256];

    int ch_num;
    int bpm, speed;
    int freq_table;
} mpl__xm_t;

typedef struct {
    int active;
    int ctrl;
    int key;
    int real_key;
    int key_off;
    int eff;

    int inst;
    mpl__xm_inst_t *p_inst;
    mpl__xm_sample_t *p_smp;
//      double                  sample_pos;
    int smp_offset;

    int vol;
    int vol_delta;
    int env_vol;
    int env_vol_tick;
    int fade_out_vol;

    int pan;
    int pan_delta;
    int env_pan;
    int env_pan_tick;

    int per;
    int per_delta;
    int dst_per;

    int porta_period;
    int porta_speed;

    int vib_speed;
    int vib_depth;
    int vib_pos;

    int inst_vib_sweep_pos;
    int inst_vib_pos;

    int tremolo_speed;
    int tremolo_depth;
    int tremolo_pos;

    int vol_slide;
    int fvol_slide_up;
    int fvol_slide_down;
    int vib_vol_slide;
    int porta_vol_slide;
    int retrig_vol_slide;
    int pan_slide;

    int loop_row;
    int loop_num;

    int porta_up;
    int porta_down;
    int fporta_up;
    int fporta_down;
    int efporta_up;
    int efporta_down;

    int tremor_pos;
    int tremor_spd;

    int wave_control;
} mpl__mp_xm_chan_t;

typedef struct {
    mpl__xm_t *xm;

    int pat_cur;
    int pat_next;
    int row_cur;
    int row_next;

    int bpm;
    int speed;
    int tick_cur;
    int delay;
    int ctrl;
    int paused;

    mpl__mp_xm_chan_t ch[64];

    int smp_handle[300];
    int smp_fine_tune[300];

    float32 vol;
    int glob_vol, glob_vol_slide;

    int loop;

    mpl__snd_dev_t *dev;
    mpl__snd_mixer_t *mixer;
    mpl__msg_box_t *mb;
} mpl__mp_xm_t;

int mpl__mp_xm_construct(mpl__xm_t * xm, mpl__mp_t * mp, mpl__msg_box_t * mb);
int mpl__mp_xm_destruct(mpl__mp_t * mp);

#endif
