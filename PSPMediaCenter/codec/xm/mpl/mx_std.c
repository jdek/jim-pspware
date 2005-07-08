#include "../mpl/sys/mem.h"
#include "../mpl/mx_std.h"
#include "../mpl/string.h"
#include "../mpl/dsp_std.h"

// formats: mono/stereo, 8/16/24/32/float

// to do
// - better error checks
// - more mixer functions? (four channels, etc.)
// - lagrange interpolation?
// - 24 bit mixing
// - extensions: "EXT_FILTER EXT_REVERB EXT_CHORUS EXT_FX"
// - quality improvement


#define RAMPING 0.02f

#define CHECK_FOR_ERRORS

static int command_reset(void *internal);
static int command_pause(void *internal);
static int command_stop(void *internal);
static int command_upload_sample(void *internal, mpl__snd_mixer_smp_t sample);
static int command_destroy_sample(void *internal, int handle);
static int command_set_sustain_loop(mpl__snd_mixer_t * sndmx, int handle, int begin, int end, int mode);
static int command_end_sustain(mpl__snd_mixer_t * sndmx, int ch);
static int command_set_vol(void *internal, float32 vol, float32 pan_lr, float32 pan_fb);
static int command_get_vol(void *internal, float32 * vol, float32 * pan_lr, float32 * pan_fb);
static int command_get_output_options(void *internal, mpl__snd_dev_output_t * opt);
static int command_get_latency(void *internal);
static int command_get_num_channels(void *internal);
static int command_get_num_active_channels(void *internal);
static int command_get_free_channel(void *internal);
static int command_is_channel_active(void *internal, int ch);
static int command_get_free_channel(void *internal);
static int command_is_channel_active(void *internal, int ch);
static int command_set_channel_vol(void *internal, int ch, float32 vol, float32 pan_lr, float32 pan_fb);
static int command_set_channel_freq(void *internal, int ch, float32 freq);
static int command_set_channel_pos(void *internal, int ch, float64 pos, int dir);
static int command_play_channel(void *internal, int ch, int handle, float32 freq, float32 vol, float32 pan_lr,
				float32 pan_fb, float64 pos, int dir);
static int command_stop_channel(void *internal, int ch);
static int command_set_option(void *internal, char *option, char *value);
static int command_get_option(void *internal, char *option, char **value);
static int command_get_proc(void *internal, char *name, void_func_t * proc);
static int command_set_call_back(void *internal, mpl__snd_call_back_t call_back);

static int mixer_command_set_output_options(void *internal, mpl__snd_dev_output_t * opt, int latency);
static int mixer_command_set_options(void *internal, mpl__mixer_opt_t * opt);
static int mixer_command_get_options(void *internal, mpl__mixer_opt_t * opt);
static int mixer_command_get_proc(void *internal, char *name, void_func_t * func);
static int mixer_command_set_vol(void *internal, float32 vol, float32 pan_lr, float32 pan_fb);
static int mixer_command_mix(void *internal, void *buffer, int length);
static int mixer_command_get_num_active_channels(void *internal);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// no interpolation
//#define __INTERPOLATE__(valm1, val0, val1, val2, t) (val0)
// linear interpolation
//#define __INTERPOLATE__(valm1, val0, val1, val2, t) (val0 + (val1 - val0) * t)
// spline interpolation
#define __INTERPOLATE__(valm1, val0, val1, val2, t) (((((3.0f * (val0 - val1) - valm1 + val2) * 0.5f) * t + (2.0f * val1 + valm1 - (5.0f * val0 + val2) * 0.5f)) * t + ((val1 - valm1) * 0.5f)) * t + val0)
#define __RAMP___(a,b,c, d) {if (b) {a+=c;b--;if(!b) a=d;}}
#define __RAMP__ {__RAMP___(p_ch->vol_left, p_ch->vol_left_ramp_num, p_ch->vol_left_ramp, p_ch->vol_left_dst) __RAMP___(p_ch->vol_right, p_ch->vol_right_ramp_num, p_ch->vol_right_ramp, p_ch->vol_right_dst) __RAMP___(p_ch->vol_center, p_ch->vol_center_ramp_num, p_ch->vol_center_ramp, p_ch->vol_center_dst)}

void mix_inner_loop(mpl__mx_std_t * mx, int ch, float64 step, float32 * buf, int pos, int num)
{
    mpl__mx_std_smp_t *p_smp;
    mpl__mx_std_ch_t *p_ch;
    float32 *smp, tmp, pos_fract;
    int pos_int;

    p_ch = mx->ch + ch;
    p_smp = mx->smp + p_ch->smp;
    smp = (float32 *) p_smp->smp.data;

    switch (2 * (mx->output.channels - 1) + p_smp->smp.channels) {
    case 1:
	while (num--) {
	    pos_int = (int) p_ch->pos;
	    pos_fract = (float32) (p_ch->pos - pos_int);

	    buf[pos++] += __INTERPOLATE__(smp[pos_int], smp[pos_int + 1],
					  smp[pos_int + 2], smp[pos_int + 3], pos_fract) * p_ch->vol_center;

	    __RAMP__ p_ch->pos += step;
	}
	break;
    case 2:
	while (num--) {
	    pos_int = (int) p_ch->pos;
	    pos_fract = (float32) (p_ch->pos - pos_int);
	    pos_int <<= 1;

	    buf[pos] += __INTERPOLATE__(smp[pos_int], smp[pos_int + 2],
					smp[pos_int + 4], smp[pos_int + 6], pos_fract) * p_ch->vol_left;

	    buf[pos++] += __INTERPOLATE__(smp[pos_int + 1], smp[pos_int + 3],
					  smp[pos_int + 5], smp[pos_int + 7], pos_fract) * p_ch->vol_right;

	    __RAMP__ p_ch->pos += step;
	}
	break;
    case 3:
	pos <<= 1;

	while (num--) {
	    pos_int = (int) p_ch->pos;
	    pos_fract = (float32) (p_ch->pos - pos_int);

	    tmp = __INTERPOLATE__(smp[pos_int], smp[pos_int + 1], smp[pos_int + 2], smp[pos_int + 3], pos_fract);

	    buf[pos++] += tmp * p_ch->vol_left;
	    buf[pos++] += tmp * p_ch->vol_right;

	    __RAMP__ p_ch->pos += step;
	}
	break;
    case 4:
	pos <<= 1;

	while (num--) {
	    pos_int = (int) p_ch->pos;
	    pos_fract = (float32) (p_ch->pos - pos_int);
	    pos_int <<= 1;

	    buf[pos++] += __INTERPOLATE__(smp[pos_int], smp[pos_int + 2],
					  smp[pos_int + 4], smp[pos_int + 6], pos_fract) * p_ch->vol_left;

	    buf[pos++] += __INTERPOLATE__(smp[pos_int + 1], smp[pos_int + 3],
					  smp[pos_int + 5], smp[pos_int + 7], pos_fract) * p_ch->vol_right;

	    __RAMP__ p_ch->pos += step;
	}
	break;
    }
}

