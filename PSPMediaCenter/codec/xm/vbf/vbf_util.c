#include "../vbf/vbf_util.h"

// to do:
// - read_line_*()
#if 0
int vbf_read_u64(vbf_t * vbf, int endianess, u64 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, (char *) val, sizeof(u64), &size) < 0) {
	return status;
    }

    if (size != sizeof(u64)) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	*val = ((*val & 0xff00000000000000) >> 56) +
	    ((*val & 0x00ff000000000000) >> 40) +
	    ((*val & 0x0000ff0000000000) >> 24) +
	    ((*val & 0x000000ff00000000) >> 8) +
	    ((*val & 0x00000000ff000000) << 8) +
	    ((*val & 0x0000000000ff0000) << 24) +
	    ((*val & 0x000000000000ff00) << 40) + ((*val & 0x00000000000000ff) << 56);
    }

    return 0;
}

int vbf_read_i64(vbf_t * vbf, int endianess, i64 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(i64), &size) < 0) {
	return status;
    }

    if (size != sizeof(i64)) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	*val = ((*val & 0xff00000000000000) >> 56) +
	    ((*val & 0x00ff000000000000) >> 40) +
	    ((*val & 0x0000ff0000000000) >> 24) +
	    ((*val & 0x000000ff00000000) >> 8) +
	    ((*val & 0x00000000ff000000) << 8) +
	    ((*val & 0x0000000000ff0000) << 24) +
	    ((*val & 0x000000000000ff00) << 40) + ((*val & 0x00000000000000ff) << 56);
    }

    return 0;
}
#endif

int vbf_read_u32(vbf_t * vbf, int endianess, u32 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(u32), &size) < 0) {
	return status;
    }

    if (size != sizeof(u32)) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	*val = ((*val & 0xff000000) >> 24) +
	    ((*val & 0x00ff0000) >> 8) + ((*val & 0x0000ff00) << 8) + ((*val & 0x000000ff) << 24);
    }

    return 0;
}

int vbf_read_i32(vbf_t * vbf, int endianess, i32 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(i32), &size) < 0) {
	return status;
    }

    if (size != sizeof(i32)) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	*val = ((*val & 0xff000000) >> 24) +
	    ((*val & 0x00ff0000) >> 8) + ((*val & 0x0000ff00) << 8) + ((*val & 0x000000ff) << 24);
    }

    return 0;
}

int vbf_read_u16(vbf_t * vbf, int endianess, u16 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(u16), &size) < 0) {
	return status;
    }

    if (size != sizeof(u16)) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	*val = ((*val & 0xff00) >> 8) + ((*val & 0x00ff) << 8);
    }

    return 0;
}

int vbf_read_i16(vbf_t * vbf, int endianess, i16 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(i16), &size) < 0) {
	return status;
    }

    if (size != sizeof(i16)) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	*val = ((*val & 0xff00) >> 8) + ((*val & 0x00ff) << 8);
    }

    return 0;
}

int vbf_read_u8(vbf_t * vbf, int endianess, u8 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(u8), &size) < 0) {
	return status;
    }

    if (size != sizeof(u8)) {
	return VBF_ERR_GENERIC;
    }

    return 0;
}

int vbf_read_i8(vbf_t * vbf, int endianess, i8 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(i8), &size) < 0) {
	return status;
    }

    if (size != sizeof(i8)) {
	return VBF_ERR_GENERIC;
    }

    return 0;
}

int vbf_read_float32(vbf_t * vbf, int endianess, float32 * val)
{
    return vbf_read_u32(vbf, endianess, (u32 *) val);
}

#if 0
int vbf_read_float64(vbf_t * vbf, int endianess, float64 * val)
{
    return vbf_read_u64(vbf, endianess, (u64 *) val);
}
int vbf_read_array_u64(vbf_t * vbf, int endianess, int num, u64 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, (char *) val, sizeof(u64) * num, &size) < 0) {
	return status;
    }

    if ((unsigned int) size != sizeof(u64) * num) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	while (num--) {
	    *val = ((*val & 0xff00000000000000) >> 56) +
		((*val & 0x00ff000000000000) >> 40) +
		((*val & 0x0000ff0000000000) >> 24) +
		((*val & 0x000000ff00000000) >> 8) +
		((*val & 0x00000000ff000000) << 8) +
		((*val & 0x0000000000ff0000) << 24) +
		((*val & 0x000000000000ff00) << 40) + ((*val & 0x00000000000000ff) << 56);
	    val++;
	}
    }

    return 0;
}

