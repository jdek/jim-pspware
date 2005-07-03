/*

bugs
----

- does MPL__XM_EFF_SMP_OFFSET retrigger?
- cpplay.xm - ahhhhhhhhhh
- crash while playing Enter_the_Merregnon.xm (traxinspace Merregnon compo)
- -27-.xm by floppi -> wrong frequencies
- instrument loading ???
- note delay (btw note delay also has to retrigger if issued with no note!)
- retrig ??
- key off
- do_vol() -> case XM_VOL_EFF_VIB
- note with invalid sample number has to stop current sample

to do
-----

- multi threading !!
- new_note()


*/

#include "../mpl/sys/mem.h"
#include "../mpl/string.h"
#include "../mpl/mp_xm.h"
#include <math.h>
#include "minimath.h"


static int sinus_table[32]=
{
  0, 24, 49, 74, 97,120,141,161,
180,197,212,224,235,244,250,253,
255,253,250,244,235,224,212,197,
180,161,141,120, 97, 74, 49, 24
};

static int mp_cmd_reset(void *internal_data);

static int
clip_to(int value, int min, int max)
{
	if (value < min)
	{
		return min;
	}

	if (value > max)
	{
		return max;
	}

	return value;
}

static int
linear_note_to_period(int note, int fine_tune)
{
	return (int)(7680.0f - note * 64.0f - fine_tune * 0.5f);
}

static float32
linear_period_to_freq(int per)
{
	return (float32)(8363.0 * pow(2.0, (4608.0 - per) * 0.00130208333333));
}

static int
note_to_period(int note, int fine_tune)
{
	float32 period, diff;

	period = (float32) pow(2.0f, (132.0f - note) * 0.08333333333f) * 13.375f;

	if (fine_tune < 0 && note)
	{
		diff = period - (float32)pow(2.0f, (132.0f - note + 1) * 0.08333333333f) * 13.375f;
		diff *= (float32)fabs(fine_tune);
		diff /= -128;
	}
	else
	{
		diff = (float32)pow(2.0f, (132.0f - note - 1) * 0.08333333333f) * 13.375f - period;
		diff *= fine_tune;
		diff /= 128;
	}

	period += diff;

	return (int)period;
}

static float32
period_to_freq(int per)
{
	return 14317056.0f / per;
}

static mpl__xm_cell_t*
get_row(mpl__mp_xm_t *pl)
{
	return pl->xm->pat[pl->xm->order[pl->pat_cur]] + pl->row_cur * pl->xm->ch_num;
}

static void run_tick(mpl__mp_xm_t *player, int param);

static void
update_ctrl(mpl__mp_xm_t *pl)
{
	float32 vol, pan, freq;
	mpl__snd_dev_output_t opt;
	mpl__mp_xm_chan_t *p_ch;
	int cur;

	mpl__snd_mixer_get_output_options(pl->mixer, &opt);

	for(cur = 0; cur < pl->xm->ch_num; cur++)
	{

		p_ch = pl->ch + cur;

		p_ch->active = mpl__snd_mixer_is_channel_active(pl->mixer, cur);

		if (!p_ch->active & !(p_ch->ctrl & MPL__XM_CTRL_START))
		{
			continue;
		}

		if (!p_ch->p_smp)
		{
			continue;
		}

		if (p_ch->ctrl & MPL__XM_CTRL_STOP)
		{
			mpl__snd_mixer_stop_channel(pl->mixer, cur);

			p_ch->active = 0;

			continue;
		}


		if (p_ch->ctrl & (MPL__XM_CTRL_START | MPL__XM_CTRL_VOL))
		{
			p_ch->vol = clip_to(p_ch->vol, 0, 64);

			vol = (float32)clip_to(p_ch->vol + p_ch->vol_delta, 0, 128);
			vol = vol * p_ch->env_vol * p_ch->fade_out_vol;
			vol = vol * (1.0f / (64.0f * 65536.0f * 64.0f));

			p_ch->pan = clip_to(p_ch->pan, 0, 255);

			pan = clip_to(p_ch->pan + p_ch->pan_delta + (p_ch->env_pan << 2) - 128, 16, 239) * (1.0f / 255.0f);
		}
	
		if (p_ch->ctrl & (MPL__XM_CTRL_START | MPL__XM_CTRL_PER))
		{
			p_ch->per = clip_to(p_ch->per, 56, 100000);


			if (pl->xm->freq_table & MPL__XM_FREQ_TABLE_LINEAR)
			{
				freq = linear_period_to_freq(clip_to(p_ch->per + p_ch->per_delta, 56, 100000));
			}
			else
			{
				freq = period_to_freq(clip_to(p_ch->per + p_ch->per_delta, 56, 100000));
			}

			if (freq < 50)
			{
				freq = 50;
			}
		}

		if (p_ch->ctrl & MPL__XM_CTRL_START)
		{
			if (pl->smp_handle[p_ch->p_smp->index] < 0)
			{
				continue;
			}

			mpl__snd_mixer_play_channel(pl->mixer, cur, pl->smp_handle[p_ch->p_smp->index], freq, vol, pan, 0.5f, p_ch->smp_offset, MPL__SND_DEV_DIR_FORWARDS);

			p_ch->smp_offset = 0;
			p_ch->ctrl = 0;
		}

		if (p_ch->ctrl & MPL__XM_CTRL_PER)
		{
			mpl__snd_mixer_set_channel_freq(pl->mixer, cur, freq);
		}

		if (p_ch->ctrl & MPL__XM_CTRL_VOL)
		{
			mpl__snd_mixer_set_channel_vol(pl->mixer, cur, vol, pan, 0.5f);
		}

		p_ch->ctrl = 0;
	}

	if (pl->ctrl & MPL__XM_PL_CTRL_VOL)
	{
		pl->glob_vol = clip_to(pl->glob_vol, 0, 64);

		vol = pl->vol * pl->glob_vol * (1.0f / 64.0f);

		mpl__snd_mixer_set_vol(pl->mixer, vol, 0.5f, 0.5f);
	}

	if (pl->ctrl & MPL__XM_PL_CTRL_BPM)
	{
		mpl__snd_call_back_t cb;

		cb.func   = (mpl__snd_call_back_func_t)run_tick;
		cb.data   = pl;
		cb.param  = 0;
		cb.period = 2500.0f / pl->bpm;

		mpl__snd_mixer_set_call_back(pl->mixer, cb);
	}

	pl->ctrl = 0;
}

