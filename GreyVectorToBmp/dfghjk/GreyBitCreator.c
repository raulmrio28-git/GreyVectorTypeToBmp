#include "GreyBitType.h"
#include "GreyBitType_Def.h"
#include "GreyBitSystem.h"
#include "GreyBitCodec.h"

#ifdef ENABLE_ENCODER
extern int GreyBit_Format_Probe(GB_Format format, GB_Stream stream);
extern GB_Encoder GreyBit_Format_EncoderNew(GB_Format format, GB_Creator creator, GB_Stream stream);

GB_Encoder GreyBitType_Creator_Probe(GB_Library library, GB_Creator creator)
{
	GB_BYTE *pExt;
	GB_Encoder encoder;
	GB_Format format;

	format = library->gbFormatHeader;
	encoder = 0;
	while (format)
	{
		if (GreyBit_Format_Probe(format, creator->gbStream))
			return GreyBit_Format_EncoderNew(format, creator, creator->gbStream);
		pExt = (GB_BYTE*)GB_STRRCHR(creator->gbStream->pfilename, '.');
		if (pExt)
		{
			if (!GB_STRCMP(pExt + 1, format->tag))
				return (GB_Encoder)GreyBit_Format_EncoderNew(format, creator, creator->gbStream);
		}
		format = format->next;
	}
	return encoder;
}

GBHANDLE GreyBitType_Creator_New(GBHANDLE library, const GB_CHAR* filepathname)
{
	GB_Library my_library;
	GB_Creator creator;

	my_library = (GB_Library)library;
	creator = (GB_Creator)GreyBit_Malloc(my_library->gbMem, sizeof(GB_Creator));
	if (!creator)
		return creator;
	creator->gbLibrary = my_library;
	creator->gbMem = my_library->gbMem;
	creator->gbStream = (GB_Stream)GreyBit_Stream_New(filepathname, 1);
	if (creator->gbStream)
		creator->gbEncoder = GreyBitType_Creator_Probe(my_library, creator);
	if (creator->gbStream && creator->gbEncoder)
		return creator;
	GreyBitType_Creator_Done(creator);
	return 0;
}

GBHANDLE GreyBitType_Creator_New_Memory(GBHANDLE library, void *pBuf, GB_INT32 nBufSize)
{
	GB_Library my_library;
	GB_Creator creator;

	my_library = (GB_Library)library;
	creator = (GB_Creator)GreyBit_Malloc(my_library->gbMem, sizeof(GB_Creator));
	if (!creator)
		return creator;
	creator->gbLibrary = my_library;
	creator->gbMem = my_library->gbMem;
	creator->gbStream = (GB_Stream)GreyBit_Stream_New_Memory(pBuf, nBufSize);
	if (creator->gbStream)
		creator->gbEncoder = GreyBitType_Creator_Probe(creator->gbLibrary, creator);
	if (creator->gbStream && creator->gbEncoder)
		return creator;
	GreyBitType_Creator_Done(creator);
	return 0;
}

int		GreyBitType_Creator_SetParam(GBHANDLE creator, GB_Param nParam, GB_UINT32 dwParam)
{
	GB_Creator me = (GB_Creator)creator;
	return GreyBit_Encoder_SetParam(me->gbEncoder, nParam, dwParam);
}

int		GreyBitType_Creator_DelChar(GBHANDLE creator, GB_UINT32 nCode)
{
	GB_Creator me = (GB_Creator)creator;
	return GreyBit_Encoder_Delete(me->gbEncoder, nCode);
}

int		GreyBitType_Creator_SaveChar(GBHANDLE creator, GB_UINT32 nCode, GB_Data pData)
{
	GB_Creator me = (GB_Creator)creator;
	return GreyBit_Encoder_Encode(me->gbEncoder, nCode, pData);
}

int		GreyBitType_Creator_Flush(GBHANDLE creator)
{
	GB_Creator me = (GB_Creator)creator;
	return GreyBit_Encoder_Flush(me->gbEncoder);
}

void GreyBitType_Creator_Done(GBHANDLE creator)
{
	GB_Creator me = (GB_Creator)creator;
	if (me->gbEncoder)
		GreyBit_Encoder_Done(me->gbEncoder);
	if (me->gbStream)
		GreyBit_Stream_Done(me->gbStream);
	GreyBit_Free(me->gbMem, creator);
}
#endif //ENABLE_ENCODER