#ifndef sys_cfg_h_n423788423342
#define sys_cfg_h_n423788423342

typedef int i64;
typedef int i32;
typedef short i16;
typedef char i8;
#ifndef REMOVE_TYPEDEF
typedef unsigned int u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
#endif

typedef float float32;
typedef float float64;

typedef short wide_char;

typedef void (*void_func_t) (void);

#define SYS_SMALL_ENDIAN     1

#define inline_function __inline

#endif