static long
get_random()
{
	static long seed = 37842;
    unsigned long low, high;

	low  = 16807 * (seed & 0xFFFF);
	high = 16807 * (long)((unsigned long)seed >> 16);

	low = low + ((high & 0x7FFF) << 16);

	if (low > 2147483647L)
	{
		low = (low & 2147483647L) + 1;
	}

	low = low + (high >> 15);

	if (low > 2147483647L)
	{
		low = (low & 2147483647L) + 1;
	}

	return seed = (long)low;
}

static void
do_vibrato(mpl__mp_xm_chan_t *p_ch, int exp, int update_pos)
{
	int delta, temp;

	temp = p_ch->vib_pos & 31;

	switch (p_ch->wave_control & 3)
	{
		case 0:
			delta = (int)(fabs(sin(p_ch->vib_pos * 0.0981747704246f) * 256.0f));
			break;
		case 1:
/*			temp <<= 3;

			if (p_ch->vib_pos < 0)
			{
				temp = 255 - temp;
			}

			delta = temp;
			break;
		case 2:
		case 3:
			delta = get_random() & 0xff;*/
			break;
	};

	delta *= p_ch->vib_depth;
	delta >>= 7;
	delta <<= exp;

	p_ch->ctrl |= MPL__XM_CTRL_PER;

	if (p_ch->vib_pos >= 0)
	{
		p_ch->per_delta = -delta;
	}
	else
	{
		p_ch->per_delta = delta;
	}

	if (!update_pos)
	{
		return;
	}

	p_ch->vib_pos += p_ch->vib_speed;

	if (p_ch->vib_pos > 31)
	{
		p_ch->vib_pos -= 64;
	}
}

static void
do_inst_vibrato(mpl__mp_xm_chan_t *p_ch)
{
	int delta;

	if (!p_ch->p_inst)
	{
		return;
	}

	switch (p_ch->wave_control & 3)
	{
		case 0:
			delta = (int)((sin(p_ch->inst_vib_pos * 0.0245437f)) * 64.0f);
			break;
		case 1:
			delta = 64;
			if (p_ch->inst_vib_pos > 127)
			{
				delta = -64;
			}
			break;
		case 2:
		case 3:
			delta = (get_random() & 0x8f) - 64;
			break;
	};

	delta *= p_ch->p_inst->vib_depth;

	if (p_ch->p_inst->vib_sweep)
	{
		delta = delta * p_ch->inst_vib_sweep_pos / p_ch->p_inst->vib_sweep;
	}

	delta >>= 6;

	p_ch->per_delta += delta;
	p_ch->ctrl      |= MPL__XM_CTRL_PER;

	if (++p_ch->inst_vib_sweep_pos > p_ch->p_inst->vib_sweep)
	{
		p_ch->inst_vib_sweep_pos = p_ch->p_inst->vib_sweep;
	}

	p_ch->inst_vib_pos += p_ch->p_inst->vib_rate;

	if (p_ch->inst_vib_pos > 255)
	{
		p_ch->inst_vib_pos -= 256;
	}
}


static void
do_tremolo(mpl__mp_xm_chan_t *p_ch)
{
    int delta;

    switch((p_ch->wave_control >> 4) & 3)
	{
	case 0:
		delta = sinus_table[p_ch->tremolo_pos&31];
		break;
	case 1:
		delta = p_ch->tremolo_pos < 32 ?
				p_ch->tremolo_pos << 3 :
				255 - (p_ch->tremolo_pos << 3);
		break;
	case 2:
		delta = 255;
		break;
	case 3:
		delta = get_random() & 0xff;
		break;
    };

    delta *= p_ch->tremolo_depth;
	delta >>= 6;

    if (p_ch->tremolo_pos < 32)
	{
		delta = -delta;
	}

	p_ch->vol_delta = delta;
	p_ch->ctrl     |= MPL__XM_CTRL_VOL;

    p_ch->tremolo_pos += p_ch->tremolo_speed;

	while(p_ch->tremolo_pos >= 64)
	{
		p_ch->tremolo_pos -= 64;
	}
}



static void
do_porta(mpl__mp_xm_chan_t *p_ch)
{
	if (p_ch->porta_period)
	{
		p_ch->per  += clip_to(p_ch->porta_period - p_ch->per, -p_ch->porta_speed, p_ch->porta_speed);
		p_ch->ctrl |= MPL__XM_CTRL_PER;
	}
}

static void
do_tremor(mpl__mp_xm_chan_t *p_ch)
{
	if ((p_ch->tremor_spd >> 4) + (p_ch->tremor_spd & 15) == 0)
	{
		p_ch->tremor_pos = 0;
		return;
	}

	p_ch->tremor_pos %= (p_ch->tremor_spd >> 4) + (p_ch->tremor_spd & 15);

	if (p_ch->tremor_pos < (p_ch->tremor_spd >> 4))
	{
		p_ch->vol_delta = 0;
	}
	else
	{
		p_ch->vol_delta = -p_ch->vol;
	}

	p_ch->ctrl |= MPL__XM_CTRL_VOL;

	p_ch->tremor_pos++;
}

static void
do_env_vol(mpl__mp_xm_chan_t *p_ch)
{
	mpl__xm_point_t *cur, *next;
	int pos, tick_inc = 1, divide;

	if (!p_ch->p_inst->vol_num)
	{
		return;
	}

	pos = 0;

	if (p_ch->p_inst->vol_num > 1)
	{
		while(p_ch->env_vol_tick >= p_ch->p_inst->vol_env[pos + 1].x && pos < p_ch->p_inst->vol_num - 1)
		{
			pos++;
		}
	}

	if (p_ch->env_vol_tick == p_ch->p_inst->vol_env[pos].x)
	{
		if ((p_ch->p_inst->vol_type & MPL__XM_ENV_LOOP) &&
			pos == p_ch->p_inst->vol_loop_end)
		{
			pos                = p_ch->p_inst->vol_loop_beg;
			p_ch->env_vol_tick = p_ch->p_inst->vol_env[pos].x;
		}

		if ((p_ch->p_inst->vol_type & MPL__XM_ENV_SUSTAIN) && pos == p_ch->p_inst->vol_sus && !p_ch->key_off)
		{
			tick_inc = 0;
		}

		if (pos == p_ch->p_inst->vol_num - 1)
		{
			tick_inc = 0;
		}
	}

	cur  = p_ch->p_inst->vol_env + pos;
	next = cur;

	if (p_ch->p_inst->vol_num > 1 && pos < p_ch->p_inst->vol_num - 1)
	{
		next++;
	}

	divide = next->x - cur->x;

	if (divide > 1)
	{
		p_ch->env_vol = cur->y + (next->y - cur->y) * (p_ch->env_vol_tick - cur->x) / divide;
	}
	else
	{
		p_ch->env_vol = cur->y;
	}

	p_ch->ctrl |= MPL__XM_CTRL_VOL;

	if (tick_inc)
	{
		p_ch->env_vol_tick++;
	}
}