#undef __INTERPOLATE__
#undef __RAMP___
#undef __RAMP__

static void patch_smp(mpl__mx_std_ch_t * p_ch, mpl__mx_std_smp_t * p_smp)
{
    float32 *data;

    if (p_ch->loop_mode != MPL__SND_DEV_FMT_LOOPING) {
	return;
    }

    data = (float32 *) p_smp->smp.data;

    if (p_ch->pos < p_ch->loop_beg) {
	switch (p_smp->smp.channels) {
	case 1:
	    p_ch->patch[0] = data[p_ch->loop_beg];
	    break;
	case 2:
	    p_ch->patch[0] = data[p_ch->loop_beg];
	    p_ch->patch[1] = data[p_ch->loop_beg + 1];
	    break;
	}
    } else {
	switch (p_smp->smp.channels) {
	case 1:
	    p_ch->patch[0] = data[p_ch->loop_beg];

	    data[p_ch->loop_beg] = data[p_ch->loop_end];
	    break;
	case 2:
	    p_ch->patch[0] = data[p_ch->loop_beg];
	    p_ch->patch[1] = data[p_ch->loop_beg + 1];

	    data[p_ch->loop_beg] = data[p_ch->loop_end];
	    data[p_ch->loop_beg + 1] = data[p_ch->loop_end + 1];
	    break;
	}
    }

    switch (p_smp->smp.channels) {
    case 1:
	p_ch->patch[1] = data[p_ch->loop_end + 1];
	p_ch->patch[2] = data[p_ch->loop_end + 2];
	p_ch->patch[3] = data[p_ch->loop_end + 3];

	data[p_ch->loop_end + 1] = data[p_ch->loop_beg + 1];
	data[p_ch->loop_end + 2] = data[p_ch->loop_beg + 2];
	data[p_ch->loop_end + 3] = data[p_ch->loop_beg + 3];
	break;
    case 2:
	p_ch->patch[2] = data[p_ch->loop_end * 2 + 2];
	p_ch->patch[3] = data[p_ch->loop_end * 2 + 3];
	p_ch->patch[4] = data[p_ch->loop_end * 2 + 4];
	p_ch->patch[5] = data[p_ch->loop_end * 2 + 5];
	p_ch->patch[6] = data[p_ch->loop_end * 2 + 6];
	p_ch->patch[7] = data[p_ch->loop_end * 2 + 7];

	data[p_ch->loop_end * 2 + 2] = data[p_ch->loop_beg * 2 + 2];
	data[p_ch->loop_end * 2 + 3] = data[p_ch->loop_beg * 2 + 3];
	data[p_ch->loop_end * 2 + 4] = data[p_ch->loop_beg * 2 + 4];
	data[p_ch->loop_end * 2 + 5] = data[p_ch->loop_beg * 2 + 5];
	data[p_ch->loop_end * 2 + 4] = data[p_ch->loop_beg * 2 + 6];
	data[p_ch->loop_end * 2 + 5] = data[p_ch->loop_beg * 2 + 7];
	break;
    }
}

static void undo_patch(mpl__mx_std_ch_t * p_ch, mpl__mx_std_smp_t * p_smp)
{
    float32 *data;

    if (p_ch->loop_mode != MPL__SND_DEV_FMT_LOOPING) {
	return;
    }

    data = (float32 *) p_smp->smp.data;

    switch (p_smp->smp.channels) {
    case 1:
	data[p_ch->loop_beg] = p_ch->patch[0];
	data[p_ch->loop_end + 1] = p_ch->patch[1];
	data[p_ch->loop_end + 2] = p_ch->patch[2];
	data[p_ch->loop_end + 3] = p_ch->patch[3];
	break;
    case 2:
	data[p_ch->loop_beg * 2] = p_ch->patch[0];
	data[p_ch->loop_beg * 2] = p_ch->patch[1];
	data[p_ch->loop_end * 2 + 2] = p_ch->patch[2];
	data[p_ch->loop_end * 2 + 3] = p_ch->patch[3];
	data[p_ch->loop_end * 2 + 4] = p_ch->patch[4];
	data[p_ch->loop_end * 2 + 5] = p_ch->patch[5];
	data[p_ch->loop_end * 2 + 6] = p_ch->patch[6];
	data[p_ch->loop_end * 2 + 7] = p_ch->patch[7];
	break;
    }
}

