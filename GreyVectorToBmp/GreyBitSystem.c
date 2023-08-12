#include "GreyBitSystem.h"

extern GB_IOHandler GreyBit_Open_Sys(const GB_CHAR *p, GB_BOOL bcreate);
extern GB_INT32 GreyBit_Read_Sys(GB_IOHandler f, GB_BYTE *p, GB_INT32 size);
extern GB_INT32 GreyBit_Write_Sys(GB_IOHandler f, GB_BYTE *p, GB_INT32 size);
extern GB_INT32 GreyBit_Seek_Sys(GB_IOHandler f, GB_INT32 pos);
extern GB_INT32 GreyBit_GetSize_Sys(GB_IOHandler f);
extern void GreyBit_Close_Sys(GB_IOHandler f);
extern void* GreyBit_Malloc_Sys(GB_INT32 size);
extern void* GreyBit_Realloc_Sys(void *p, GB_INT32 newsize);
extern void GreyBit_Free_Sys(void *p);

typedef struct _GB_MemStreamRec
{
	GB_BYTE		*pData;
	GB_INT32	size;
	GB_INT32	pos;
} GB_MemStreamRec, *GB_MemStream;

GB_Memory GreyBit_Memory_New()
{
	GB_Memory mem; 

	mem = (GB_Memory)GreyBit_Malloc_Sys(sizeof(GB_MemoryRec));
	if (mem)
	{
		mem->malloc = GreyBit_Malloc_Sys;
		mem->realloc = GreyBit_Realloc_Sys;
		mem->free = GreyBit_Free_Sys;
	}
	return mem;
}

void * GreyBit_Malloc(GB_Memory mem, GB_INT32 size)
{
	return mem->malloc(size);
}

void * GreyBit_Realloc(GB_Memory mem, void *p, GB_INT32 newsize)
{
	return mem->realloc(p, newsize);
}

void GreyBit_Free(GB_Memory mem, void *p)
{
	mem->free(p);
}

void GreyBit_Memory_Done(GB_Memory mem)
{
	GreyBit_Free_Sys(mem);
}

GB_MemStream GreyBit_Open_Mem(const void *p, GB_INT32 size)
{
	GB_MemStream memstream; 

	memstream = (GB_MemStream)GreyBit_Malloc_Sys(sizeof(GB_MemStreamRec));
	if (memstream)
	{
		memstream->pData = (GB_BYTE *)p;
		memstream->size = size;
		memstream->pos = 0;
	}
	return memstream;
}

GB_INT32 GreyBit_Read_Mem(GB_IOHandler f, GB_BYTE *p, GB_INT32 size)
{
	GB_INT32  j; 
	GB_INT32  i;
	GB_MemStream me = (GB_MemStream)f;

	if (size + me->pos > me->size)
		size = me->size - me->pos;
	i = 0;
	j = me->pos;
	while (i < size)
		p[i++] = me->pData[j++];
	me->pos = j;
	return size;
} 

GB_INT32 GreyBit_Write_Mem(GB_IOHandler f, GB_BYTE *p, GB_INT32 size)
{
	GB_INT32  j;
	GB_INT32  i;
	GB_MemStream me = (GB_MemStream)f;
	GB_BYTE *data = me->pData;

	if (size + me->pos > me->size)
		size = me->size - me->pos;
	i = me->pos;
	j = 0;
	while (j < size)
		data[i++] = p[j++];
	me->pos = i;
	return size;
}

GB_INT32 GreyBit_Seek_Mem(GB_IOHandler f, GB_INT32 pos)
{
	GB_MemStream me = (GB_MemStream)f;
	if (pos >= me->size)
		return 0;
	me->pos = pos;
	return me->pos;
}

GB_INT32 GreyBit_Close_Mem(GB_IOHandler f)
{
	GreyBit_Free_Sys(f);
	if (f != 0)
		return -1;
	return 0;
}

