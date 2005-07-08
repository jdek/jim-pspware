#include "../mpl/sys/mem.h"
#include "../mpl/ml_xm.h"

static int clip_to(int value, int min, int max)
{
    if (value < min) {
	return min;
    }

    if (value > max) {
	return max;
    }

    return value;
}

int mpl__xm_destruct(mpl__xm_t * xm)
{
    int cnt1, cnt2;

    if (xm) {
	if (xm->inst) {
	    for (cnt1 = 0; cnt1 < xm->inst_num; cnt1++) {
		if (xm->inst[cnt1].smp_num) {
		    for (cnt2 = 0; cnt2 < xm->inst[cnt1].smp_num; cnt2++) {
			if (xm->inst[cnt1].smp[cnt2].data) {
			    mpl__mem_free(xm->inst[cnt1].smp[cnt2].data);
			}
		    }

		    mpl__mem_free(xm->inst[cnt1].smp);
		}
	    }

	    mpl__mem_free(xm->inst);
	}

	if (xm->pat_num) {
	    for (cnt1 = 0; cnt1 < xm->pat_num; cnt1++) {
		if (xm->pat[cnt1]) {
		    mpl__mem_free(xm->pat[cnt1]);
		}
	    }
	}
    }

    return 0;
}

#define return_false(err) { mpl__xm_destruct(xm); return err; }
#define seek_to(a)	{if (vbf_seek_beg(file, a) < 0) return_false(MPL__ERR_GENERIC);}
#define read_i8() {if (vbf_read_i8(file, VBF_SMALL_ENDIAN, &tmp_i8) < 0) return_false(MPL__ERR_GENERIC);}
#define read_u8() {if (vbf_read_u8(file, VBF_SMALL_ENDIAN, &tmp_u8) < 0) return_false(MPL__ERR_GENERIC);}
#define read_u16() {if (vbf_read_u16(file, VBF_SMALL_ENDIAN, &tmp_u16) < 0) return_false(MPL__ERR_GENERIC);}
#define read_u32() {if (vbf_read_u32(file, VBF_SMALL_ENDIAN, &tmp_u32) < 0) return_false(MPL__ERR_GENERIC);}