static int calc_samples_left(mpl__mx_std_ch_t * p_ch, float64 fratio, float64 oofratio)
{
    int tmp;

    if (p_ch->dir == MPL__SND_DEV_DIR_FORWARDS) {
	tmp = (int) ((p_ch->loop_end - p_ch->pos) * oofratio);

	if (p_ch->loop_mode != MPL__SND_DEV_FMT_BIDILOOPING) {
	    if (tmp * fratio + p_ch->pos < p_ch->loop_end) {
		tmp++;
	    }
	}
    } else {
	if (p_ch->pos < p_ch->loop_beg) {
	    tmp = (int) (p_ch->pos * oofratio);
	} else {
	    tmp = (int) ((p_ch->pos - p_ch->loop_beg) * oofratio);
	}
    }

    if (tmp < 0) {
	tmp = 0;
    }

    return tmp;
}

static int fix_loop(mpl__mx_std_ch_t * p_ch, float64 * step, float64 fratio, float64 oofratio)
{
    if (p_ch->loop_mode == MPL__SND_DEV_FMT_NONLOOPING) {
	return 0;
    }

    if (p_ch->loop_mode == MPL__SND_DEV_FMT_LOOPING) {
	while (!calc_samples_left(p_ch, fratio, oofratio)) {
	    p_ch->pos -= p_ch->loop_end - p_ch->loop_beg;
	}

	if ((int) p_ch->pos < 0) {

	    p_ch->pos += fratio;
	}

	return 1;
    }

    if (p_ch->loop_mode == MPL__SND_DEV_FMT_BIDILOOPING) {
	*step = -*step;
	p_ch->dir = (~p_ch->dir) & 1;

	return 1;
    }

    return 0;
}

#define PAN2VOL_LEFT(pan) ((pan) <= 0.5f ? 1.0f : (1.0f - (pan)) * 2.0f)
#define PAN2VOL_RIGHT(pan) ((pan) >= 0.5f ? 1.0f : (pan) * 2.0f)

static void mix_channel(mpl__mx_std_t * mx, int ch, float32 * buf, i32 buf_pos, i32 num)
{
    i32 samples_left, cnt;
    float64 fratio, oofratio, step;
    float32 vol;
    mpl__mx_std_smp_t *p_smp;
    mpl__mx_std_ch_t *p_ch;

    p_ch = mx->ch + ch;
    p_smp = mx->smp + p_ch->smp;

    fratio = p_ch->freq / mx->output.freq;
    oofratio = 1 / fratio;

    step = fratio * (p_ch->dir == MPL__SND_DEV_DIR_FORWARDS ? 1 : -1);

    if (ch < mx->ch_num) {
	vol = p_smp->smp.vol * p_ch->vol * mx->vol_global * mx->vol_local;
	p_ch->vol_left_dst =
	    vol * PAN2VOL_LEFT(p_ch->pan_lr + p_smp->smp.pan_lr + mx->pan_lr_global + mx->pan_lr_local - 1.5f);
	p_ch->vol_right_dst =
	    vol * PAN2VOL_RIGHT(p_ch->pan_lr + p_smp->smp.pan_lr + mx->pan_lr_global + mx->pan_lr_local - 1.5f);
	p_ch->vol_center_dst = vol;
    } else {
	p_ch->vol_left_dst = 0;
	p_ch->vol_right_dst = 0;
	p_ch->vol_center_dst = 0;
    }

    if (p_ch->vol_left_dst != p_ch->vol_left) {
	if (p_ch->vol_left_dst > p_ch->vol_left) {
	    p_ch->vol_left_ramp = mx->ramp;
	    p_ch->vol_left_ramp_num = (int) ((p_ch->vol_left_dst - p_ch->vol_left) / mx->ramp);
	} else {
	    p_ch->vol_left_ramp = -mx->ramp;
	    p_ch->vol_left_ramp_num = (int) ((p_ch->vol_left - p_ch->vol_left_dst) / mx->ramp);
	}
    }

    if (p_ch->vol_right_dst != p_ch->vol_right) {
	if (p_ch->vol_right_dst > p_ch->vol_right) {
	    p_ch->vol_right_ramp = mx->ramp;
	    p_ch->vol_right_ramp_num = (int) ((p_ch->vol_right_dst - p_ch->vol_right) / mx->ramp);
	} else {
	    p_ch->vol_right_ramp = -mx->ramp;
	    p_ch->vol_right_ramp_num = (int) ((p_ch->vol_right - p_ch->vol_right_dst) / mx->ramp);
	}
    }

    if (p_ch->vol_center_dst != p_ch->vol_center) {
	if (p_ch->vol_center_dst > p_ch->vol_center) {
	    p_ch->vol_center_ramp = mx->ramp;
	    p_ch->vol_center_ramp_num = (int) ((p_ch->vol_center_dst - p_ch->vol_center) / mx->ramp);
	} else {
	    p_ch->vol_center_ramp = -mx->ramp;
	    p_ch->vol_center_ramp_num = (int) ((p_ch->vol_center - p_ch->vol_center_dst) / mx->ramp);
	}
    }

    cnt = num;

    while (cnt) {
	samples_left = calc_samples_left(p_ch, fratio, oofratio);

	if (samples_left == 0) {
	    if (!fix_loop(p_ch, &step, fratio, oofratio)) {
		p_ch->playing = 0;

		return;
	    }

	    samples_left = calc_samples_left(p_ch, fratio, oofratio);
	}

	if (samples_left == 0) {
	    p_ch->playing = 0;

	    return;
	}

	samples_left = samples_left < cnt ? samples_left : cnt;

	if (samples_left < 0) {
	    samples_left = 0;
	}

	patch_smp(p_ch, p_smp);
	mix_inner_loop(mx, ch, step, buf, buf_pos, samples_left);
	undo_patch(p_ch, p_smp);

	if (p_ch->vol_left == 0 && p_ch->vol_right == 0 && ch >= mx->ch_num) {
	    p_ch->playing = 0;

	    return;
	}

	cnt -= samples_left;
	buf_pos += samples_left;
    }
}