GB_Stream GreyBit_Stream_New(const char* filepathname, char bcreate)
{
	GB_Stream stream;
	GB_IOHandler f;

	f = (GB_IOHandler)GreyBit_Open_Sys(filepathname, bcreate);
	if (!f)
		return 0;
	stream = (GB_Stream)GreyBit_Malloc_Sys(sizeof(GB_StreamRec));
	if (stream)
	{
		stream->parent = 0;
		stream->read = GreyBit_Read_Sys;
		stream->write = GreyBit_Write_Sys;
		stream->seek = GreyBit_Seek_Sys;
		stream->close = GreyBit_Close_Sys;
		stream->handler = f;
		stream->size = GreyBit_GetSize_Sys(stream->handler);
		stream->pfilename = (char *)GreyBit_Malloc_Sys(GreyBit_Strlen_Sys(filepathname) + 1);
		stream->offset = 0;
		stream->refcnt = 1;
		GreyBit_Strcpy_Sys(stream->pfilename, filepathname);
	}
	return stream;
}

GB_Stream GreyBit_Stream_New_Memory(const void *pBuf, GB_INT32 nBufSize)
{
	GB_Stream stream; 
	GB_MemStream f;

	f = GreyBit_Open_Mem(pBuf, nBufSize);
	if (!f)
		return 0;
	stream = (GB_Stream)GreyBit_Malloc_Sys(sizeof(GB_StreamRec));
	if (stream)
	{
		stream->parent = 0;
		stream->read = GreyBit_Read_Mem;
		stream->write = GreyBit_Write_Mem;
		stream->seek = GreyBit_Seek_Mem;
		stream->close = GreyBit_Close_Mem;
		stream->handler = f;
		stream->size = nBufSize;
		stream->pfilename = 0;
		stream->offset = 0;
		stream->refcnt = 1;
	}
	return stream;
}

GB_Stream    GreyBit_Stream_New_Child(GB_Stream parent)
{
	GB_Stream stream;

	if (!parent)
		return 0;
	stream = (GB_Stream)GreyBit_Malloc_Sys(sizeof(GB_StreamRec));
	if (stream)
	{
		++parent->refcnt;
		stream->read = GreyBit_Read_Sys;
		stream->parent = parent;
		stream->write = GreyBit_Write_Sys;
		stream->seek = GreyBit_Seek_Sys;
		stream->close = GreyBit_Close_Sys;
		stream->handler = parent->handler;
		stream->size = parent->size;
		stream->offset = parent->offset;
		stream->refcnt = 1;
	}
	return stream;
}

GB_INT32 GreyBit_Stream_Read(GB_Stream stream, GB_BYTE *p, GB_INT32 size)
{
	if (stream->read)
		return stream->read(stream->handler, p, size);
	else
		return 0;
}

GB_INT32 GreyBit_Stream_Write(GB_Stream stream, GB_BYTE *p, GB_INT32 size)
{
	if (stream->write)
		return stream->write(stream->handler, p, size);
	else
		return 0;
}

GB_INT32 GreyBit_Stream_Seek(GB_Stream stream, GB_INT32 pos)
{
	if (stream->seek)
		return stream->seek(stream->handler, stream->offset + pos);
	else
		return 0;
}

GB_INT32     GreyBit_Stream_Offset(GB_Stream stream, GB_INT32 offset, GB_INT32 size)
{
	if (stream->parent)
	{
		stream->offset = stream->parent->offset + offset;
		if (size)
			stream->size = size;
		else
			stream->size = stream->parent->size - stream->offset;
	}
	else
	{
		stream->offset = offset;
	}
	if (stream->seek)
		return stream->seek(stream->handler, stream->offset);
	else
		return 0;
}

void GreyBit_Stream_Done(GB_Stream stream)
{
	stream->refcnt--;
	if (stream->refcnt <= 0)
	{
		if (stream->parent)
			GreyBit_Stream_Done(stream->parent);
		else
		{
			if (stream->close)
				stream->close(stream->handler);
			if (stream->pfilename)
				GreyBit_Free_Sys(stream->pfilename);
		}
		GreyBit_Free_Sys(stream);
	}
}