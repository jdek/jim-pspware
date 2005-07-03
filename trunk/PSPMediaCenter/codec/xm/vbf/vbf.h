#ifndef vbf_h_n423789784923789423
#define vbf_h_n423789784923789423

#include "../sys_cfg.h"

#define VBF_ATTR_OPEN     1
#define VBF_ATTR_READ     2
#define VBF_ATTR_WRITE    4
#define VBF_ATTR_APPEND   8
#define VBF_ATTR_SEEK     16
#define VBF_ATTR_LENGTH   32

#define VBF_ERR_GENERIC   -1
#define VBF_ERR_BAD_POS   -2
#define VBF_ERR_NOT_OPEN  -3
#define VBF_ERR_NO_LENGTH -4
#define VBF_ERR_NO_SEEK   -5
#define VBF_ERR_NO_READ   -6
#define VBF_ERR_NO_WRITE  -7

typedef long int vbf_size_t;

typedef struct vbf_s {
	int        mode;
	vbf_size_t pos, length;
	int       data;

	unsigned char *buffer;

	int (*raw_read)(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t *out_size);
	int (*raw_write)(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t *out_size);
	int (*raw_ioctl)(struct vbf_s *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t *out_size);
	int (*raw_seek_rel)(struct vbf_s *vbf, vbf_size_t new_pos);
	int (*raw_close)(struct vbf_s *vbf);
} vbf_t;

int vbf_create(vbf_t *vbf);
int vbf_destroy(vbf_t *vbf);

int vbf_seek_beg(vbf_t *vbf, vbf_size_t pos);
int vbf_seek_cur(vbf_t *vbf, vbf_size_t pos);
int vbf_seek_end(vbf_t *vbf, vbf_size_t pos);
int vbf_tell(vbf_t *vbf, vbf_size_t *pos);
int vbf_size(vbf_t *vbf, vbf_size_t *size);
int vbf_mode(vbf_t *vbf, int *mode);
int vbf_read(vbf_t *vbf, void *buffer, vbf_size_t size, vbf_size_t *out_size);
int vbf_write(vbf_t *vbf, void *buffer, vbf_size_t size, vbf_size_t *out_size);
int vbf_ioctl(vbf_t *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t *out_size);
int vbf_close(vbf_t *vbf);

#endif