static void
do_env_pan(mpl__mp_xm_chan_t *p_ch)
{
	mpl__xm_point_t *cur, *next;
	int pos, tick_inc = 1, divide;

	if (!p_ch->p_inst->pan_num)
	{
		p_ch->env_pan_tick++;
		return;
	}

	pos = 0;

	if (p_ch->p_inst->pan_num > 1)
	{
		while(p_ch->env_pan_tick >= p_ch->p_inst->pan_env[pos + 1].x  && pos < p_ch->p_inst->pan_num - 1)
		{
			pos++;
		}
	}

	if (p_ch->env_pan_tick == p_ch->p_inst->pan_env[pos].x)
	{
		if ((p_ch->p_inst->pan_type & MPL__XM_ENV_LOOP) &&
			pos == p_ch->p_inst->pan_loop_end)
		{
			pos                = p_ch->p_inst->pan_loop_beg;
			p_ch->env_pan_tick = p_ch->p_inst->pan_env[pos].x;
		}

		if ((p_ch->p_inst->pan_type & MPL__XM_ENV_SUSTAIN) && pos == p_ch->p_inst->pan_sus && !p_ch->key_off)
		{
			tick_inc = 0;
		}

		if (pos == p_ch->p_inst->pan_num - 1)
		{
			tick_inc = 0;
		}
	}

	cur  = p_ch->p_inst->pan_env + pos;
	next = cur;

	if (p_ch->p_inst->pan_num > 1 && pos < p_ch->p_inst->pan_num - 1)
	{
		next++;
	}

	divide = next->x - cur->x;

	if (divide > 1)
	{
		p_ch->env_pan = cur->y + (next->y - cur->y) * (p_ch->env_pan_tick - cur->x) / divide;
	}
	else
	{
		p_ch->env_pan = cur->y;
	}

	p_ch->ctrl |= MPL__XM_CTRL_VOL;

	if (tick_inc)
	{
		p_ch->env_pan_tick++;
	}
}

static void
do_vol_slide(mpl__mp_xm_chan_t *p_ch, int slide_value)
{
	int param_x, param_y;

	param_x = slide_value >> 4;
	param_y = slide_value & 0xf;

	if (param_x)
	{
		p_ch->vol += param_x;
	}
	else
	{
		if (param_y)
		{
			p_ch->vol -= param_y;
		}
	}

	p_ch->ctrl |= MPL__XM_CTRL_VOL;
}

static void
do_vol(mpl__mp_xm_t *pl, mpl__mp_xm_chan_t *p_ch, int vol)
{
	if (vol < 0x10)
	{
		return;
	}

	if (vol <= 0x50 && !pl->tick_cur)
	{
		p_ch->vol   = vol - 0x10;
		p_ch->ctrl |= MPL__XM_CTRL_VOL;
	}

	switch(vol & 0xf0)
	{
	case MPL__XM_VOL_EFF_VOL_SLIDE_DOWN:
		if (pl->tick_cur)
		{
			p_ch->vol  = p_ch->vol - (vol & 0xf);
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
		}
		break;
	case MPL__XM_VOL_EFF_VOL_SLIDE_UP:
		if (pl->tick_cur)
		{
			p_ch->vol  = p_ch->vol + (vol & 0xf);
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
		}
		break;
	case MPL__XM_VOL_EFF_FINE_VOL_SLIDE_DOWN:
		if (!pl->tick_cur)
		{
			p_ch->vol  = p_ch->vol - (vol & 0xf);
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
		}
		break;
	case MPL__XM_VOL_EFF_FINE_VOL_SLIDE_UP:
		if (!pl->tick_cur)
		{
			p_ch->vol  = p_ch->vol + (vol & 0xf);
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
		}
		break;
	case MPL__XM_VOL_EFF_VIB_SPEED:
		if (!pl->tick_cur)
		{
			p_ch->vib_speed = vol & 0xf;
		}
		break;
	case MPL__XM_VOL_EFF_VIB:
		if (!pl->tick_cur)
		{
			p_ch->vib_depth = vol & 0xf;
//			do_vibrato(p_ch, 2, 0);
		}
		else
		{
//			do_vibrato(p_ch, 2, 1);
		}

		break;
	case MPL__XM_VOL_EFF_PAN:
		if (!pl->tick_cur)
		{
			p_ch->pan = (vol & 0xf) << 4;
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
		}
		break;
	case MPL__XM_VOL_EFF_PAN_SLIDE_LEFT:
		p_ch->pan   = p_ch->pan - (vol & 0xf);
		p_ch->ctrl |= MPL__XM_CTRL_VOL;
		break;
	case MPL__XM_VOL_EFF_PAN_SLIDE_RIGHT:
		p_ch->pan   = p_ch->pan + (vol & 0xf);
		p_ch->ctrl |= MPL__XM_CTRL_VOL;
		break;
	case MPL__XM_VOL_EFF_PORTA:
		if (vol & 0xf)
		{
			p_ch->porta_speed = (vol & 0xf) << 6;
		}

		if (pl->tick_cur)
		{
			do_porta(p_ch);
		}
		else
		{
			p_ch->porta_period = p_ch->dst_per;
		}
		break;
	}
}

static void
new_smp(mpl__mp_xm_chan_t *p_ch)
{
	if (!p_ch->p_smp || p_ch->key == MPL__XM_NO_NOTE)
	{
		return;
	}

	p_ch->vol                = p_ch->p_smp->vol;
	p_ch->env_vol            = 64;
	p_ch->env_vol_tick       = 0;
	p_ch->fade_out_vol       = 65536;
	p_ch->pan                = p_ch->p_smp->pan;
	p_ch->env_pan            = 32;
	p_ch->env_pan_tick       = 0;
	p_ch->key_off            = 0;
	p_ch->inst_vib_sweep_pos = 0;
	p_ch->inst_vib_pos       = 0;
	p_ch->retrig_vol_slide   = 1;

	if ((p_ch->wave_control & 0xf) < 4)
	{
		p_ch->vib_pos = 0;
	}

	if ((p_ch->wave_control >> 4) < 4)
	{
		p_ch->tremolo_pos = 0;
	}

	p_ch->ctrl |= MPL__XM_CTRL_VOL;
}

