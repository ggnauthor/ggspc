/*---------------------------------------------------------------*/
/*                            ds_debug.h                         */
/*---------------------------------------------------------------*/
/*!
*	@file	ds_debug.h
*	@author	ds
*	@brief	デバッグマクロ
*/

#pragma once

/*-----*/
/* lib */
/*-----*/

/*---------*/
/* include */
/*---------*/
#include <crtdbg.h>

/*---------*/
/* define  */
/*---------*/
#if _DEBUG
	#define ENABLE_DUMP_MEM_LEAK	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF)
	#define DEBUG_NEW	new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#define LOG_OUT(fmt, ...) _CrtDbgReport(_CRT_WARN, NULL, NULL, NULL, fmt, __VA_ARGS__);
#else
	#define ENABLE_DUMP_MEM_LEAK
	#define DEBUG_NEW new
	#define LOG_OUT(fmt, ...)
#endif
