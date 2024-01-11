#include "GreyBitType.h"
#include "GreyBitType_Def.h"
#include "GreyBitSystem.h"
#include "GreyBitCodec.h"

extern int GreyBit_Format_Probe(GB_Format format, GB_Stream stream);
extern GB_Decoder GreyBit_Format_DecoderNew(GB_Format format, GB_Loader loader, GB_Stream stream);

GB_Decoder GreyBitType_Loader_Probe(GB_Library library, GB_Loader loader)
{
	GB_Decoder decoder;
	GB_Format format;

	format = library->gbFormatHeader;
	decoder = 0;
	while (format)
	{
		if (GreyBit_Format_Probe(format, loader->gbStream))
			decoder = GreyBit_Format_DecoderNew(format, loader, loader->gbStream);
		format = format->next;
	}
	return decoder;
}

GBHANDLE GreyBitType_Loader_New(GBHANDLE library, const GB_CHAR *filepathname)
{
	GB_Library my_library;
	GB_Loader loader;

	my_library = (GB_Library)library;
	loader = (GB_Loader)GreyBit_Malloc(my_library->gbMem, sizeof(GB_LoaderRec));
	if (!loader)
		return loader;
	loader->gbLibrary = my_library;
	loader->gbMem = loader->gbLibrary->gbMem;
	loader->gbStream = (GB_Stream)GreyBit_Stream_New(filepathname, 0);
	if (loader->gbStream)
		loader->gbDecoder = GreyBitType_Loader_Probe(loader->gbLibrary, loader);
	if (loader->gbStream && loader->gbDecoder)
		return loader;
	GreyBitType_Loader_Done(loader);
	return 0;
}

GBHANDLE     GreyBitType_Loader_New_Stream(GBHANDLE library, GBHANDLE stream, GB_INT32 size)
{
	GB_Library my_library;
	GB_Loader loader;

	my_library = (GB_Library)library;
	loader = (GB_Loader)GreyBit_Malloc(my_library->gbMem, sizeof(GB_LoaderRec));
	if (!loader)
		return loader;
	loader->gbLibrary = my_library;
	loader->gbMem = loader->gbLibrary->gbMem;
	loader->gbStream = (GB_Stream)GreyBit_Stream_New_Child(stream);
	if (loader->gbStream)
		GreyBit_Stream_Offset(loader->gbStream, 0, size);
		loader->gbDecoder = GreyBitType_Loader_Probe(loader->gbLibrary, loader);
	if (loader->gbStream && loader->gbDecoder)
		return loader;
	GreyBitType_Loader_Done(loader);
	return 0;
}

GBHANDLE GreyBitType_Loader_New_Memory(GBHANDLE library, void *pBuf, GB_INT32 nBufSize)
{
	GB_Library my_library;
	GB_Loader loader;

	my_library = library;
	loader = (GB_Loader)GreyBit_Malloc(my_library->gbMem, sizeof(GB_LoaderRec));
	if (!loader)
		return loader;
	loader->gbLibrary = my_library;
	loader->gbMem = loader->gbLibrary->gbMem;
	loader->gbStream = (GB_Stream)GreyBit_Stream_New_Memory(pBuf, nBufSize);
	if (loader->gbStream)
		loader->gbDecoder = GreyBitType_Loader_Probe(loader->gbLibrary, loader);
	if (loader->gbStream && loader->gbDecoder)
		return loader;
	GreyBitType_Loader_Done(loader);
	return 0;
}

int GreyBitType_Loader_SetParam(GBHANDLE loader, GB_Param nParam, GB_UINT32 dwParam)
{
	GB_Loader me = (GB_Loader)loader;
	return GreyBit_Decoder_SetParam(me->gbDecoder, nParam, dwParam);
}

GB_INT32 GreyBitType_Loader_GetCount(GBHANDLE loader)
{
	GB_Loader me = (GB_Loader)loader;
	return GreyBit_Decoder_GetCount(me->gbDecoder);
}

GB_INT32 GreyBitType_Loader_GetHeight(GBHANDLE loader)
{
	GB_Loader me = (GB_Loader)loader;
	return GreyBit_Decoder_GetHeight(me->gbDecoder);
}

GB_BOOL GreyBitType_Loader_IsExist(GBHANDLE loader, GB_UINT32 nCode)
{
	GB_Loader me = (GB_Loader)loader;
	return GreyBit_Decoder_GetWidth(me->gbDecoder, nCode, 100) != 0;
}

void GreyBitType_Loader_Done(GBHANDLE loader)
{
	GB_Loader me = (GB_Loader)loader;
	if (me->gbDecoder)
		GreyBit_Decoder_Done(me->gbDecoder);
	if (me->gbStream)
		GreyBit_Stream_Done(me->gbStream);
	GreyBit_Free(me->gbMem, loader);
}