int mpl__xm_load(vbf_t * file, mpl__xm_t * xm)
{
    int mode, attr;
    u8 ID[] = "Extended Module: ", raw[32], tmp_u8;
    i8 tmp_i8;
    u16 tmp_u16;
    u32 tmp_u32;
    vbf_size_t tmp_size, pos;
    u32 h_size, packed_pat_size, ctrl, smp_index;
    i32 cnt1, cnt2, cnt3;
    mpl__xm_sample_t *smp;
    mpl__xm_inst_t *inst;

    if (vbf_mode(file, &mode) < 0) {
	return MPL__ERR_GENERIC;
    }

    attr = VBF_ATTR_OPEN | VBF_ATTR_READ | VBF_ATTR_LENGTH | VBF_ATTR_SEEK;

    if ((mode & attr) != attr) {
	return MPL__ERR_GENERIC;
    }

    mpl__mem_set_zero(xm, sizeof(mpl__xm_t));

    seek_to(0);

    if (vbf_read(file, raw, 17, &tmp_size) < 0 || tmp_size < 17) {
	return MPL__ERR_GENERIC;
    }

    for (cnt1 = 0; cnt1 < 17; cnt1++) {
	if (raw[cnt1] != ID[cnt1]) {
	    return MPL__ERR_GENERIC;
	}
    }

    seek_to(37);

    read_u8();

    if (tmp_u8 != 0x1A) {
	return_false(MPL__ERR_GENERIC);
    }
//      to do
//      if (vbf_read_u16(vbf, VBF_SMALL_ENDIAN, tmp_u16) < 0 || tmp_u16 != 0x104)
//      {
//              return_false;
//      }

    seek_to(17);

    if (vbf_read(file, xm->title, 20, &tmp_size) < 0 || tmp_size < 20) {
	return_false(MPL__ERR_GENERIC);
    }

    xm->title[20] = 0;

    if (vbf_seek_beg(file, 38) < 0) {
	return_false(MPL__ERR_GENERIC);
    }

    if (vbf_read(file, xm->tracker, 20, &tmp_size) < 0 || tmp_size < 20) {
	return_false(MPL__ERR_GENERIC);
    }

    xm->tracker[20] = 0;

    seek_to(60);
    read_u32();
    h_size = tmp_u32;

    read_u16();
    xm->length = tmp_u16;
    read_u16();
    xm->restart_pos = tmp_u16;
    read_u16();
    xm->ch_num = tmp_u16;
    read_u16();
    xm->pat_num = tmp_u16;
    read_u16();
    xm->inst_num = tmp_u16;
    read_u8();
    xm->freq_table = tmp_u8 & 1;
    read_u8();
    read_u16();
    xm->speed = tmp_u16;
    read_u16();
    xm->bpm = tmp_u16;

    seek_to(80);

    for (cnt1 = 0; cnt1 < 256; cnt1++) {
	read_u8();
	xm->order[cnt1] = tmp_u8;
    }

    pos = 60 + h_size;
    seek_to(pos);

    for (cnt1 = 0; cnt1 < xm->pat_num; cnt1++) {
	read_u32();
	h_size = tmp_u32;
	read_u8();
	read_u16();
	xm->pat_length[cnt1] = tmp_u16;
	read_u16();
	packed_pat_size = tmp_u16;

	if (mpl__mem_alloc(xm->pat_length[cnt1] * xm->ch_num * sizeof(mpl__xm_cell_t),
			   (void **) &xm->pat[cnt1]) <= MPL__ERR_GENERIC) {
	    return_false(MPL__ERR_NOMEM);
	}
	pos += h_size;
	seek_to(pos);

	if (!packed_pat_size) {
	    for (cnt2 = 0; cnt2 < xm->pat_length[cnt1] * xm->ch_num; cnt2++) {
		xm->pat[cnt1][cnt2].key = MPL__XM_NO_NOTE;
		xm->pat[cnt1][cnt2].inst = 0;
		xm->pat[cnt1][cnt2].volume = 0;
		xm->pat[cnt1][cnt2].effect = 0;
		xm->pat[cnt1][cnt2].param = 0;
	    }
	} else {
	    for (cnt2 = 0; cnt2 < xm->pat_length[cnt1] * xm->ch_num; cnt2++) {
		read_u8();
		ctrl = tmp_u8;

		if (ctrl & 0x80) {
		    if (ctrl & 1) {
			read_u8();
		    } else {
			tmp_u8 = MPL__XM_NO_NOTE;
		    }
		    xm->pat[cnt1][cnt2].key = tmp_u8 && tmp_u8 < MPL__XM_NO_NOTE ? tmp_u8 : MPL__XM_NO_NOTE;
		    if (ctrl & 2) {
			read_u8();
		    } else {
			tmp_u8 = 0;
		    }
		    xm->pat[cnt1][cnt2].inst = tmp_u8;
		    if (ctrl & 4) {
			read_u8();
		    } else {
			tmp_u8 = 0;
		    }
		    xm->pat[cnt1][cnt2].volume = tmp_u8;
		    if (ctrl & 8) {
			read_u8();
		    } else {
			tmp_u8 = 0;
		    }
		    xm->pat[cnt1][cnt2].effect = tmp_u8;
		    if (ctrl & 16) {
			read_u8();
		    } else {
			tmp_u8 = 0;
		    }
		    xm->pat[cnt1][cnt2].param = tmp_u8;
		} else {
		    xm->pat[cnt1][cnt2].key = ctrl && ctrl < MPL__XM_NO_NOTE ? ctrl : MPL__XM_NO_NOTE;
		    read_u8();
		    xm->pat[cnt1][cnt2].inst = tmp_u8;
		    read_u8();
		    xm->pat[cnt1][cnt2].volume = tmp_u8;
		    read_u8();
		    xm->pat[cnt1][cnt2].effect = tmp_u8;
		    read_u8();
		    xm->pat[cnt1][cnt2].param = tmp_u8;
		}
	    }
	}

	pos += packed_pat_size;
	seek_to(pos);
    }

    if (mpl__mem_alloc(sizeof(mpl__xm_inst_t) * xm->inst_num, (void **) &xm->inst) <= MPL__ERR_GENERIC) {
	return_false(MPL__ERR_NOMEM);
    }

    mpl__mem_set_zero(xm->inst, sizeof(mpl__xm_inst_t) * xm->inst_num);

    smp_index = 0;

    for (cnt1 = 0; cnt1 < xm->inst_num; cnt1++) {
	vbf_tell(file, &pos);

	read_u32();
	h_size = tmp_u32;

	inst = xm->inst + cnt1;

	if (vbf_read(file, inst->name, 22, &tmp_size) < 0 || tmp_size < 22)
	    return_false(MPL__ERR_GENERIC);
	inst->name[22] = 0;

	read_u8();
	read_u16();
	inst->smp_num = tmp_u16;

	if (inst->smp_num) {
	    read_u32();
	    for (cnt2 = 0; cnt2 < 96; cnt2++) {
		read_u8();
		inst->note2smp[cnt2] = tmp_u8;
	    }

	    for (cnt2 = 0; cnt2 < 12; cnt2++) {
		read_u16();
		inst->vol_env[cnt2].x = tmp_u16;
		read_u16();
		inst->vol_env[cnt2].y = tmp_u16;
	    }

	    for (cnt2 = 0; cnt2 < 12; cnt2++) {
		read_u16();
		inst->pan_env[cnt2].x = tmp_u16;
		read_u16();
		inst->pan_env[cnt2].y = tmp_u16;
	    }

	    read_u8();
	    inst->vol_num = tmp_u8;
	    read_u8();
	    inst->pan_num = tmp_u8;
	    read_u8();
	    inst->vol_sus = tmp_u8;
	    read_u8();
	    inst->vol_loop_beg = tmp_u8;
	    read_u8();
	    inst->vol_loop_end = tmp_u8;
	    read_u8();
	    inst->pan_sus = tmp_u8;
	    read_u8();
	    inst->pan_loop_beg = tmp_u8;
	    read_u8();
	    inst->pan_loop_end = tmp_u8;
	    read_u8();
	    inst->vol_type = tmp_u8;
	    read_u8();
	    inst->pan_type = tmp_u8;

	    read_u8();
	    inst->vib_type = tmp_u8;
	    read_u8();
	    inst->vib_sweep = tmp_u8;
	    read_u8();
	    inst->vib_depth = tmp_u8;
	    read_u8();
	    inst->vib_rate = tmp_u8;

	    read_u16();
	    inst->vol_fade_out = 2 * tmp_u16;
	}

	pos += h_size;
	seek_to(pos);

	if (inst->smp_num) {
	    if (mpl__mem_alloc(sizeof(mpl__xm_sample_t) * inst->smp_num, (void **) &inst->smp) <= MPL__ERR_GENERIC) {
		return_false(MPL__ERR_NOMEM);
	    }

	    mpl__mem_set_zero(inst->smp, sizeof(mpl__xm_sample_t) * inst->smp_num);
	}

	for (cnt2 = 0; cnt2 < inst->smp_num; cnt2++) {
	    smp = inst->smp + cnt2;

	    read_u32();
	    smp->length = tmp_u32;
	    read_u32();
	    smp->loop_begin = tmp_u32;
	    read_u32();
	    smp->loop_end = tmp_u32;

	    if (smp->loop_begin >= smp->length) {
		smp->loop_begin = 0;
	    }

	    smp->loop_end += smp->loop_begin;

	    if (smp->loop_end > smp->length) {
		smp->loop_end = smp->length;
	    }

	    read_u8();
	    smp->vol = tmp_u8;
	    read_i8();
	    smp->finetune = tmp_i8;
	    read_u8();
	    smp->format = tmp_u8;
	    read_u8();
	    smp->pan = tmp_u8;
	    read_i8();
	    smp->rel_note = tmp_i8;

	    if (smp->loop_begin == smp->loop_end) {
		smp->format &= ~(MPL__XM_SMP_LOOP | MPL__XM_SMP_BIDI_LOOP);
	    }

	    if (smp->format & MPL__XM_SMP_16BIT) {
		smp->length >>= 1;
		smp->loop_begin >>= 1;
		smp->loop_end >>= 1;
	    }

	    read_u8();

	    if (vbf_read(file, smp->name, 22, &tmp_size) < 0 || tmp_size < 22) {
		return_false(MPL__ERR_GENERIC);
	    }

	    smp->name[22] = 0;

	    smp->index = smp_index++;

	    if (smp->index >= 300) {
		return_false(MPL__ERR_GENERIC);
	    }
	}

	for (cnt2 = 0; cnt2 < inst->smp_num; cnt2++) {
	    smp = inst->smp + cnt2;

	    if (smp->length == 0) {
		continue;
	    }

	    if (mpl__mem_alloc(smp->length * (1 + (smp->format & MPL__XM_SMP_16BIT ? 1 : 0)), &smp->data) <=
		MPL__ERR_GENERIC) {
		return_false(MPL__ERR_NOMEM);
	    }

	    if (smp->format & MPL__XM_SMP_16BIT) {
		i16 *cur = (i16 *) smp->data;

		if (vbf_read_array_i16(file, VBF_SMALL_ENDIAN, smp->length, cur) < 0) {
		    return_false(MPL__ERR_GENERIC);
		}

		for (cnt3 = 1; cnt3 < smp->length; cnt3++) {
		    cur[cnt3] += cur[cnt3 - 1];
		}
	    } else {
		i8 *cur = (i8 *) smp->data;

		if (vbf_read(file, cur, smp->length, &tmp_size) < 0 || tmp_size < smp->length) {
		    return_false(MPL__ERR_GENERIC);
		}

		for (cnt3 = 1; cnt3 < smp->length; cnt3++) {
		    cur[cnt3] += cur[cnt3 - 1];
		}
	    }
	}
    }

    xm->smp_index_num = smp_index;

    return MPL__ERR_OK;
}

#undef return_false
#undef seek_to
#undef read_i8
#undef read_u8
#undef read_u16
#undef read_u32
