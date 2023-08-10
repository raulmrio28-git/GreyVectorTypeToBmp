#include "GreyBitType.h"
#include "GreyBitType_Def.h"
#ifdef ENABLE_GREYBITFILE
#include "GreyBitFile.h"
#endif //ENABLE_GREYBITFILE
#ifdef ENABLE_GREYVECTORFILE
#include "GreyVectorFile.h"
#endif //ENABLE_GREYVECTORFILE

extern void GreyBit_Close_Sys(GB_IOHandler f);
extern void* GreyBit_Malloc_Sys(GB_INT32 size);
extern void* GreyBit_Realloc_Sys(void *p, GB_INT32 newsize);
extern void GreyBit_Free_Sys(void *p);

void GreyBitType_Format_Insert(GB_Library library, GB_Format format)
{
	GB_Format curr;

	if (format)
	{
		curr = library->gbFormatHeader;
		library->gbFormatHeader = format;
		format->next = curr;
	}
}

GB_Format GreyBitType_Format_Init(GB_Library library)
{
	GB_Format format;

#ifdef ENABLE_GREYBITFILE
	format = GreyBitFile_Format_New(library);
	GreyBitType_Format_Insert(library, format);
#endif //ENABLE_GREYBITFILE
#ifdef ENABLE_GREYVECTORFILE
	format = GreyVectorFile_Format_New(library);
	GreyBitType_Format_Insert(library, format);
#endif //ENABLE_GREYVECTORFILE
	return library->gbFormatHeader;
}

void GreyBit_Format_Done(GB_Format format, GB_Memory mem)
{
	GreyBit_Free(mem, format);
}

void GreyBitType_Format_Done(GB_Library library)
{
	GB_Format next; 

	while (library->gbFormatHeader)
	{
		next = library->gbFormatHeader->next;
		GreyBit_Format_Done(library->gbFormatHeader, library->gbMem);
		library->gbFormatHeader = next;
	}
}

GB_Bitmap GreyBitType_Bitmap_New(GBHANDLE library, GB_INT16 nWidth, GB_INT16 nHeight, GB_INT16 bitcount, GB_BYTE *pInitBuf)
{
	GB_Bitmap bitmap;
	GB_INT16 currBmp_4;
	GB_Library me = (GB_Library)library;

	currBmp_4 = (bitcount * 8 * nWidth + 63) >> 6;
	bitmap = (GB_Bitmap)GreyBit_Malloc(me->gbMem, nHeight * currBmp_4 + sizeof(GB_BitmapRec));
	if (bitmap)
	{
		bitmap->width = nWidth;
		bitmap->height = nHeight;
		bitmap->bitcount = bitcount;
		bitmap->pitch = currBmp_4;
		bitmap->buffer = (GB_BYTE *)&bitmap[1];
		if (pInitBuf)
			GreyBit_Memcpy_Sys(bitmap->buffer, pInitBuf, nHeight * currBmp_4);
	}
	return bitmap;
}

void GreyBitType_Bitmap_Done(GBHANDLE library, GB_Bitmap bitmap)
{
	GB_Library me = (GB_Library)library;
	GreyBit_Free(me->gbMem, bitmap);
}

#ifdef ENABLE_GREYVECTORFILE
GB_Outline GreyBitType_Outline_New(GBHANDLE library, GB_INT16 n_contours, GB_INT16 n_points)
{
	GB_Outline outline; 
	GB_Library me = (GB_Library)library;

	outline = (GB_Outline)GreyBit_Malloc(me->gbMem, sizeof(GB_OutlineRec) + sizeof(GB_INT16) * n_contours + sizeof(GB_Point) * n_points + n_points);
	if (outline)
	{
		outline->n_contours = n_contours;
		outline->n_points = n_points;
		outline->contours = (GB_INT16*)((GB_BYTE*)outline + sizeof(GB_OutlineRec));
		outline->points = (GB_Point)(outline->contours + n_contours);
		outline->tags = (GB_BYTE*)(outline->points + n_points);
	}
	return outline;
}

