#include "../vbf/vbf_mem.h"

/*

to do
-----

- add support of all attributes

*/

static int vbf_mem_raw_read(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_mem_raw_write(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_mem_raw_ioctl(struct vbf_s *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_mem_raw_seek_rel(struct vbf_s *vbf, vbf_size_t new_pos);
static int vbf_mem_raw_close(struct vbf_s *vbf);

static void __copy_memory__(char *src, char *dst, vbf_size_t size)
{
    while (size-- > 0) {
	*(dst++) = *(src++);
    }
}

int vbf_mem_open(vbf_t * vbf, void *buffer, vbf_size_t size)
{
    vbf_create(vbf);

    vbf->mode = VBF_ATTR_LENGTH | VBF_ATTR_SEEK | VBF_ATTR_OPEN | VBF_ATTR_READ;

    vbf->length = size;
    vbf->data = buffer;

    vbf->raw_read = vbf_mem_raw_read;
    vbf->raw_write = vbf_mem_raw_write;
    vbf->raw_ioctl = vbf_mem_raw_ioctl;
    vbf->raw_seek_rel = vbf_mem_raw_seek_rel;
    vbf->raw_close = vbf_mem_raw_close;

    return 0;
}

static int vbf_mem_raw_read(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    __copy_memory__((char *) vbf->data + vbf->pos, (char *) buffer, size);
    vbf->pos += size;

    if (out_size) {
	*out_size = size;
    }

    return 0;
}

static int vbf_mem_raw_write(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    return 0;
}

static int vbf_mem_raw_ioctl(struct vbf_s *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    return VBF_ERR_GENERIC;
}

static int vbf_mem_raw_seek_rel(struct vbf_s *vbf, vbf_size_t new_pos)
{
    vbf->pos += new_pos;

    return 0;
}

static int vbf_mem_raw_close(struct vbf_s *vbf)
{
    vbf->mode = 0;

    return 0;
}
