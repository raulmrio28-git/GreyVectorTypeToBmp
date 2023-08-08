#include "GreyBitType.h"
#include "GreyBitType_Def.h"
#include "GreyBitSystem.h"
#include "GreyBitCodec.h"

extern GB_Stream GreyBit_Stream_New(const char* filepathname, char bcreate);
extern GB_Stream GreyBit_Stream_New_Memory(const void *pBuf, GB_INT32 nBufSize);
extern int GreyBit_Format_Probe(GB_FormatRec *format, GB_StreamRec *stream);
extern GB_Decoder GreyBit_Format_DecoderNew(GB_FormatRec *format, GB_LoaderRec *loader, GB_StreamRec *stream);
GB_Decoder GreyBitType_Loader_Probe(GB_Library library, GB_Loader loader)
{
	GB_Decoder decoder;
	GB_Format format;

	format = library->gbFormatHeader;
	decoder = 0;
	while (format)
	{
		if (GreyBit_Format_Probe(format, loader->gbStream))
			return (GB_Decoder)GreyBit_Format_DecoderNew(format, loader, loader->gbStream);
		format = format->next;
	}
	return decoder;
}

GBHANDLE GreyBitType_Loader_New(GBHANDLE library, const GB_CHAR *filepathname)
{
	GB_Library clibrary;
	GB_Loader loader;

	clibrary = (GB_Library)library;
	loader = (GB_Loader)GreyBit_Malloc(clibrary->gbMem, sizeof(GB_LoaderRec));
	if (!loader)
		return loader;
	loader->gbLibrary = clibrary;
	loader->gbMem = loader->gbLibrary->gbMem;
	loader->gbStream = (GB_Stream)GreyBit_Stream_New(filepathname, 0);
	if (loader->gbStream)
		loader->gbDecoder = GreyBitType_Loader_Probe(loader->gbLibrary, loader);
	if (loader->gbStream && loader->gbDecoder)
		return loader;
	GreyBitType_Loader_Done(loader);
	return 0;
}

GBHANDLE GreyBitType_Loader_New_Memory(GBHANDLE library, void *pBuf, GB_INT32 nBufSize)
{
	GB_Library clibrary;
	GB_Loader loader;

	clibrary = library;
	loader = (GB_Loader)GreyBit_Malloc(clibrary->gbMem, sizeof(GB_LoaderRec));
	if (!loader)
		return loader;
	loader->gbLibrary = clibrary;
	loader->gbMem = loader->gbLibrary->gbMem;
	loader->gbStream = (GB_Stream)GreyBit_Stream_New_Memory(pBuf, nBufSize);
	if (loader->gbStream)
		loader->gbDecoder = GreyBitType_Loader_Probe(loader->gbLibrary, loader);
	if (loader->gbStream && loader->gbDecoder)
		return loader;
	GreyBitType_Loader_Done(loader);
	return 0;
}

int GreyBitType_Loader_SetParam(GBHANDLE loader, void *pParam)
{
	GB_Loader me = (GB_Loader)loader;
	return GreyBit_Decoder_SetParam(me->gbDecoder, pParam);
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