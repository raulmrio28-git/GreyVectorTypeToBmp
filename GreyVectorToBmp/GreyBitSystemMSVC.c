#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include "GreyBitSystem.h"

typedef struct _GB_SysFileRec {
	FILE *fp;
}GB_SysFileRec, *GB_SysFile;

int GreyBit_Memcmp_Sys(const void *b1, const void *b2, GB_UINT32 n)
{
	return memcmp(b1, b2, (size_t)n);
}

void *GreyBit_Memcpy_Sys(void *d, const void *s, GB_UINT32 n)
{
	return memcpy(d, s, (size_t)n);
}

void *GreyBit_Memset_Sys(void *s, int i, GB_UINT32 n)
{
	return memset(s, i, (size_t)n);
}

char *GreyBit_Strcat_Sys(char *d, const char *s)
{
	return strcat(d, s);
}

int GreyBit_Strcmp_Sys(const char *s1, const char * s2)
{
	return strcmp(s1, s2);
}

char *GreyBit_Strcpy_Sys(char *dest, const char *src)
{
	return strcpy(dest, src);
}

int GreyBit_Strlen_Sys(const char *s)
{
	return strlen(s);
}

int GreyBit_Strncmp_Sys(const char *s1, const char *s2, GB_UINT32 n)
{
	return strncmp(s1, s2, (size_t)n);
}

char *GreyBit_Strncpy_Sys(char *d, const char *s, GB_INT32 n)
{
	return strncpy(d, s, (size_t)n);
}

char *GreyBit_Strchr_Sys(const char *s, char c)
{
	return strchr(s, (int)c);
}

char *GreyBit_Strrchr_Sys(const char *s, char c)
{
	return strrchr(s, (int)c);
}

char *GreyBit_Strstr_Sys(const char *s1, const char *s2)
{
	return strstr(s1, s2);
}

GB_INT32 GreyBit_Atol_Sys(const char *s)
{
	return atoi(s);
}

GB_INT32 GreyBit_Labs_Sys(GB_INT32 i)
{
	return labs(i);
}

void *GreyBit_Malloc_Sys(GB_INT32 size)
{
	void *p = malloc(size);
	GreyBit_Memset_Sys(p, 0, size);
	return p;
}

void *GreyBit_Realloc_Sys(void *p, GB_INT32 newsize)
{
	return realloc(p, newsize);
}

void GreyBit_Free_Sys(void *p)
{
	free(p);
}

GB_IOHandler GreyBit_Open_Sys(const GB_CHAR *p, GB_BOOL bcreate)
{
	GB_SysFile handle = (GB_SysFile)GreyBit_Malloc_Sys(sizeof(GB_SysFileRec));
	if (handle == NULL)
	{
		goto CLEAN_UP;
	}

	if (bcreate)
	{
		handle->fp = fopen(p, "wb");
	}
	else
	{
		handle->fp = fopen(p, "rb");
	}
	if (handle->fp == NULL)
	{
		GreyBit_Free_Sys(handle);
		handle = 0;
		goto CLEAN_UP;
	}

CLEAN_UP:
	return handle;
}

GB_INT32 GreyBit_Read_Sys(GB_IOHandler f, GB_BYTE *p, GB_INT32 size)
{
	GB_SysFile handle = (GB_SysFile)f;
	fread((void*)p, size, 1, handle->fp);
	return size;
}

GB_INT32 GreyBit_Write_Sys(GB_IOHandler f, GB_BYTE *p, GB_INT32 size)
{
	GB_SysFile handle = (GB_SysFile)f;
	return fwrite((void*)p, size, 1, handle->fp);
}

GB_INT32 GreyBit_Seek_Sys(GB_IOHandler f, GB_INT32 pos)
{
	GB_SysFile handle = (GB_SysFile)f;
	return fseek(handle->fp, pos, SEEK_SET);
}

GB_INT32 GreyBit_GetSize_Sys(GB_IOHandler f)
{
	GB_SysFile handle = (GB_SysFile)f;
	GB_INT32 length;
	fseek(handle->fp, 0, SEEK_END);
	length = ftell(handle->fp);
	fseek(handle->fp, 0, SEEK_SET);
	return length;
}

void GreyBit_Close_Sys(GB_IOHandler f)
{
	GB_SysFile handle = (GB_SysFile)f;
	fclose(handle->fp);
	GreyBit_Free_Sys(handle);
}
