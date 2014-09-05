/*---------------------------------------------------------------*/
/*                      ds_sharedmemory.cpp                      */
/*---------------------------------------------------------------*/

#include "_prefix.h"

/*---------*/
/* include */
/*---------*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include "ds_sharedmemory.h"

/*---------*/
/* methods */
/*---------*/
ds_sharedmemory::ds_sharedmemory(char* p_name, int p_size)
{
	sprintf(m_mutexname, "%s_mutex", p_name);
	m_mutex		= CreateMutex(0, 0, m_mutexname);
	bool alreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);

	WaitForSingleObject(m_mutex, INFINITE);

	m_mapping	= CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, p_size, p_name);
	m_mapview	= MapViewOfFile(m_mapping, FILE_MAP_ALL_ACCESS, 0, 0, p_size);
	if (!alreadyExists) memset(m_mapview, 0, p_size);
	
	ReleaseMutex(m_mutex);
}

ds_sharedmemory::~ds_sharedmemory(void)
{
	UnmapViewOfFile(m_mapview);
	CloseHandle(m_mapping);

	CloseHandle(m_mutex);
}

void ds_sharedmemory::get(void* p_ptr, int p_offset, int p_size)
{
	HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS , false, m_mutexname);
	WaitForSingleObject(mutex, INFINITE);
	
	memcpy(p_ptr, (void*)((int)m_mapview + p_offset), p_size);

	ReleaseMutex(mutex);
	CloseHandle(mutex);
}

void ds_sharedmemory::set(void* p_ptr, int p_offset, int p_size)
{
	HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS , false, m_mutexname);
	WaitForSingleObject(mutex, INFINITE);

	memcpy((void*)((int)m_mapview + p_offset), p_ptr, p_size);

	ReleaseMutex(mutex);
	CloseHandle(mutex);

}

#include "_postfix.h"
