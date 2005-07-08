#define REMOVE_TYPEDEF
#include "../vbf/vbf_std.h"

/*

to do
-----

- detection of attributes

*/

static int vbf_std_raw_read(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_std_raw_write(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_std_raw_ioctl(struct vbf_s *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_std_raw_seek_rel(struct vbf_s *vbf, vbf_size_t new_pos);
static int vbf_std_raw_close(struct vbf_s *vbf);

static int pos = 0;

int vbf_std_open(vbf_t * vbf, void *buffer, int bufferSize)
{
    vbf_create(vbf);

    vbf->mode = VBF_ATTR_LENGTH | VBF_ATTR_SEEK | VBF_ATTR_OPEN | VBF_ATTR_READ;

    vbf->data = 0;

    vbf->buffer = buffer;
    vbf->length = bufferSize;

    vbf->pos = 0;
    pos = 0;

    vbf->raw_read = vbf_std_raw_read;
    vbf->raw_write = vbf_std_raw_write;
    vbf->raw_ioctl = vbf_std_raw_ioctl;
    vbf->raw_seek_rel = vbf_std_raw_seek_rel;
    vbf->raw_close = vbf_std_raw_close;

    return 0;
}

static int vbf_std_raw_read(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    int i;
    unsigned char *t = (unsigned char *) buffer;

    if (!size) {
	return 0;
    }

    for (i = 0; i < size; i++) {
	t[i] = vbf->buffer[pos + i];
    }

    vbf->pos += size;
    pos += size;

    if (out_size) {
	*out_size = size;
    }

    return 0;
}

static int vbf_std_raw_write(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    return 0;
}

static int vbf_std_raw_ioctl(struct vbf_s *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    return VBF_ERR_GENERIC;
}

static int vbf_std_raw_seek_rel(struct vbf_s *vbf, vbf_size_t new_pos)
{
    vbf->pos += new_pos;
    pos += new_pos;

    return 0;
}

static int vbf_std_raw_close(struct vbf_s *vbf)
{
    vbf->mode = 0;

    return 0;
}
