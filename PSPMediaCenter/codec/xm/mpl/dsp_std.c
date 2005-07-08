#include "../mpl/dsp_std.h"
#include "../mpl/sys/mem.h"

/*
	to do
	-----

    - optimize (especially 24 bit adding)
	- dsp_convert_format() -> 24_bit
    - dsp_resample()
*/

int mpl__dsp_add(void *src, void *dst, int size, int ch_num, float32 vol, int format)
{
    int num;

    num = size * ch_num;

    switch (format) {
    case MPL__SND_DEV_FMT_8BIT:
	while (num--) {
	    ((i8 *) dst)[num] += (i8) (((i8 *) src)[num] * vol);
	}
	break;
    case MPL__SND_DEV_FMT_16BIT:
	while (num--) {
	    ((i16 *) dst)[num] += (i16) (((i16 *) src)[num] * vol);
	}
	break;
    case MPL__SND_DEV_FMT_24BIT:
	while (num--) {
//                      *((i32*)dst) = ((*((i32*)dst) & 0xffffff) + (i32)((*((i32*)src) & 0xffffff) * vol)) & 0xffffff;

//                      ((i8*)src) += 3;
//                      ((i8*)dst) += 3;
	}
	break;
    case MPL__SND_DEV_FMT_32BIT:
	while (num--) {
	    ((i32 *) dst)[num] += (i32) (((i32 *) src)[num] * vol);
	}
	break;
    case MPL__SND_DEV_FMT_FLOAT_32BIT:
	while (num--) {
	    ((float32 *) dst)[num] += ((float32 *) src)[num] * vol;
	}
	break;
    default:
	return -1;
    }

    return 0;
}

int mpl__dsp_sub(void *src, void *dst, int size, int ch_num, float32 vol, int format)
{
    return mpl__dsp_add(src, dst, size, ch_num, -vol, format);
}

int mpl__dsp_move(void *src, void *dst, int size, int ch_num, float32 vol, int format)
{
    switch (format) {
    case MPL__SND_DEV_FMT_8BIT:
	break;
    case MPL__SND_DEV_FMT_16BIT:
	size *= 2;
	break;
    case MPL__SND_DEV_FMT_24BIT:
	size *= 3;
	break;
    case MPL__SND_DEV_FMT_32BIT:
	size *= 4;
	break;
    case MPL__SND_DEV_FMT_FLOAT_32BIT:
	size *= 4;
	break;
    default:
	return -1;
    }

    size *= ch_num;

    mpl__mem_copy(src, dst, size);

    return 0;
}

int mpl__dsp_conv_format(void *src, void *dst, int src_format, int dst_format, int size, int ch_num, float32 vol)
{
    float32 *buf;
    int cnt;

    if (dst_format != MPL__SND_DEV_FMT_FLOAT_32BIT) {
	if (mpl__mem_alloc(4 * size * ch_num, (void **) &buf) < MPL__ERR_OK) {
	    return MPL__ERR_NOMEM;
	}
    } else {
	buf = (float32 *) dst;
    }

    cnt = size * ch_num;

    switch (src_format) {
    case MPL__SND_DEV_FMT_8BIT:
	vol *= 1.0f / 256.0f;
	while (cnt--) {
	    buf[cnt] = ((i8 *) src)[cnt] * vol;
	}
	break;
    case MPL__SND_DEV_FMT_16BIT:
	vol *= 1.0f / 65536.0f;
	while (cnt--) {
	    buf[cnt] = ((i16 *) src)[cnt] * vol;
	}
	break;
    case MPL__SND_DEV_FMT_24BIT:
/*        vol *= 1.0f / 16777216.0f;
		while(cnt--)
		{
			buf[cnt] = (((i32*)src)[cnt] & 0xffffff) * vol;

			((i8*)src) += 3;
		}*/
	break;
    case MPL__SND_DEV_FMT_32BIT:
	vol *= 1.0f / 4294967296.0f;
	while (cnt--) {
	    buf[cnt] = ((i32 *) src)[cnt] * vol;
	}
	break;
    case MPL__SND_DEV_FMT_FLOAT_32BIT:
	while (cnt--) {
	    buf[cnt] = ((float32 *) src)[cnt] * vol;
	}
	break;
    }

    if (dst_format == MPL__SND_DEV_FMT_FLOAT_32BIT) {
	return MPL__ERR_OK;
    }

    cnt = size * ch_num;

    switch (dst_format) {
    case MPL__SND_DEV_FMT_8BIT:
	vol = 256.0f;
	while (cnt--) {
	    ((i8 *) dst)[cnt] = (i8) (buf[cnt] * vol);
	}
	break;
    case MPL__SND_DEV_FMT_16BIT:
	vol = 65536.0f;
	while (cnt--) {
	    ((i16 *) dst)[cnt] = (i16) (buf[cnt] * vol);
	}
	break;
    case MPL__SND_DEV_FMT_24BIT:
	vol = 16777216.0f;
	while (cnt--) {
	    // how to implement it portable?!?
	}
	break;
    case MPL__SND_DEV_FMT_32BIT:
	vol = 4294967296.0f;
	while (cnt--) {
	    ((i32 *) dst)[cnt] = (i32) (buf[cnt] * vol);
	}
	break;
    }

    mpl__mem_free(buf);

    return MPL__ERR_OK;
}
