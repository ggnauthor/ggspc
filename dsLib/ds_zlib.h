/*---------------------------------------------------------------*/
/*                           ds_zlib.h                           */
/*---------------------------------------------------------------*/
/*!
	@file	ds_zlib.h
	@brief	zlibÇópÇ¢ÇΩà≥èkÅEâìÄÅB
*/

#pragma once

#pragma comment(lib, "zlib.lib")

#ifdef _DEBUG
#pragma comment(lib, "winmm.lib")
#endif

/*---------*/
/* include */
/*---------*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#ifdef _DEBUG
#include <mmsystem.h>
#endif

#define ZLIB_WINAPI
#include "inc/zlib.h"

namespace ds_zlib
{
	enum EError
	{
		EError_NoError = 0,
		EError_InvalidArgs,
		EError_InBufferSize,
		EError_OutBufferSize,
		EError_BadData,
	};

	EError getLastError(void);
	int fsize(FILE* p_fp);
	
	int zfsize(FILE* p_fp);
	int zfread(char* p_outptr, int p_outsize, FILE* p_fp);
	int zfwrite(char* p_inptr, int p_insize, FILE* p_fp);

	int zmsize(char* p_inptr);
	int zmread(char* p_inptr, int p_insize, char* p_outptr, int p_outsize);
	int zmwrite(char* p_inptr, int p_insize, char* p_outptr, int p_outsize);
};
