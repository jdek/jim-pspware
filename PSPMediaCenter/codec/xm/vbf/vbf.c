#include "../vbf/vbf.h"

/*

to do
-----

- equivalent of ungetc(...) ??

*/

static int vbf_raw_read(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_raw_write(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_raw_ioctl(struct vbf_s *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t * out_size);
static int vbf_raw_seek_rel(struct vbf_s *vbf, vbf_size_t new_pos);
static int vbf_raw_close(struct vbf_s *vbf);

int vbf_create(vbf_t * vbf)
{
    vbf->data = 0;
    vbf->mode = 0;
    vbf->length = 0;
    vbf->pos = 0;

    vbf->raw_read = vbf_raw_read;
    vbf->raw_write = vbf_raw_write;
    vbf->raw_ioctl = vbf_raw_ioctl;
    vbf->raw_seek_rel = vbf_raw_seek_rel;
    vbf->raw_close = vbf_raw_close;

    return 0;
}

int vbf_destroy(vbf_t * vbf)
{
    if (vbf->mode & VBF_ATTR_OPEN) {
	return vbf_close(vbf);
    }

    return 0;
}

int vbf_seek_beg(vbf_t * vbf, vbf_size_t pos)
{
    vbf_size_t new_pos;

    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    if (vbf->mode & VBF_ATTR_SEEK == 0) {
	return VBF_ERR_NO_SEEK;
    }

    if (vbf->mode & VBF_ATTR_LENGTH == 0) {
	return VBF_ERR_NO_LENGTH;
    }

    if (pos < 0 || pos > vbf->length) {
	return VBF_ERR_BAD_POS;
    }

    new_pos = pos - vbf->pos;

    if (!new_pos) {
	return 0;
    }

    return vbf->raw_seek_rel(vbf, new_pos);
}

int vbf_seek_cur(vbf_t * vbf, vbf_size_t pos)
{
    vbf_size_t abs_pos;

    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    if (vbf->mode & VBF_ATTR_SEEK == 0) {
	return VBF_ERR_NO_SEEK;
    }

    if (!pos) {
	return 0;
    }

    if (vbf->mode & VBF_ATTR_LENGTH) {
	abs_pos = pos + vbf->pos;

	if (abs_pos < 0 || abs_pos > vbf->length) {
	    return VBF_ERR_BAD_POS;
	}
    }

    return vbf->raw_seek_rel(vbf, pos);
}

int vbf_seek_end(vbf_t * vbf, vbf_size_t pos)
{
    return vbf_seek_beg(vbf, vbf->length + pos);
}

int vbf_tell(vbf_t * vbf, vbf_size_t * pos)
{
    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    if (vbf->mode & VBF_ATTR_LENGTH == 0) {
	return VBF_ERR_NO_LENGTH;
    }

    *pos = vbf->pos;

    return 0;
}

int vbf_size(vbf_t * vbf, vbf_size_t * size)
{
    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    if (vbf->mode & VBF_ATTR_LENGTH == 0) {
	return VBF_ERR_NO_LENGTH;
    }

    *size = vbf->length;

    return 0;
}

int vbf_mode(vbf_t * vbf, int *mode)
{
    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    *mode = vbf->mode;

    return 0;
}

int vbf_read(vbf_t * vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    if (vbf->mode & VBF_ATTR_READ == 0) {
	return VBF_ERR_NO_READ;
    }

    if (vbf->mode & VBF_ATTR_LENGTH) {
	if (vbf->pos + size > vbf->length) {
	    size = vbf->length - vbf->pos;
	}
    }

    return vbf->raw_read(vbf, buffer, size, out_size);
}

int vbf_write(vbf_t * vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    if (vbf->mode & VBF_ATTR_WRITE == 0) {
	return VBF_ERR_NO_WRITE;
    }

    if (vbf->mode & VBF_ATTR_LENGTH && !(vbf->mode & VBF_ATTR_APPEND)) {
	if (vbf->pos + size > vbf->length) {
	    size = vbf->length - vbf->pos;
	}
    }

    return vbf->raw_write(vbf, buffer, size, out_size);
}

int vbf_ioctl(vbf_t * vbf, int command, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    return vbf->raw_ioctl(vbf, command, buffer, size, out_size);
}

int vbf_close(vbf_t * vbf)
{
    if (vbf->mode & VBF_ATTR_OPEN == 0) {
	return VBF_ERR_NOT_OPEN;
    }

    return vbf->raw_close(vbf);
}

static int vbf_raw_read(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    return VBF_ERR_GENERIC;
}

static int vbf_raw_write(struct vbf_s *vbf, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    return VBF_ERR_GENERIC;
}

static int vbf_raw_ioctl(struct vbf_s *vbf, int command, void *buffer, vbf_size_t size, vbf_size_t * out_size)
{
    return VBF_ERR_GENERIC;
}

static int vbf_raw_seek_rel(struct vbf_s *vbf, vbf_size_t new_pos)
{
    return VBF_ERR_GENERIC;
}

static int vbf_raw_close(struct vbf_s *vbf)
{
    vbf->mode = 0;

    return 0;
}