static void
retrig(mpl__mp_xm_chan_t *p_ch)
{
	if (!p_ch->p_smp || p_ch->key == MPL__XM_NO_NOTE)
	{
		return;
	}

	p_ch->env_vol            = 64;
	p_ch->env_vol_tick       = 0;
	p_ch->fade_out_vol       = 65536;

	p_ch->env_pan            = 32;
	p_ch->env_pan_tick       = 0;

	p_ch->inst_vib_sweep_pos = 0;
	p_ch->inst_vib_pos       = 0;

	if ((p_ch->wave_control & 0xf) < 4)
	{
		p_ch->vib_pos = 0;
	}

	if ((p_ch->wave_control >> 4) < 4)
	{
		p_ch->tremolo_pos = 0;
	}

	p_ch->ctrl |= MPL__XM_CTRL_START | MPL__XM_CTRL_VOL;
}

static void
new_note(mpl__mp_xm_chan_t *p_ch)
{
}

static void
update_row(mpl__mp_xm_t *pl)
{
	int ch;
	mpl__xm_cell_t *row_cur, *cell;
	mpl__xm_t *xm;
	mpl__mp_xm_chan_t *p_ch;
	mpl__msg_t msg;
	int pattern_jump, pattern_break, porta;
	int param, param_x, param_y;
	int old_vol, old_pan, old_per;

	xm = pl->xm;

	if (pl->pat_cur >= xm->length)
	{
		if (pl->loop == MPL__MP_LOOP_NONE)
		{
			mp_cmd_reset(pl);

			if (pl->mb)
			{
				msg.msg = MPL__MP_MSG_END;

				mpl__msg_box_send(pl->mb, &msg);
			}

			return;
		}

		pl->bpm = xm->bpm;
		pl->speed = xm->speed;
		pl->row_cur = 0;
		pl->pat_cur = 0;
		pl->delay = 0;

		if (pl->loop != MPL__MP_LOOP_FOREVER)
		{
			pl->loop--;
		}

		if (pl->mb)
		{
			msg.msg = MPL__MP_MSG_RESTART;

			mpl__msg_box_send(pl->mb, &msg);
		}
	}

	pattern_jump = 0;
	pattern_break = 0;

	pl->row_next = pl->row_cur + 1;

	row_cur = get_row(pl);

	for(ch = 0; ch < pl->xm->ch_num; ch++)
	{
		cell = &row_cur[ch];
		p_ch  = &pl->ch[ch];

		porta = cell->effect == MPL__XM_EFF_PORTA || 
				cell->effect == MPL__XM_EFF_PORTA_VOL_SLIDE ||
				(cell->volume & 0xf0) == MPL__XM_VOL_EFF_PORTA;

		if (cell->key < MPL__XM_KEY_OFF && !porta)
		{
			p_ch->key = cell->key - 1;
		}

		if (cell->inst)
		{
			p_ch->inst = cell->inst;
		}

		if (cell->key < MPL__XM_KEY_OFF && cell->inst && !porta)
		{
			if (cell->inst <= xm->inst_num)
			{
				int note2smp;

				p_ch->p_inst = xm->inst + p_ch->inst - 1;

				note2smp = p_ch->p_inst->note2smp[p_ch->key];

				if (note2smp >= p_ch->p_inst->smp_num)
				{
					p_ch->inst = 0;
					p_ch->p_inst = 0;
					p_ch->p_smp = 0;
				}
				else
				{
					p_ch->p_smp  = p_ch->p_inst->smp + note2smp;
				}
			}
			else
			{
				p_ch->inst = 0;
				p_ch->p_inst = 0;
				p_ch->p_smp = 0;
			}
		}

		if (p_ch->eff == MPL__XM_EFF_TREMOLO && cell->effect != MPL__XM_EFF_TREMOLO)
		{
			p_ch->vol += p_ch->vol_delta;
		}

		p_ch->vol_delta = 0;
		p_ch->per_delta = 0;

		if (p_ch->p_inst)
		{
			if (cell->key < MPL__XM_KEY_OFF)
			{
				int period;

				p_ch->real_key = cell->key + p_ch->p_smp->rel_note - 1;

				if (xm->freq_table & MPL__XM_FREQ_TABLE_LINEAR)
				{
					period = linear_note_to_period(p_ch->real_key, pl->smp_fine_tune[p_ch->p_smp->index]);
				}
				else
				{
					period = note_to_period(p_ch->real_key, pl->smp_fine_tune[p_ch->p_smp->index]);
				}

				if (porta)
				{
					p_ch->dst_per = period;
				}
				else
				{
					p_ch->per   = period;
					p_ch->ctrl |= MPL__XM_CTRL_START;
				}

			}

			if (cell->inst)
			{
				new_smp(p_ch);
			}
		}

		old_vol = p_ch->vol;
		old_pan = p_ch->pan;
		old_per = p_ch->per;

		do_vol(pl, p_ch, cell->volume);

		if (cell->key == MPL__XM_KEY_OFF || cell->effect == MPL__XM_EFF_KEY_OFF)
		{
			p_ch->key_off = 1;
		}

		if (cell->effect == 0 && cell->param == 0)
		{
			goto no_effects;
		}

		param   = cell->param;
		param_x = param >> 4;
		param_y = param & 0xf;

		switch(cell->effect)
		{
		case MPL__XM_EFF_ARPEGGIO:
			break;
		case MPL__XM_EFF_PORTA_UP:
			if (param)
			{
				p_ch->porta_up = param << 2;
			}
			break;
		case MPL__XM_EFF_PORTA_DOWN:
			if (param)
			{
				p_ch->porta_down = param << 2;
			}
			break;
		case MPL__XM_EFF_PORTA:
			if (param)
			{
				p_ch->porta_speed = param << 2;
			}
			p_ch->porta_period = p_ch->dst_per;
			break;
		case MPL__XM_EFF_VIB:
			if (param_x)
			{
				p_ch->vib_speed = param_x;
			}
			if (param_y)
			{
				p_ch->vib_depth = param_y;
			}
			do_vibrato(p_ch, 2, 0);
			break;
		case MPL__XM_EFF_PORTA_VOL_SLIDE:
			if (param)
			{
				p_ch->porta_vol_slide = param;
			}
			p_ch->porta_period = p_ch->dst_per;
			break;
		case MPL__XM_EFF_VIB_VOL_SLIDE:
			if (param)
			{
				p_ch->vib_vol_slide = param;
			}
			do_vibrato(p_ch, 2, 0);
			break;
		case MPL__XM_EFF_TREMOLO:
			if (cell->param & 0xf0)
			{
				p_ch->tremolo_speed = param_x;
			}
			if (cell->param & 0xf)
			{
				p_ch->tremolo_depth = param_y;
			}
			break;
		case MPL__XM_EFF_PAN:
			p_ch->pan   = param;
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
			break;
		case MPL__XM_EFF_SMP_OFFSET:
			p_ch->smp_offset = param << 8;
			p_ch->ctrl      |= MPL__XM_CTRL_START;
			break;
		case MPL__XM_EFF_VOL_SLIDE:
			if (param)
			{
				p_ch->vol_slide = param;
			}
			break;
		case MPL__XM_EFF_POS_JUMP:
			if (param < xm->length)
			{
				pl->pat_next = param;
				pattern_jump = 1;
			}
			break;
		case MPL__XM_EFF_VOL:
			p_ch->vol   = param;
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
			break;
		case MPL__XM_EFF_PAT_BREAK:
			pl->row_next  = param_x * 10 + param_y;
			pattern_break = 1;
			break;
		case MPL__XM_EFF_MOD_EXT:
			switch(param_x)
			{
			case MPL__XM_EXT_EFF_FINE_PORTA_UP:
				if (param_y)
				{
					p_ch->fporta_up = param_y << 2;
				}
				p_ch->per -= p_ch->fporta_up;
				break;
			case MPL__XM_EXT_EFF_FINE_PORTA_DOWN:
				if (param_y)
				{
					p_ch->fporta_down = param_y << 2;
				}
				p_ch->per += p_ch->fporta_down;
				break;
			case MPL__XM_EXT_EFF_GLISSANDO:
				// *fart*
				break;
			case MPL__XM_EXT_EFF_VIB_WAVE:
				p_ch->wave_control &= 0xf0;
				p_ch->wave_control |= param_y;
				break;
			case MPL__XM_EXT_EFF_FINE_TUNE:
				if (p_ch->p_smp)
				{
					pl->smp_fine_tune[p_ch->p_smp->index] = param_y;
				}
				break;
			case MPL__XM_EXT_EFF_PAT_LOOP:
				if (param_y == 0)
				{
					p_ch->loop_row = pl->row_cur;
				}
				else
				{
					if (!p_ch->loop_num)
					{
						p_ch->loop_num = param_y;
					}
					else
					{
						p_ch->loop_num--;
					}
					if (p_ch->loop_num)
					{
						pl->row_next = p_ch->loop_row - 1;
					}
				}
				break;
			case MPL__XM_EXT_EFF_TREMOLO_WAVE:
				p_ch->wave_control &= 0xf;
				p_ch->wave_control |= param_y << 4;
				break;
			case MPL__XM_EXT_EFF_PAN:
				p_ch->pan   = param_y << 4;
				p_ch->ctrl |= MPL__XM_CTRL_VOL;
				break;
			case MPL__XM_EXT_EFF_RETRIG:
				break;
			case MPL__XM_EXT_EFF_FINE_VOL_SLIDE_UP:
				if (param_y)
				{
					p_ch->fvol_slide_up = param_y;
				}
				p_ch->vol  += p_ch->fvol_slide_up;
				p_ch->ctrl |= MPL__XM_CTRL_VOL;
				break;
			case MPL__XM_EXT_EFF_FINE_VOL_SLIDE_DOWN:
				if (param_y)
				{
					p_ch->fvol_slide_down = param_y;
				}
				p_ch->vol  -= p_ch->fvol_slide_up;
				p_ch->ctrl |= MPL__XM_CTRL_VOL;
				break;
			case MPL__XM_EXT_EFF_NOTE_CUT:
				break;
			case MPL__XM_EXT_EFF_NOTE_DELAY:
/*				p_ch->vol   = old_vol;
				p_ch->pan   = old_pan;
				p_ch->per   = old_per;
				p_ch->ctrl &= ~(MPL__XM_CTRL_START|MPL__XM_CTRL_VOL|MPL__XM_CTRL_PER);*/
				break;
			case MPL__XM_EXT_EFF_PAT_DELAY:
				pl->delay = param_y * pl->speed;
				break;
			default:
				break;
			}
			break;
		case MPL__XM_EFF_SPEED:
			if (param < 32)
			{
				pl->speed = param;
			}
			else
			{
				pl->bpm        = param;
				pl->ctrl      |= MPL__XM_PL_CTRL_BPM;
			}
			break;
		case MPL__XM_EFF_GLOB_VOL:
			pl->glob_vol = param;
			pl->ctrl    |= MPL__XM_PL_CTRL_VOL;
			break;
		case MPL__XM_EFF_GLOB_VOL_SLIDE:
			pl->glob_vol_slide = param;
			pl->ctrl          |= MPL__XM_PL_CTRL_VOL;
			break;
		case MPL__XM_EFF_KEY_OFF:
			break;
		case MPL__XM_EFF_ENV_POS:
			if (p_ch->p_inst)
			{
				if (p_ch->p_inst->vol_type & MPL__XM_ENV_ON)
				{
					p_ch->env_vol_tick = param;
				}
			}
			break;
		case MPL__XM_EFF_PAN_SLIDE:
			if (param)
			{
				p_ch->pan_slide = param;
			}
			break;
		case MPL__XM_EFF_RETRIG_VOL_SLIDE:
			if (param)
			{
				p_ch->retrig_vol_slide = param;
			}
			break;
		case MPL__XM_EFF_TREMOR:
			if (param)
			{
				p_ch->tremor_spd = param;
			}
			do_tremor(p_ch);
			break;
		case MPL__XM_EFF_EXTRA_FINE_PORTA:
			switch(param_x)
			{
			case 1:
				if (param_y)
				{
					p_ch->efporta_up = param_y;
				}
				p_ch->per  -= p_ch->efporta_up;
				p_ch->ctrl |= MPL__XM_CTRL_PER;
				break;
			case 2:
				if (param_y)
				{
					p_ch->efporta_down = param_y;
				}
				p_ch->per  += p_ch->efporta_down;
				p_ch->ctrl |= MPL__XM_CTRL_PER;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
no_effects:
		if (p_ch->p_inst)
		{
			if (p_ch->p_inst->vol_type & MPL__XM_ENV_ON)
			{
				do_env_vol(p_ch);
			}
			else
			{
				if (p_ch->key_off)
				{
					p_ch->env_vol = 0;
				}
			}

			if (p_ch->p_inst->pan_type & MPL__XM_ENV_ON)
			{
				do_env_pan(p_ch);
			}

			if (p_ch->key_off)
			{
				p_ch->fade_out_vol = clip_to(p_ch->fade_out_vol - p_ch->p_inst->vol_fade_out, 0, 65536);
				p_ch->ctrl        |= MPL__XM_CTRL_VOL;
			}
		}

		do_inst_vibrato(p_ch);
	}

	if (pattern_break && !pattern_jump)
	{
		pl->pat_next++;
	}

	if (!pattern_break && pattern_jump)
	{
		pl->row_next=0;
	}

	if (pl->row_next >= xm->pat_length[xm->order[pl->pat_cur]])
	{
		pl->row_next = 0;
		pl->pat_next++;
	}
}

void
update_effects(mpl__mp_xm_t *pl)
{
	int ch;
	mpl__xm_cell_t *row_cur, *cell;
	mpl__xm_t *xm;
	mpl__mp_xm_chan_t *p_ch;

	xm = pl->xm;
	row_cur = get_row(pl);

	for(ch = 0; ch < xm->ch_num; ch++)
	{
		int param, param_x, param_y;

		cell = &row_cur[ch];
		p_ch = &pl->ch[ch];

		p_ch->per_delta = 0;
		p_ch->vol_delta = 0;

		do_vol(pl, p_ch, cell->volume);

		if (cell->effect == 0 && cell->param == 0)
		{
			goto no_effects;
		}

		param   = cell->param;
		param_x = param >> 4;
		param_y = param & 0xf;

		switch(cell->effect)
		{
		case MPL__XM_EFF_ARPEGGIO:
			switch(pl->tick_cur % 3)
			{
			case 1:
				if (p_ch->p_smp)
				{
					if (xm->freq_table & MPL__XM_FREQ_TABLE_LINEAR)
					{
						p_ch->per_delta = param_x << 6;
					}
					else
					{
						p_ch->per_delta = note_to_period(p_ch->real_key + param_x, pl->smp_fine_tune[p_ch->p_smp->index]) - note_to_period(p_ch->real_key, pl->smp_fine_tune[p_ch->p_smp->index]);
					}

					p_ch->ctrl |= MPL__XM_CTRL_PER;
				}
				break;
			case 2:
				if (p_ch->p_smp)
				{
					if (xm->freq_table & MPL__XM_FREQ_TABLE_LINEAR)
					{
						p_ch->per_delta = param_y << 6;
					}
					else
					{
						p_ch->per_delta = note_to_period(p_ch->real_key + param_y, pl->smp_fine_tune[p_ch->p_smp->index]) - note_to_period(p_ch->real_key, pl->smp_fine_tune[p_ch->p_smp->index]);
					}
					p_ch->ctrl |= MPL__XM_CTRL_PER;
				}
				break;
			default:
				p_ch->ctrl |= MPL__XM_CTRL_PER;
				break;
			}
			break;
		case MPL__XM_EFF_PORTA_UP:
			p_ch->per  -= p_ch->porta_up;
			p_ch->ctrl |= MPL__XM_CTRL_PER;
			break;
		case MPL__XM_EFF_PORTA_DOWN:
			p_ch->per  += p_ch->porta_down;
			p_ch->ctrl |= MPL__XM_CTRL_PER;
			break;
		case MPL__XM_EFF_PORTA:
			do_porta(p_ch);
			break;
		case MPL__XM_EFF_VIB:
			do_vibrato(p_ch, 2, 1);
			break;
		case MPL__XM_EFF_PORTA_VOL_SLIDE:
			do_porta(p_ch);
			do_vol_slide(p_ch, p_ch->porta_vol_slide);
			break;
		case MPL__XM_EFF_VIB_VOL_SLIDE:
			do_vibrato(p_ch, 2, 1);
			do_vol_slide(p_ch, p_ch->vib_vol_slide);
			break;
		case MPL__XM_EFF_TREMOLO:
			do_tremolo(p_ch);
			break;
		case MPL__XM_EFF_PAN:
			break;
		case MPL__XM_EFF_SMP_OFFSET:
			break;
		case MPL__XM_EFF_VOL_SLIDE:
			do_vol_slide(p_ch, p_ch->vol_slide);
			break;
		case MPL__XM_EFF_POS_JUMP:
			break;
		case MPL__XM_EFF_VOL:
			break;
		case MPL__XM_EFF_PAT_BREAK:
			break;
		case MPL__XM_EFF_MOD_EXT:
			switch(param & 0xf0)
			{
			case MPL__XM_EXT_EFF_FINE_PORTA_UP:
				break;
			case MPL__XM_EXT_EFF_FINE_PORTA_DOWN:
				break;
			case MPL__XM_EXT_EFF_GLISSANDO:
				break;
			case MPL__XM_EXT_EFF_VIB_WAVE:
				break;
			case MPL__XM_EXT_EFF_FINE_TUNE:
				break;
			case MPL__XM_EXT_EFF_PAT_LOOP:
				break;
			case MPL__XM_EXT_EFF_TREMOLO_WAVE:
				break;
			case MPL__XM_EXT_EFF_PAN:
				break;
			case MPL__XM_EXT_EFF_RETRIG:
				if (!param_y)
				{
					return;
				}
				if (!(pl->tick_cur % param_y))
				{
					retrig(p_ch);
				}
				break;
			case MPL__XM_EXT_EFF_FINE_VOL_SLIDE_UP:
				break;
			case MPL__XM_EXT_EFF_FINE_VOL_SLIDE_DOWN:
				break;
			case MPL__XM_EXT_EFF_NOTE_CUT:
				if (pl->tick_cur == param_y)
				{
					p_ch->ctrl |= MPL__XM_CTRL_STOP;
				}
				break;
			case MPL__XM_EXT_EFF_NOTE_DELAY:
/*				if (pl->tick_cur == param_y)
				{
					if (p_ch->p_smp)
					{
						new_smp(p_ch);
						do_vol(pl, p_ch, cell->volume);
						p_ch->ctrl |= XM_CTRL_START|XM_CTRL_PER|XM_CTRL_VOL;
					}
				}
				else
				{
					p_ch->ctrl &= ~(XM_CTRL_START|XM_CTRL_PER|XM_CTRL_VOL);
				}*/
				break;
			case MPL__XM_EXT_EFF_PAT_DELAY:
				break;
			default:
				break;
			}
			break;
		case MPL__XM_EFF_SPEED:
			break;
		case MPL__XM_EFF_GLOB_VOL:
			break;
		case MPL__XM_EFF_GLOB_VOL_SLIDE:
			param_x = pl->glob_vol_slide >> 4;
			param_y = pl->glob_vol_slide & 0xf;
			if (param_x)
			{
				pl->glob_vol += param_x;
			}
			else
			{
				if (param_y)
				{
					pl->glob_vol -= param_y;
				}
			}
			pl->ctrl |= MPL__XM_PL_CTRL_VOL;
			break;
		case MPL__XM_EFF_KEY_OFF:
			if (pl->tick_cur == param)
			{
				p_ch->key_off = 1;
			}
			break;
		case MPL__XM_EFF_ENV_POS:
			break;
		case MPL__XM_EFF_PAN_SLIDE:
			param_x = p_ch->pan_slide >> 4;
			param_y = p_ch->pan_slide & 0xf;
			if (param_x)
			{
				p_ch->pan += param_x;
			}
			else
			{
				if (param_y)
				{
					p_ch->pan -= param_y;
				}
			}
			p_ch->ctrl |= MPL__XM_CTRL_VOL;
			break;
		case MPL__XM_EFF_RETRIG_VOL_SLIDE:
			if (!(p_ch->retrig_vol_slide & 0xf))
			{
				break;
			}
			if (pl->tick_cur % (p_ch->retrig_vol_slide & 0xf))
			{
				break;
			}

			retrig(p_ch);

			switch(p_ch->retrig_vol_slide & 0xf0)
			{
			case 0x10: p_ch->vol -= 1;	break;
			case 0x90: p_ch->vol += 1; break;
			case 0x20: p_ch->vol -= 2; break;
			case 0xA0: p_ch->vol += 2; break;
			case 0x30: p_ch->vol -= 4; break;
			case 0xB0: p_ch->vol += 4; break;
			case 0x40: p_ch->vol -= 8; break;
			case 0xC0: p_ch->vol += 8; break;
			case 0x50: p_ch->vol -= 16; break;
			case 0xD0: p_ch->vol += 16; break;
			case 0x60: p_ch->vol = (p_ch->vol << 1) / 3; break;
			case 0xE0: p_ch->vol = (p_ch->vol * 3) >> 1; break;
			case 0x70: p_ch->vol >>= 1; break;
			case 0xF0: p_ch->vol <<= 1; break;
			}

			p_ch->ctrl |= MPL__XM_CTRL_VOL;

			break;
		case MPL__XM_EFF_TREMOR:
			do_tremor(p_ch);
			break;
		case MPL__XM_EFF_EXTRA_FINE_PORTA:
			break;
		default:
			break;
		}

no_effects:

		if (p_ch->p_inst)
		{
			if (p_ch->key_off)
			{
				p_ch->fade_out_vol = clip_to(p_ch->fade_out_vol - p_ch->p_inst->vol_fade_out, 0, 65536);
				p_ch->ctrl        |= MPL__XM_CTRL_VOL;
			}

			if (p_ch->p_inst->vol_type & MPL__XM_ENV_ON)
			{
				do_env_vol(p_ch);
			}
			else
			{
				if (p_ch->key_off)
				{
					p_ch->env_vol = 0;
				}
			}

			if (p_ch->p_inst->pan_type & MPL__XM_ENV_ON)
			{
				do_env_pan(p_ch);
			}
		}

		do_inst_vibrato(p_ch);
	}
}

static void
run_tick(mpl__mp_xm_t *pl, int param)
{
	if (!pl->xm->length)
	{
		return;
	}

	if (pl->tick_cur >= pl->speed)
	{
		pl->tick_cur = 0;

		if (!pl->delay)
		{
			pl->pat_cur = pl->pat_next;
			pl->row_cur	= pl->row_next;
			update_row(pl);
		}
		else
		{
			pl->delay--;
		}
	}
	else
	{
		update_effects(pl);
	}

	update_ctrl(pl);

	pl->tick_cur++;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///  interface  ////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

static int
mp_cmd_reset(void *internal_data)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;
	mpl__mp_xm_chan_t *p_ch;
	int ch, inst, smp;

	pl->bpm            = pl->xm->bpm;
	pl->ctrl          |= MPL__XM_PL_CTRL_BPM;
	pl->speed          = pl->xm->speed;
	pl->tick_cur       = pl->speed;
	pl->pat_cur        = 0;
	pl->pat_next       = 0;
	pl->row_cur        = 0;
	pl->row_next       = 0;
	pl->delay          = 0;
	pl->glob_vol       = 64;
	pl->glob_vol_slide = 0;
	pl->delay          = 0;

	mpl__mem_set_zero(pl->ch, sizeof(mpl__mp_xm_chan_t) * 32);

	for(inst = 0; inst < pl->xm->inst_num; inst++)
	{
		for(smp = 0; smp < pl->xm->inst[inst].smp_num; smp++)
		{
			pl->smp_fine_tune[pl->xm->inst[inst].smp[smp].index] = pl->xm->inst[inst].smp[smp].finetune;
		}
	}

	for(ch = 0; ch < 64; ch++)
	{
		p_ch = pl->ch + ch;

		p_ch->pan = 0x80;
		p_ch->key = MPL__XM_NO_NOTE;
	}

	if (pl->dev)
	{
		if (!pl->paused)
		{
			mpl__snd_mixer_pause(pl->mixer);

			pl->paused = 1;
		}

		mpl__snd_mixer_stop(pl->mixer);

		run_tick(pl, 0);
	}

	return 0;
}

static int
mp_cmd_set_dev(void *internal_data, mpl__snd_dev_t *dev)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;
	mpl__snd_mixer_opt_t opt;
	mpl__snd_dev_output_t output;
	mpl__snd_mixer_smp_t upload;
	int inst, smp, cur;
	mpl__xm_sample_t *p_smp;

	if (pl->dev)
	{
		mpl__snd_dev_destroy_mixer(pl->dev, pl->mixer);

		pl->dev = 0;
		pl->mixer = 0;
	}

	if (!dev)
	{
		return MPL__ERR_BAD_PARAM;
	}

	if (mpl__snd_dev_get_output_options(dev, &output) < 0)
	{
		return MPL__ERR_GENERIC;
	}

	opt.latency = MPL__SND_DEV_LATENCY_HIGH;
	opt.ch_num  = pl->xm->ch_num;
	opt.smp_num = pl->xm->smp_index_num;

	if (mpl__snd_dev_create_mixer(dev, &pl->mixer, opt) < 0)
	{
		return MPL__ERR_GENERIC;
	}

	pl->dev = dev;

	for (cur = 0; cur < pl->xm->smp_index_num; cur++)
	{
		pl->smp_handle[cur] = -1;
	}

	for(inst = 0; inst < pl->xm->inst_num; inst++)
	{
		for(smp = 0; smp < pl->xm->inst[inst].smp_num; smp++)
		{
			p_smp = pl->xm->inst[inst].smp + smp;

			if (p_smp->length < 4)
			{
				continue;
			}

			upload.format = p_smp->format & MPL__XM_SMP_16BIT ? MPL__SND_DEV_FMT_16BIT : MPL__SND_DEV_FMT_8BIT;
			upload.channels = 1;
			upload.freq = 8363;
			upload.loop = p_smp->loop_begin;
			upload.data = p_smp->data;

			switch(p_smp->format & 3)
			{
			default:
			case MPL__XM_SMP_NO_LOOP:
				upload.loopmode = MPL__SND_DEV_FMT_NONLOOPING;
				upload.length   = p_smp->length;
				break;
			case MPL__XM_SMP_LOOP:
				upload.loopmode = MPL__SND_DEV_FMT_LOOPING;
				upload.length   = p_smp->loop_end;
				break;
			case MPL__XM_SMP_BIDI_LOOP:
				upload.loopmode = MPL__SND_DEV_FMT_BIDILOOPING;
				upload.length   = p_smp->loop_end;
				break;
			}

			upload.vol    = 1.0f;
			upload.pan_lr = 0.5f;
			upload.pan_fb = 0.5f;

			pl->smp_handle[p_smp->index] = mpl__snd_mixer_upload_sample(pl->mixer, upload);

			if (pl->smp_handle[p_smp->index] < 0)
			{
				mpl__snd_dev_destroy_mixer(pl->dev, pl->mixer);

				pl->dev   = 0;
				pl->mixer = 0;

				return MPL__ERR_GENERIC;
			}
		}
	}

	pl->ctrl          |= MPL__XM_PL_CTRL_BPM;

	if (pl->paused)
	{
		mpl__snd_mixer_pause(pl->mixer);
	}

	run_tick(pl, 0);

	return 0;
}

static int
mp_cmd_play(void *internal_data)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	if (!pl->dev)
	{
		return MPL__ERR_GENERIC;
	}

	if (pl->paused)
	{
		mpl__snd_mixer_pause(pl->mixer);
		pl->paused = 0;

		return MPL__ERR_OK;
	}

	return MPL__ERR_GENERIC;
}

