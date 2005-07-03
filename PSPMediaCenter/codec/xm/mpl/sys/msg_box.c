#include "../../mpl/sys/msg_box.h"
#include "../../mpl/sys/mem.h"

int
mpl__msg_box_create(mpl__msg_box_t **mb, int max)
{
	/*
	int result;

	result = mpl__mem_alloc(sizeof(mpl__msg_box_t), (void**)mb);

	if (result <= MPL__ERR_GENERIC)
	{
		return result;
	}

	result = mpl__msg_box_construct(*mb, max);

	if (result <= MPL__ERR_GENERIC)
	{
		mpl__mem_free(*mb);

		return result;
	}
	*/

	return 0;
}

int
mpl__msg_box_destroy(mpl__msg_box_t *mb)
{
	/*
	mpl__msg_box_destruct(mb);

	mpl__mem_free(mb);
	*/

	return 0;
}

int
mpl__msg_box_construct(mpl__msg_box_t *mb, int max)
{
	/*
	int result;

	result = mpl__mem_alloc(sizeof(mpl__msg_t) * max, (void**)&mb->msg);
	
	if (result <= MPL__ERR_GENERIC)
	{
		return result;
	}

	mb->first = 0;
	mb->last  = 0;
	mb->cnt   = 0;
	mb->max   = max;

	result = mpl__cs_construct(&mb->cs);

	if (result <= MPL__ERR_GENERIC)
	{
		mpl__mem_free(mb->msg);

		return result;
	}
	*/

	return 0;
}

int
mpl__msg_box_destruct(mpl__msg_box_t *mb)
{
	/*
	mpl__cs_destruct(&mb->cs);

	mpl__mem_free(mb->msg);
	*/

	return 0;
}

int
mpl__msg_box_send(mpl__msg_box_t *mb, mpl__msg_t *msg)
{
	/*
	mpl__cs_enter(&mb->cs);

	if (mb->cnt >= mb->max)
	{
		return MPL__ERR_MSG_BOX_FULL;
	}

	mb->msg[mb->last] = *msg;

	if (++mb->last >= mb->max)
	{
		mb->last = 0;
	}

	mb->cnt++;

	mpl__cs_leave(&mb->cs);
	*/

	return 0;
}

int
mpl__msg_box_receive(mpl__msg_box_t *mb, mpl__msg_t *msg)
{
	/*
	mpl__cs_enter(&mb->cs);

	if (mb->cnt == 0)
	{
		return MPL__ERR_MSG_BOX_EMPTY;
	}

	*msg = mb->msg[mb->first];

	if (++mb->first >= mb->max)
	{
		mb->first = 0;
	}

	mb->cnt--;

	mpl__cs_leave(&mb->cs);
	*/

	return 0;
}