static void filter_dc(mpl__mx_std_t * mx, float32 * buf, int length)
{
    if (mx->output.channels > 1) {
	while (length--) {
	    mx->dc_spd[0] = mx->dc_spd[0] + (*buf - mx->dc_pos[0]) * 0.000004567f;
	    mx->dc_pos[0] = mx->dc_pos[0] + mx->dc_spd[0];
	    *buf = *buf - mx->dc_pos[0];
	    buf++;
	    mx->dc_spd[1] = mx->dc_spd[1] + (*buf - mx->dc_pos[1]) * 0.000004567f;
	    mx->dc_pos[1] = mx->dc_pos[1] + mx->dc_spd[1];
	    *buf = *buf - mx->dc_pos[1];
	    buf++;
	}
    } else {
	while (length--) {
	    mx->dc_spd[0] = mx->dc_spd[0] + (*buf - mx->dc_pos[0]) * 0.000004567f;
	    mx->dc_pos[0] = mx->dc_pos[0] + mx->dc_spd[0];
	    *buf = *buf - mx->dc_pos[0];
	    buf++;
	}

	mx->dc_spd[1] = mx->dc_spd[0];
	mx->dc_pos[1] = mx->dc_pos[0];
    }
}

#undef PAN2VOL_LEFT
#undef PAN2VOL_RIGHT

static void quantize(float32 * src, void *dest, int length, int format)
{
    i32 tmp_i32;
    i64 tmp_i64;


    switch (format) {
    case MPL__SND_DEV_FMT_8BIT:
	while (length--) {
	    tmp_i32 = (i32) (src[length] * 128.0f);

	    if (tmp_i32 < -128)
		tmp_i32 = -125;
	    if (tmp_i32 > 127)
		tmp_i32 = 125;

	    ((i8 *) dest)[length] = (i8) tmp_i32;
	}
	break;
    case MPL__SND_DEV_FMT_16BIT:
	while (length--) {
	    tmp_i32 = (i32) (src[length] * 32768.0f);

	    if (tmp_i32 < -32768)
		tmp_i32 = -32760;
	    if (tmp_i32 > 32767)
		tmp_i32 = 32760;

	    ((i16 *) dest)[length] = (i16) tmp_i32;
	}
	break;
    case MPL__SND_DEV_FMT_24BIT:
	// to do
	break;
    case MPL__SND_DEV_FMT_32BIT:
	while (length--) {
	    tmp_i64 = (i64) (src[length] * 2147483648.0f);

	    if (tmp_i64 < -(i64) 2147483648)
		tmp_i64 = -(i64) 2147483640;
	    if (tmp_i64 > 2147483647)
		tmp_i64 = 2147483640;

	    ((i32 *) dest)[length] = (i32) tmp_i64;
	}
	break;
    case MPL__SND_DEV_FMT_FLOAT_32BIT:
	while (length--) {
	    ((float32 *) dest)[length] = src[length];
	}
	break;
    }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int command_reset(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    int smp, ch;

    for (ch = 0; ch < mx->ch_num * 2; ch++) {
	mx->ch[ch].playing = 0;
    }

    for (smp = 0; smp < mx->smp_num; smp++) {
	if (mx->smp[smp].smp.data) {
	    mpl__mem_free(mx->smp[smp].smp.data);
	}

	mx->smp[smp].exists = 0;
    }

    mx->paused = 0;

    return 0;
}

static int command_pause(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    mx->paused ^= 1;

    return 0;
}

static int command_stop(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    int ch;

    for (ch = 0; ch < mx->ch_num; ch++) {
	mx->ch[ch + mx->ch_num] = mx->ch[ch];
	mx->ch[ch].playing = 0;
    }

    return 0;
}

static int command_upload_sample(void *internal, mpl__snd_mixer_smp_t sample)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    int smp, size;
    float32 *data;

    for (smp = 0; smp < mx->smp_num; smp++) {
	if (!mx->smp[smp].exists)
	    break;
    }

    if (smp == mx->smp_num) {
	return MPL__ERR_GENERIC;
    }
#ifdef CHECK_FOR_ERRORS

    if (sample.channels != 1 && sample.channels != 2) {
	return MPL__ERR_BAD_PARAM;
    }

    if (sample.format < MPL__SND_DEV_FMT_8BIT || sample.format > MPL__SND_DEV_FMT_FLOAT_32BIT) {
	return MPL__ERR_BAD_PARAM;
    }

    if (sample.loopmode != MPL__SND_DEV_FMT_NONLOOPING &&
	sample.loopmode != MPL__SND_DEV_FMT_LOOPING && sample.loopmode != MPL__SND_DEV_FMT_BIDILOOPING) {
	return MPL__ERR_BAD_PARAM;
    }

    if (sample.length == 0) {
	return MPL__ERR_BAD_PARAM;
    }

    if (sample.loopmode != MPL__SND_DEV_FMT_NONLOOPING && sample.loop >= sample.length) {
	return MPL__ERR_BAD_PARAM;
    }

    if (sample.pan_fb < 0.0f || sample.pan_fb > 1.0f || sample.pan_lr < 0.0f || sample.pan_fb > 1.0f) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    size = sizeof(float32) * sample.channels;

    mx->smp[smp].smp = sample;

    mx->smp[smp].sus_loop_beg = sample.loop;
    mx->smp[smp].sus_loop_end = sample.length;
    mx->smp[smp].sus_loop_mode = sample.loopmode;

    if (mpl__mem_alloc(size * (sample.length + 4), &mx->smp[smp].smp.data) <= MPL__ERR_GENERIC) {
	return MPL__ERR_NOMEM;
    }

    data = (float32 *) mx->smp[smp].smp.data;

    if (mpl__dsp_conv_format(sample.data, data + sample.channels,
			     sample.format, MPL__SND_DEV_FMT_FLOAT_32BIT, sample.length,
			     sample.channels, 1.0f) < MPL__ERR_OK) {
	mpl__mem_free((void *) data);

	return MPL__ERR_GENERIC;
    }


    switch (sample.channels) {
    case 1:
	data[0] = data[1];
	data[sample.length + 1] = data[sample.length];
	data[sample.length + 2] = data[sample.length];
	data[sample.length + 3] = data[sample.length];
	break;
    case 2:
	data[0] = data[2];
	data[1] = data[3];
	data[sample.length * 2 + 2] = data[sample.length * 2];
	data[sample.length * 2 + 3] = data[sample.length * 2 + 1];
	data[sample.length * 2 + 4] = data[sample.length * 2];
	data[sample.length * 2 + 5] = data[sample.length * 2 + 1];
	data[sample.length * 2 + 6] = data[sample.length * 2];
	data[sample.length * 2 + 7] = data[sample.length * 2 + 1];
	break;
    }

    mx->smp[smp].exists = 1;

    return smp;
}