static int
mp_cmd_stop(void *internal_data)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	if (!pl->dev)
	{
		return MPL__ERR_GENERIC;
	}

	if (!pl->paused)
	{
		mpl__snd_mixer_pause(pl->mixer);
		pl->paused = 1;

		return MPL__ERR_OK;
	}

	return MPL__ERR_GENERIC;
}

static int
mp_cmd_set_loop(void *internal_data, int loop)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	if (loop < 0 && loop != MPL__MP_LOOP_FOREVER)
	{
		return MPL__ERR_GENERIC;
	}

	pl->loop = loop;

	return MPL__ERR_OK;
}

static int
mp_cmd_set_pos(void *internal_data, int pos)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;
	int pat_next, row_next;
	int cur = 0;

	pat_next = pos >> 24;
	row_next = pos & 0xfff;

	if (pat_next < 0 || pat_next >= pl->xm->length)
	{
		return MPL__ERR_BAD_PARAM;
	}

	if (row_next < 0 || row_next >= pl->xm->pat_length[pat_next])
	{
		return MPL__ERR_BAD_PARAM;
	}

	pl->pat_next = pat_next;
	pl->row_next = row_next;

	for(cur = 0; cur < pl->xm->ch_num; cur++)
	{
		pl->ch[cur].loop_num = 0;
	}

	return 0;
}