int vbf_read_array_i64(vbf_t * vbf, int endianess, int num, i64 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(i64) * num, &size) < 0) {
	return status;
    }

    if ((unsigned int) size != sizeof(i64) * num) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	while (num--) {
	    *val = ((*val & 0xff00000000000000) >> 56) +
		((*val & 0x00ff000000000000) >> 40) +
		((*val & 0x0000ff0000000000) >> 24) +
		((*val & 0x000000ff00000000) >> 8) +
		((*val & 0x00000000ff000000) << 8) +
		((*val & 0x0000000000ff0000) << 24) +
		((*val & 0x000000000000ff00) << 40) + ((*val & 0x00000000000000ff) << 56);
	    val++;
	}
    }

    return 0;
}
#endif
int vbf_read_array_u32(vbf_t * vbf, int endianess, int num, u32 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(u32) * num, &size) < 0) {
	return status;
    }

    if ((unsigned int) size != sizeof(u32) * num) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	while (num--) {
	    *val = ((*val & 0xff000000) >> 24) +
		((*val & 0x00ff0000) >> 8) + ((*val & 0x0000ff00) << 8) + ((*val & 0x000000ff) << 24);
	    val++;
	}
    }

    return 0;
}

int vbf_read_array_i32(vbf_t * vbf, int endianess, int num, i32 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(i32) * num, &size) < 0) {
	return status;
    }

    if ((unsigned int) size != sizeof(i32) * num) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	while (num--) {
	    *val = ((*val & 0xff000000) >> 24) +
		((*val & 0x00ff0000) >> 8) + ((*val & 0x0000ff00) << 8) + ((*val & 0x000000ff) << 24);
	    val++;
	}
    }

    return 0;
}

int vbf_read_array_u16(vbf_t * vbf, int endianess, int num, u16 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(u16) * num, &size) < 0) {
	return status;
    }

    if ((unsigned int) size != sizeof(u16) * num) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	while (num--) {
	    *val = ((*val & 0xff00) >> 8) + ((*val & 0x00ff) << 8);
	    val++;
	}
    }

    return 0;
}

int vbf_read_array_i16(vbf_t * vbf, int endianess, int num, i16 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(i16) * num, &size) < 0) {
	return status;
    }

    if ((unsigned int) size != sizeof(i16) * num) {
	return VBF_ERR_GENERIC;
    }
#ifdef SYS_SMALL_ENDIAN
    if (endianess == VBF_BIG_ENDIAN)
#else
    if (endianess == VBF_SMALL_ENDIAN)
#endif
    {
	while (num--) {
	    *val = ((*val & 0xff00) >> 8) + ((*val & 0x00ff) << 8);
	}
    }

    return 0;
}

int vbf_read_array_u8(vbf_t * vbf, int endianess, int num, u8 * val)
{
    vbf_size_t size;
    int status;

    if (status = vbf_read(vbf, val, sizeof(u8) * num, &size) < 0) {
	return status;
    }

    if ((unsigned int) size != sizeof(u8) * num) {
	return VBF_ERR_GENERIC;
    }

    return 0;
}

int vbf_read_array_i8(vbf_t * vbf, int endianess, int num, i8 * val)
{
    return vbf_read_array_u8(vbf, endianess, num, (u8 *) val);
}

int vbf_read_array_float32(vbf_t * vbf, int endianess, int num, float32 * val)
{
    return vbf_read_array_u32(vbf, endianess, num, (u32 *) val);
}

#if 0
int vbf_read_array_float64(vbf_t * vbf, int endianess, int num, float64 * val)
{
    return vbf_read_array_u64(vbf, endianess, num, (u64 *) val);
}
#endif
int vbf_read_line_char(vbf_t * vbf, char *buf, int max, int *read)
{
    return VBF_ERR_GENERIC;
}

int vbf_read_line_wide_char(vbf_t * vbf, int endianess, wide_char * buf, int max, int *read)
{
    return VBF_ERR_GENERIC;
}