static int command_destroy_sample(void *internal, int handle)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    int ch;

#ifdef CHECK_FOR_ERRORS

    if (handle < 0 || handle >= mx->smp_num) {
	return MPL__ERR_BAD_PARAM;
    }

    if (!mx->smp[handle].exists) {
	return MPL__ERR_BAD_PARAM;
    }

    for (ch = 0; ch < mx->ch_num * 2; ch++) {
	if (mx->ch[ch].playing && mx->ch[ch].smp == handle) {
	    return MPL__ERR_BAD_PARAM;
	}
    }
#endif

    mpl__mem_free(mx->smp[handle].smp.data);

    mx->smp[handle].exists = 0;

    return 0;
}

static int command_set_sustain_loop(mpl__snd_mixer_t * sndmx, int handle, int begin, int end, int mode)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) sndmx->internal_data;
    mpl__mx_std_smp_t *smp;

#ifdef CHECK_FOR_ERRORS

    if (handle < 0 || handle >= mx->smp_num) {
	return MPL__ERR_BAD_PARAM;
    }

    smp = mx->smp + handle;

    if (!smp->exists) {
	return MPL__ERR_BAD_PARAM;
    }

    if (end > smp->smp.length) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    smp->sus_loop_beg = begin;
    smp->sus_loop_end = end;
    smp->sus_loop_mode = mode;

    return 0;
}

static int command_end_sustain(mpl__snd_mixer_t * sndmx, int ch)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) sndmx->internal_data;
    mpl__mx_std_smp_t *smp;

#ifdef CHECK_FOR_ERRORS

    if (ch < 0 || ch >= mx->ch_num) {
	return MPL__ERR_BAD_PARAM;
    }

    if (!mx->ch[ch].playing) {
	return MPL__ERR_GENERIC;
    }

    smp = &mx->smp[mx->ch[ch].smp];

#endif

    mx->ch[ch].loop_beg = smp->smp.loop;
    mx->ch[ch].loop_end = smp->smp.length;
    mx->ch[ch].loop_mode = smp->smp.loopmode;

    return 0;
}

static int command_set_vol(void *internal, float32 vol, float32 pan_lr, float32 pan_fb)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS
    if (pan_lr < 0.0f || pan_lr > 1.0f || pan_fb < 0.0f || pan_fb > 1.0f) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->vol_local = vol;
    mx->pan_lr_local = pan_lr;
    mx->pan_fb_local = pan_fb;

    return 0;
}

static int command_get_vol(void *internal, float32 * vol, float32 * pan_lr, float32 * pan_fb)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    *vol = mx->vol_local;
    *pan_lr = mx->pan_lr_local;
    *pan_fb = mx->pan_fb_local;

    return 0;
}

static int command_get_output_options(void *internal, mpl__snd_dev_output_t * opt)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    *opt = mx->output;

    return 0;
}

static int command_get_latency(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    return mx->latency;
}

static int command_get_num_channels(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    return mx->ch_num;
}

static int command_get_num_active_channels(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    int ch, num;

    num = 0;

    for (ch = 0; ch < mx->ch_num; ch++) {
	if (mx->ch[ch].playing) {
	    num++;
	}
    }

    return num;
}

static int command_get_free_channel(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    int ch;

    for (ch = 0; ch < mx->ch_num; ch++) {
	if (!mx->ch[ch].playing) {
	    return ch;
	}
    }

    return MPL__ERR_GENERIC;
}

static int command_is_channel_active(void *internal, int ch)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS

    if (ch < 0 || ch >= mx->ch_num) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    return mx->ch[ch].playing != 0;
}

static int command_set_channel_vol(void *internal, int ch, float32 vol, float32 pan_lr, float32 pan_fb)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS

    if (ch < 0 || ch >= mx->ch_num) {
	return MPL__ERR_BAD_PARAM;
    }

    if (!mx->ch[ch].playing) {
	return MPL__ERR_BAD_PARAM;
    }

    if (pan_lr < 0.0f || pan_lr > 1.0f || pan_fb < 0.0f || pan_fb > 1.0f) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->ch[ch].vol = vol;
    mx->ch[ch].pan_lr = pan_lr;

    return 0;
}