static int
mp_cmd_get_pos(void *internal_data)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	return (pl->pat_cur << 24) + pl->row_cur;
}

static int
mp_cmd_set_vol(void *internal_data, float32 vol)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	pl->vol = vol;

	pl->glob_vol = clip_to(pl->glob_vol, 0, 64);

	vol = pl->vol * pl->glob_vol * (1.0f / 64.0f);

	mpl__snd_mixer_set_vol(pl->mixer, vol, 0.5f, 0.5f);

	return 0;
}

static int
mp_cmd_get_vol(void *internal_data, float32 *vol)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	*vol = pl->vol;

	return 0;
}

static int
mp_cmd_set_option(void *internal_data, char *option, char *value)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	return MPL__ERR_GENERIC;
}

static int
mp_cmd_get_option(void *internal_data, char *option, char **value)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	return MPL__ERR_GENERIC;
}

static int
mp_cmd_get_proc(void *internal_data, char *name, void_func_t *proc)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)internal_data;

	return MPL__ERR_GENERIC;
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
///  creation  /////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int
mpl__mp_xm_construct(mpl__xm_t *xm, mpl__mp_t *mp, mpl__msg_box_t *mb)
{
	mpl__mp_xm_t *pl;

	if (mpl__mem_alloc(sizeof(mpl__mp_xm_t), (void**)&pl) <= MPL__ERR_GENERIC)
	{
		return MPL__ERR_NOMEM;
	}

	mpl__mem_set_zero(pl, sizeof(mpl__mp_xm_t));

	pl->xm     = xm;
	pl->vol    = 0.5f;
	pl->mb     = mb;
	pl->paused = 1;

	mp_cmd_reset(pl);

	mp->get_option = mp_cmd_get_option;
	mp->get_pos    = mp_cmd_get_pos;
	mp->get_proc   = mp_cmd_get_proc;
	mp->get_vol    = mp_cmd_get_vol;
	mp->play       = mp_cmd_play;
	mp->reset      = mp_cmd_reset;
	mp->set_dev    = mp_cmd_set_dev;
	mp->set_loop   = mp_cmd_set_loop;
	mp->set_option = mp_cmd_set_option;
	mp->set_pos    = mp_cmd_set_pos;
	mp->set_vol    = mp_cmd_set_vol;
	mp->stop       = mp_cmd_stop;

	mp->internal_data = pl;

	return 0;
}

int
mpl__mp_xm_destruct(mpl__mp_t *mp)
{
	mpl__mp_xm_t *pl = (mpl__mp_xm_t*)mp->internal_data;

	if (pl->mixer)
	{
		mpl__snd_dev_destroy_mixer(pl->dev, pl->mixer);
	}

	mpl__mem_free(mp->internal_data);

	return 0;
}
