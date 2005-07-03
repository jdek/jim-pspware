#include "../../mpl/sys/critsect.h"
#include "../../mpl/sys/mem.h"

int
mpl__cs_create(mpl__critical_section_t **cs)
{
	/*
	int result;

	result = mpl__mem_alloc(sizeof(mpl__critical_section_t), (void**)cs);

	if (result <= MPL__ERR_GENERIC)
	{
		return result;
	}

	result = mpl__cs_construct(*cs);

	if (result <= MPL__ERR_GENERIC)
	{
		return result;
	}
	*/

	return 0;
}

int
mpl__cs_destroy(mpl__critical_section_t *cs)
{
	/*
	mpl__cs_destruct(cs);
	mpl__mem_free(cs);
	*/
	
	return 0;
}

int
mpl__cs_construct(mpl__critical_section_t *cs)
{
	/*
	InitializeCriticalSection(cs);
	*/
	
	return 0;
}

int
mpl__cs_destruct(mpl__critical_section_t *cs)
{
	/*
	DeleteCriticalSection(cs);
	*/

	return 0;
}

int
mpl__cs_enter(mpl__critical_section_t *cs)
{
	/*
	EnterCriticalSection(cs);
	*/

	return 0;
}

int
mpl__cs_leave(mpl__critical_section_t *cs)
{
	/*
	LeaveCriticalSection(cs);
	*/

	return 0;
}