static int command_set_channel_freq(void *internal, int ch, float32 freq)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS

    if (ch < 0 || ch >= mx->ch_num) {
	return MPL__ERR_BAD_PARAM;
    }

    if (!mx->ch[ch].playing) {
	return MPL__ERR_BAD_PARAM;
    }

    if (freq <= 0 || freq > 1000000) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->ch[ch].freq = freq;

    return -1;
}

static int command_set_channel_pos(void *internal, int ch, float64 pos, int dir)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS

    if (ch < 0 || ch >= mx->ch_num) {
	return MPL__ERR_BAD_PARAM;
    }

    if (!mx->ch[ch].playing) {
	return MPL__ERR_BAD_PARAM;
    }

    if (dir != MPL__SND_DEV_DIR_FORWARDS || dir != MPL__SND_DEV_DIR_BACKWARDS) {
	return MPL__ERR_BAD_PARAM;
    }

    if (pos < 0 || pos >= mx->ch[ch].loop_end) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->ch[ch + mx->ch_num] = mx->ch[ch];

    mx->ch[ch].pos = pos;
    mx->ch[ch].dir = dir;
    mx->ch[ch].vol_left = 0;
    mx->ch[ch].vol_right = 0;
    mx->ch[ch].vol_center = 0;


    return 0;
}

static int
command_play_channel(void *internal, int ch, int handle, float32 freq, float32 vol, float32 pan_lr, float32 pan_fb,
		     float64 pos, int dir)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    mpl__mx_std_smp_t *smp;

#ifdef CHECK_FOR_ERRORS

    if (ch < 0 || ch >= mx->ch_num) {
	return MPL__ERR_BAD_PARAM;
    }

    if (handle < 0 || handle >= mx->smp_num) {
	return MPL__ERR_BAD_PARAM;
    }

    smp = mx->smp + handle;

    if (!smp->exists) {
	return MPL__ERR_BAD_PARAM;
    }

    if (freq != -1 && (freq < 1 || freq > 1000000)) {
	return MPL__ERR_BAD_PARAM;
    }

    if (dir != -1 && dir != MPL__SND_DEV_DIR_FORWARDS && dir != MPL__SND_DEV_DIR_BACKWARDS) {
	return MPL__ERR_BAD_PARAM;
    }

    if (pan_lr < 0.0f || pan_lr > 1.0f || pan_fb < 0.0f || pan_fb > 1.0f) {
	return MPL__ERR_BAD_PARAM;
    }

    if (pos < 0 || pos >= smp->sus_loop_end) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->ch[ch + mx->ch_num] = mx->ch[ch];

    mx->ch[ch].pos = pos;
    mx->ch[ch].dir = dir;
    mx->ch[ch].smp = handle;
    mx->ch[ch].freq = freq == -1 ? mx->smp[handle].smp.freq : freq;
    mx->ch[ch].vol = vol;
    mx->ch[ch].pan_lr = pan_lr;
    mx->ch[ch].vol_left = 0;
    mx->ch[ch].vol_right = 0;
    mx->ch[ch].vol_center = 0;
    mx->ch[ch].loop_beg = smp->sus_loop_beg;
    mx->ch[ch].loop_end = smp->sus_loop_end;
    mx->ch[ch].loop_mode = smp->sus_loop_mode;
    mx->ch[ch].playing = 1;

    return 0;
}

static int command_stop_channel(void *internal, int ch)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS

    if (ch < 0 || ch >= mx->ch_num) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->ch[ch + mx->ch_num] = mx->ch[ch];
    mx->ch[ch].playing = 0;

    return 0;
}

static int command_set_option(void *internal, char *option, char *value)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    return MPL__ERR_BAD_PARAM;
}

static int command_get_option(void *internal, char *option, char **value)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    if (mpl__string_cmp_char(option, "ID") == MPL__STRING_EQUAL) {
	*value = "standard mixer";

	return 0;
    }

    if (mpl__string_cmp_char(option, "VERSION") == MPL__STRING_EQUAL) {
	*value = "0.01";

	return 0;
    }

    if (mpl__string_cmp_char(option, "AUTHOR") == MPL__STRING_EQUAL) {
	*value = "";

	return 0;
    }

    if (mpl__string_cmp_char(option, "EXTENSIONS") == MPL__STRING_EQUAL) {
	*value = "EXT_SUSTAIN_LOOP";

	return 0;
    }

    return MPL__ERR_BAD_PARAM;
}

static int command_get_proc(void *internal, char *name, void_func_t * proc)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    if (mpl__string_cmp_char(name, MPL__FUNC_SND_MIXER_SET_SUSTAIN_LOOP) == MPL__STRING_EQUAL) {
	*proc = (void_func_t) command_set_sustain_loop;
	return 1;
    }

    if (mpl__string_cmp_char(name, MPL__FUNC_SND_MIXER_END_SUSTAIN) == MPL__STRING_EQUAL) {
	*proc = (void_func_t) command_end_sustain;

	return 0;
    }

    return MPL__ERR_BAD_PARAM;
}

static int command_set_call_back(void *internal, mpl__snd_call_back_t call_back)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    mx->call_back = call_back;

    mx->count_down_frac += mx->call_back.period * mx->output.freq * 0.001;
    mx->count_down = (int) (mx->count_down_frac);
    mx->count_down_frac -= mx->count_down;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static int mixer_command_set_output_options(void *internal, mpl__snd_dev_output_t * opt, int latency)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS

    if (opt->channels != 1 && opt->channels != 2) {
	return MPL__ERR_BAD_PARAM;
    }

    if (opt->format < MPL__SND_DEV_FMT_8BIT || opt->format > MPL__SND_DEV_FMT_FLOAT_32BIT) {
	return MPL__ERR_BAD_PARAM;
    }

    if (opt->latency != MPL__SND_DEV_LATENCY_HIGH && opt->latency != MPL__SND_DEV_LATENCY_HIGH) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->dc_spd[0] = 0;
    mx->dc_pos[0] = 0;
    mx->dc_spd[1] = 0;
    mx->dc_pos[1] = 0;

    mx->output = *opt;
    mx->latency = latency;

    mx->ramp = 1.0f / (opt->freq * RAMPING);

    mx->count_down = (int) (mx->count_down * opt->freq / mx->output.freq);

    return 0;
}

