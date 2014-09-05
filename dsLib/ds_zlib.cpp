/*---------------------------------------------------------------*/
/*                          ds_zlib.cpp                          */
/*---------------------------------------------------------------*/
/*!
	@file	ds_zlib.cpp
	@brief	zlibを用いた圧縮・解凍。
*/

#include "_prefix.h"

/*---------*/
/* include */
/*---------*/
#include "ds_zlib.h"

/*---------*/
/* globals */
/*---------*/
ds_zlib::EError g_lastError = ds_zlib::EError_NoError;

/*-----------*/
/* functions */
/*-----------*/
#if _DEBUG
	#define DS_ZLIB_DEBUGOUT(zmsg) {\
		char err[256];\
		sprintf(err, "ds_zlib:%s\n", zmsg);\
		OutputDebugStringA(err);\
	}
	#define BUFFERSIZE 16			// 16bytes
#else
	#define DS_ZLIB_DEBUGOUT
	#define BUFFERSIZE (1024 * 512)	// 512kb
#endif

ds_zlib::EError ds_zlib::getLastError(void)
{
	return g_lastError;
}

int ds_zlib::zfsize(FILE* p_fp)
{
	/* 展開後のサイズ */
	int size = 0;
	if (p_fp)
	{
		fpos_t pos;
		fgetpos(p_fp, &pos);
		fseek(p_fp, 0, SEEK_SET);
		fread(&size, 1, 4, p_fp);
		fseek(p_fp, (long)pos, SEEK_SET);
	}
	return size;
}

int ds_zlib::zfread(char* p_outptr, int p_outsize, FILE* p_fp)
{
	if (p_outptr && p_fp)
	{
		char buf[BUFFERSIZE];

		fseek(p_fp, 4, SEEK_SET);

		z_stream z;
		
		z.zalloc = Z_NULL;
		z.zfree  = Z_NULL;
		z.opaque = Z_NULL;
		
		if (inflateInit(&z) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

		z.next_in	= (Bytef*)buf;
		z.avail_in	= fread(buf, 1, BUFFERSIZE, p_fp);
		z.next_out	= (Bytef*)p_outptr;
		z.avail_out = p_outsize;
		
		int status = Z_OK;
		while (status != Z_STREAM_END)
		{
			status = inflate(&z, Z_FINISH);
			if (status < Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);
			if (status == Z_DATA_ERROR) { g_lastError = EError_BadData; break; }
			
			if (status == Z_BUF_ERROR && z.avail_in == 0)
			{
				z.next_in  = (Bytef*)buf;
				z.avail_in = fread(buf, 1, BUFFERSIZE, p_fp);
			}
		}

		if (inflateEnd(&z) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

		return p_outsize - z.avail_out;
	}
	g_lastError = EError_InvalidArgs;
	return 0;
}

int ds_zlib::zfwrite(char* p_inptr, int p_insize, FILE* p_fp)
{
	if (p_inptr && p_fp)
	{
		int fsize = 0;

		char buf[BUFFERSIZE];

		z_stream z;
		
		z.zalloc = Z_NULL;
		z.zfree  = Z_NULL;
		z.opaque = Z_NULL;
		
		if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

		z.next_in	= (Bytef*)p_inptr;
		z.avail_in	= p_insize;
		z.next_out	= (Bytef*)buf;
		z.avail_out = BUFFERSIZE;
		
		/* 元のサイズを先頭に記録 */
		fwrite(&p_insize, 1, 4, p_fp);
		fsize += 4;

		int status = Z_OK;
		while (status != Z_STREAM_END)
		{
			status = deflate(&z, Z_FINISH);
			if (status < Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);
			
			if (z.avail_out == 0 || status == Z_STREAM_END)
			{
				fsize += BUFFERSIZE - z.avail_out;
				fwrite(buf, 1, BUFFERSIZE - z.avail_out, p_fp);
				z.next_out = (Bytef*)buf;
				z.avail_out = BUFFERSIZE;
			}
		}
		
		if (deflateEnd(&z) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

		return fsize;
	}
	g_lastError = EError_InvalidArgs;
	return 0;
}

int ds_zlib::zmsize(char* p_inptr)
{
	/* 展開後のサイズ */
	int size = 0;
	if (p_inptr)
	{
		size = *((DWORD*)p_inptr);
	}
	return size;
}

int ds_zlib::zmread(char* p_inptr, int p_insize, char* p_outptr, int p_outsize)
{
#if _DEBUG
	DWORD t1 = timeGetTime();
#endif

	g_lastError = EError_NoError;

	if (p_inptr && p_outptr)
	{
		z_stream z;
		
		z.zalloc = Z_NULL;
		z.zfree  = Z_NULL;
		z.opaque = Z_NULL;
		
		if (inflateInit(&z) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

		z.next_in	= (Bytef*)(p_inptr + 4);
		z.avail_in	= p_insize - 4;
		z.next_out	= (Bytef*)p_outptr;
		z.avail_out = p_outsize;
		
		int status = Z_OK;
		while (status != Z_STREAM_END)
		{
			status = inflate(&z, Z_FINISH);
			if (status < Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);
			if (status == Z_BUF_ERROR && z.avail_in == 0) { g_lastError = EError_InBufferSize; break; }
			if (status == Z_BUF_ERROR && z.avail_out == 0) { g_lastError = EError_OutBufferSize; break; }
			if (status == Z_DATA_ERROR) { g_lastError = EError_BadData; break; }
		}

		if (inflateEnd(&z) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

#if _DEBUG
		char m[256];
		sprintf(m, "ds_zlib::zmread size: %d byte time: %d ms\n", (int)p_outsize - (int)z.avail_out, timeGetTime() - t1);
		DS_ZLIB_DEBUGOUT(m);
#endif

		return (int)p_outsize - (int)z.avail_out;
	}
	g_lastError = EError_InvalidArgs;
	return 0;
}

int ds_zlib::zmwrite(char* p_inptr, int p_insize, char* p_outptr, int p_outsize)
{
#if _DEBUG
	DWORD t1 = timeGetTime();
#endif

	g_lastError = EError_NoError;

	char* dp = p_outptr;

	if (p_inptr && p_outptr)
	{
		z_stream z;
		
		z.zalloc = Z_NULL;
		z.zfree  = Z_NULL;
		z.opaque = Z_NULL;
		
		if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

		z.next_in	= (Bytef*)p_inptr;
		z.avail_in	= p_insize;
		z.next_out	= (Bytef*)(p_outptr + 4);
		z.avail_out = p_outsize - 4;

		/* 元のサイズを先頭に記録 */
		*((DWORD*)p_outptr) = p_insize;

		int status = Z_OK;
		while (status == Z_OK)
		{
			status = deflate(&z, Z_FINISH);
			if (status < Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);
			if (status == Z_BUF_ERROR) { g_lastError = EError_OutBufferSize; break; } // 出力バッファが足りない
		}
		
		if (deflateEnd(&z) != Z_OK && z.msg) DS_ZLIB_DEBUGOUT(z.msg);

#if _DEBUG
		char m[256];
		sprintf(m, "ds_zlib::zmwrite size: %d byte time: %d ms\n", (int)p_outsize - (int)z.avail_out + 4, timeGetTime() - t1);
		DS_ZLIB_DEBUGOUT(m);
#endif

		return (int)p_outsize - (int)z.avail_out + 4;
	}
	g_lastError = EError_InvalidArgs;
	return 0;
}

#include "_postfix.h"
