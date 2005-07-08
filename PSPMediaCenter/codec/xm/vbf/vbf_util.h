#ifndef vbf_util_h_n4782378243893
#define  vbf_util_h_n4782378243893

#include "../vbf/vbf.h"
#include "../sys_cfg.h"

#define VBF_BIG_ENDIAN       0
#define VBF_SMALL_ENDIAN     1

int vbf_read_u64(vbf_t * vbf, int endianess, u64 * val);
int vbf_read_i64(vbf_t * vbf, int endianess, i64 * val);
int vbf_read_u32(vbf_t * vbf, int endianess, u32 * val);
int vbf_read_i32(vbf_t * vbf, int endianess, i32 * val);
int vbf_read_u16(vbf_t * vbf, int endianess, u16 * val);
int vbf_read_i16(vbf_t * vbf, int endianess, i16 * val);
int vbf_read_u8(vbf_t * vbf, int endianess, u8 * val);
int vbf_read_i8(vbf_t * vbf, int endianess, i8 * val);
int vbf_read_float32(vbf_t * vbf, int endianess, float32 * val);
int vbf_read_float64(vbf_t * vbf, int endianess, float64 * val);

int vbf_read_array_u64(vbf_t * vbf, int endianess, int num, u64 * val);
int vbf_read_array_i64(vbf_t * vbf, int endianess, int num, i64 * val);
int vbf_read_array_u32(vbf_t * vbf, int endianess, int num, u32 * val);
int vbf_read_array_i32(vbf_t * vbf, int endianess, int num, i32 * val);
int vbf_read_array_u16(vbf_t * vbf, int endianess, int num, u16 * val);
int vbf_read_array_i16(vbf_t * vbf, int endianess, int num, i16 * val);
int vbf_read_array_u8(vbf_t * vbf, int endianess, int num, u8 * val);
int vbf_read_array_i8(vbf_t * vbf, int endianess, int num, i8 * val);
int vbf_read_array_float32(vbf_t * vbf, int endianess, int num, float32 * val);
int vbf_read_array_float64(vbf_t * vbf, int endianess, int num, float64 * val);

int vbf_read_line_char(vbf_t * vbf, char *buf, int max, int *read);
int vbf_read_line_wide_char(vbf_t * vbf, int endianess, wide_char * buf, int max, int *read);

#endif