static int mixer_command_set_options(void *internal, mpl__mixer_opt_t * opt)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    mpl__mx_std_ch_t *ch = 0;
    mpl__mx_std_smp_t *smp = 0;

    if (opt->ch_num != -1) {
	int cur;

	if (opt->ch_num < 1 || opt->ch_num > 256) {
	    return MPL__ERR_BAD_PARAM;
	}

	for (cur = 0; cur < mx->ch_num; cur++) {
	    if (mx->ch[cur].playing) {
		return MPL__ERR_GENERIC;
	    }
	}

    }

    if (opt->smp_num != -1) {
	int cur;

	if (opt->smp_num < 1 || opt->smp_num > 256) {
	    return MPL__ERR_BAD_PARAM;
	}

	for (cur = 0; cur < mx->smp_num; cur++) {
	    if (mx->smp[cur].exists) {
		return MPL__ERR_GENERIC;
	    }
	}
    }

    if (opt->mem_usage != -1) {
	if (opt->mem_usage != MPL__MX_MEM_AGGRESSIVE && opt->mem_usage != MPL__MX_MEM_LOW) {
	    return MPL__ERR_BAD_PARAM;
	}
    }

    if (opt->qual != -1) {
	if (opt->qual != MPL__MX_HQ && opt->qual != MPL__MX_LQ) {
	    return MPL__ERR_BAD_PARAM;
	}

    }

    if (opt->smp_num != -1) {
	if (mpl__mem_alloc(opt->smp_num * sizeof(mpl__mx_std_smp_t), (void **) &smp) <= MPL__ERR_GENERIC) {
	    return MPL__ERR_NOMEM;
	}

	mpl__mem_set_zero(smp, opt->smp_num * sizeof(mpl__mx_std_smp_t));
    }

    if (opt->ch_num != -1) {
	if (mpl__mem_alloc(opt->ch_num * 2 * sizeof(mpl__mx_std_ch_t), (void **) &ch) <= MPL__ERR_GENERIC) {
	    if (smp) {
		mpl__mem_free(smp);
	    }

	    return MPL__ERR_NOMEM;
	}

	mpl__mem_set_zero(ch, opt->ch_num * 2 * sizeof(mpl__mx_std_ch_t));
    }

    if (opt->ch_num != -1) {
	int cur;

	mpl__mem_free(mx->ch);

	mx->ch = ch;
	mx->ch_num = opt->ch_num;

	for (cur = 0; cur < mx->ch_num * 2; cur++) {
	    mx->ch[cur].playing = 0;
	}
    }

    if (opt->smp_num != -1) {
	int cur;

	for (cur = 0; cur < mx->smp_num; cur++) {
	    if (mx->smp[cur].exists && mx->smp[cur].smp.data) {
		mpl__mem_free(mx->smp[cur].smp.data);
	    }
	}

	mpl__mem_free(mx->smp);

	mx->smp = smp;
	mx->smp_num = opt->smp_num;

	for (cur = 0; cur < mx->smp_num; cur++) {
	    mx->smp[cur].exists = 0;
	}
    }

    if (opt->mem_usage != -1) {
	mx->mem_usage = opt->mem_usage;
    }

    if (opt->qual != -1) {

	mx->qual = opt->qual;
    }

    return 0;
}

static int mixer_command_get_options(void *internal, mpl__mixer_opt_t * opt)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    opt->ch_num = mx->ch_num;
    opt->smp_num = mx->smp_num;
    opt->mem_usage = mx->mem_usage;
    opt->qual = mx->qual;

    return 0;
}

static int mixer_command_get_proc(void *internal, char *name, void_func_t * func)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    return MPL__ERR_BAD_PARAM;
}

static int mixer_command_set_vol(void *internal, float32 vol, float32 pan_lr, float32 pan_fb)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

#ifdef CHECK_FOR_ERRORS

    if (pan_lr < 0.0f || pan_lr > 1.0f || pan_fb < 0.0f || pan_fb > 1.0f) {
	return MPL__ERR_BAD_PARAM;
    }
#endif

    mx->vol_global = vol;
    mx->pan_lr_global = pan_lr;
    mx->pan_fb_global = pan_fb;

    return 0;
}

static int mixer_command_mix(void *internal, void *buffer, int length)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    mpl__mx_std_ch_t *ch;
    mpl__mx_std_smp_t *smp;
    float32 *mix_buf;
    int mix_buf_size, mix_buf_pos, cnt, gap_size;

    mix_buf_size = mx->output.channels * length;

