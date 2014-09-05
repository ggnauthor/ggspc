/*---------------------------------------------------------------*/
/*                       ds_sharedmemory.h                       */
/*---------------------------------------------------------------*/

#pragma once

/*---------*/
/* include */
/*---------*/

/*-------*/
/* class */
/*-------*/
class ds_sharedmemory
{
public:
	ds_sharedmemory(char* p_name, int p_size);
	~ds_sharedmemory(void);

	void get(void* p_ptr, int p_offset, int p_size);
	void set(void* p_ptr, int p_offset, int p_size);

private:
	HANDLE	m_mapping;
	void*	m_mapview;
	HANDLE	m_mutex;
	char	m_mutexname[256];
};