GB_Outline GreyBitType_Outline_Clone(GBHANDLE library, GB_Outline source)
{
	int i; 
	int ia;
	GB_Outline outline;

	outline = GreyBitType_Outline_New(library, source->n_contours, source->n_points);
	if (outline)
	{
		for (i = 0; i < outline->n_contours; ++i)
			outline->contours[i] = source->contours[i];
		for (ia = 0; ia < outline->n_points; ++ia)
		{
			outline->points[ia] = source->points[ia];
			outline->tags[ia] = source->tags[ia];
		}
	}
	return outline;
}

int GreyBitType_Outline_Remove(GB_Outline outline, GB_INT16 idx)
{
	GB_INT16 curr; 
	GB_INT16 curra;
	int idxinc; 
	GB_INT16 inc;
	int i; 
	int ia; 

	inc = 0;
	idxinc = 0;
	if (idx > outline->n_points || idx < 0)
		return -1;
	for (i = 0; i < outline->n_contours; ++i)
	{
		curr = outline->contours[i];
		if (curr >= idx)
			inc = 1;
		curra = curr - inc;
		if (curra >= 0)
			outline->contours[i - idxinc] = curra;
		else
			++idxinc;
	}
	outline->n_contours -= idxinc;
	--outline->n_points;
	for (ia = idx; ia < outline->n_points; ++ia)
	{
		outline->points[ia] = outline->points[ia + 1];
		outline->tags[ia] = outline->tags[ia + 1];
	}
	return 0;
}

GB_INT32 GreyBitType_Outline_GetSizeEx(GB_INT16 n_contours, GB_INT16 n_points)
{
	return sizeof(GB_OutlineRec) + sizeof(GB_INT16) * n_contours + sizeof(GB_Point) * n_points + n_points;
}

GB_INT32 GreyBitType_Outline_GetSize(GB_Outline outline)
{
	return GreyBitType_Outline_GetSizeEx(outline->n_contours, outline->n_points);
}

int GreyBitType_Outline_Transform(GB_Outline outline, GB_Outline source, GB_INT16 tosize, GB_INT16 fromsize)
{
	int i; 
	int ia; 

	outline->n_contours = source->n_contours;
	outline->n_points = source->n_points;
	for (i = 0; i < outline->n_contours; ++i)
		outline->contours[i] = source->contours[i];
	for (ia = 0; ia < outline->n_points; ++ia)
	{
		outline->points[ia].x = tosize * source->points[ia].x / fromsize;
		outline->points[ia].y = tosize * source->points[ia].y / fromsize;
		outline->tags[ia] = source->tags[ia];
	}
	return 0;
}

void GreyBitType_Outline_Done(GBHANDLE library, GB_Outline outline)
{
	GB_Library me = (GB_Library)library;
	GreyBit_Free(me->gbMem, outline);
}
#endif //ENABLE_GREYVECTORFILE

GB_Decoder GreyBit_Format_DecoderNew(GB_Format format, GB_Loader loader, GB_Stream stream)
{
	GB_Decoder result; 

	if (format->decodernew)
		result = format->decodernew(loader, stream);
	else
		result = 0;
	return result;
}

#ifdef ENABLE_ENCODER
GB_Encoder GreyBit_Format_EncoderNew(GB_Format format, GB_Creator creator, GB_Stream stream)
{
	GB_Encoder result;

	if (format->encodernew)
		result = format->encodernew(creator, stream);
	else
		result = 0;
	return result;
}
#endif // ENABLE_ENCODER

int GreyBit_Format_Probe(GB_Format format, GB_Stream stream)
{
	int result;

	if (format->probe)
		result = format->probe(stream);
	else
		result = 0;
	return result;
}

GBHANDLE GreyBitType_Init(void)
{
	GB_Library gbLib;

	gbLib = (GB_Library)GreyBit_Malloc_Sys(sizeof(GB_LibraryRec));
	if (gbLib)
	{
		gbLib->gbMem = GreyBit_Memory_New();
		gbLib->gbFormatHeader = GreyBitType_Format_Init(gbLib);
	}
	return gbLib;
}

void GreyBitType_Done(GBHANDLE library)
{
	GB_Library me = (GB_Library)library;
	GreyBitType_Format_Done((GB_Library)library);
	GreyBit_Memory_Done(me->gbMem);
	GreyBit_Free_Sys(library);
}