//      bad (and slow)
    if (mpl__mem_alloc(sizeof(float32) * mix_buf_size, (void **) &mix_buf) <= MPL__ERR_GENERIC) {
	return MPL__ERR_NOMEM;
    }

    mix_buf_pos = 0;

    for (cnt = 0; cnt < mix_buf_size; cnt++) {
	mix_buf[cnt] = 0;
    }

    if (mx->paused) {
	goto convert;
    }

    while (mix_buf_pos < length) {
	gap_size = length - mix_buf_pos;

	if (mx->call_back.func) {
	    if (!mx->count_down) {
		mx->call_back.func(mx->call_back.data, mx->call_back.param);

		mx->count_down_frac += mx->call_back.period * mx->output.freq * 0.001;
		mx->count_down = (int) (mx->count_down_frac);
		mx->count_down_frac -= mx->count_down;
	    }

	    gap_size = gap_size < mx->count_down ? gap_size : mx->count_down;
	}

	for (cnt = 0; cnt < mx->ch_num * 2; cnt++) {
	    ch = mx->ch + cnt;

	    if (!ch->playing) {
		continue;
	    }

	    smp = mx->smp + ch->smp;

	    mix_channel(mx, cnt, mix_buf, mix_buf_pos, gap_size);
	}

	mix_buf_pos += gap_size;
	mx->count_down -= gap_size;
    }

  convert:
    //filter_dc(mx, mix_buf, length);

    quantize(mix_buf, buffer, length * mx->output.channels, mx->output.format);

    mpl__mem_free(mix_buf);

    return 0;
}

static int mixer_command_get_num_active_channels(void *internal)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;
    int ch, cnt;

    cnt = 0;

    for (ch = 0; ch < mx->ch_num; ch++) {
	if (mx->ch[ch].playing) {
	    cnt++;
	}
    }

    return cnt;
}

static int mixer_command_get_interface(void *internal, mpl__snd_mixer_t ** mixer)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) internal;

    *mixer = &mx->iface;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int mpl__mx_std_create(mpl__mixer_t ** mixer)
{
    mpl__mx_std_t *mx;

    if (mpl__mem_alloc(sizeof(mpl__mixer_t), (void **) mixer) <= MPL__ERR_GENERIC) {
	return MPL__ERR_NOMEM;
    }

    if (mpl__mem_alloc(sizeof(mpl__mx_std_t), (void **) &mx) <= MPL__ERR_GENERIC) {
	mpl__mem_free(*mixer);

	return MPL__ERR_NOMEM;
    }

    if (mpl__mem_alloc(sizeof(mpl__mx_std_ch_t) * 2, (void **) &mx->ch) <= MPL__ERR_GENERIC) {
	mpl__mem_free(*mixer);
	mpl__mem_free(mx);

	return MPL__ERR_NOMEM;
    }

    if (mpl__mem_alloc(sizeof(mpl__mx_std_smp_t), (void **) &mx->smp) <= MPL__ERR_GENERIC) {
	mpl__mem_free(*mixer);
	mpl__mem_free(mx);
	mpl__mem_free(mx->ch);

	return MPL__ERR_NOMEM;
    }

    (*mixer)->get_interface = mixer_command_get_interface;
    (*mixer)->get_num_active_channels = mixer_command_get_num_active_channels;
    (*mixer)->get_options = mixer_command_get_options;
    (*mixer)->get_proc = mixer_command_get_proc;
    (*mixer)->mix = mixer_command_mix;
    (*mixer)->set_options = mixer_command_set_options;
    (*mixer)->set_output_options = mixer_command_set_output_options;
    (*mixer)->set_vol = mixer_command_set_vol;

    (*mixer)->internal_data = mx;

    mx->iface.destroy_sample = command_destroy_sample;
    mx->iface.get_free_channel = command_get_free_channel;
    mx->iface.get_latency = command_get_latency;
    mx->iface.get_num_active_channels = command_get_num_active_channels;
    mx->iface.get_num_channels = command_get_num_channels;
    mx->iface.get_option = command_get_option;
    mx->iface.get_output_options = command_get_output_options;
    mx->iface.get_proc = command_get_proc;
    mx->iface.get_vol = command_get_vol;
    mx->iface.is_channel_active = command_is_channel_active;
    mx->iface.pause = command_pause;
    mx->iface.play_channel = command_play_channel;
    mx->iface.reset = command_reset;
    mx->iface.set_call_back = command_set_call_back;
    mx->iface.set_channel_freq = command_set_channel_freq;
    mx->iface.set_channel_pos = command_set_channel_pos;
    mx->iface.set_channel_vol = command_set_channel_vol;
    mx->iface.set_option = command_set_option;
    mx->iface.set_vol = command_set_vol;
    mx->iface.stop = command_stop;
    mx->iface.stop_channel = command_stop_channel;
    mx->iface.upload_sample = command_upload_sample;

    mx->iface.internal_data = mx;

    mx->smp_num = 1;
    mx->smp->exists = 0;

    mx->ch_num = 1;
    mx->ch[0].playing = 0;
    mx->ch[1].playing = 0;

    mx->count_down_frac = 0;
    mx->call_back.func = 0;
    mx->latency = 0;
    mx->mem_usage = MPL__MX_MEM_LOW;
    mx->output.channels = 2;
    mx->output.format = MPL__SND_DEV_FMT_16BIT;
    mx->output.freq = 48000;
    mx->output.latency = MPL__SND_DEV_LATENCY_HIGH;
    mx->paused = 0;
    mx->qual = MPL__MX_HQ;
    mx->vol_global = 1.0f;
    mx->pan_lr_global = 0.5f;
    mx->pan_fb_global = 0.5f;
    mx->vol_local = 1.0f;
    mx->pan_lr_local = 0.5f;
    mx->pan_fb_local = 0.5f;
    mx->dc_spd[0] = 0;
    mx->dc_pos[0] = 0;
    mx->dc_spd[1] = 0;
    mx->dc_pos[1] = 0;
    mx->ramp = 1.0f / (48000 * RAMPING);

    return 0;
}

void mpl__mx_std_destroy(mpl__mixer_t * mixer)
{
    mpl__mx_std_t *mx = (mpl__mx_std_t *) mixer->internal_data;

    mpl__mem_free(mx->ch);
    mpl__mem_free(mx->smp);
    mpl__mem_free(mx);
    mpl__mem_free(mixer);
}
