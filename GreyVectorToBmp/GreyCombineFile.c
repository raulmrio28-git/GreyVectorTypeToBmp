#include "GreyBitSystem.h"
#include "GreyCombineFileDecoder.h"
#ifdef ENABLE_ENCODER
#include "GreyCombineFileEncoder.h"
#endif //ENABLE_ENCODER
#include "GreyCombineFile.h"

#ifdef ENABLE_GREYCOMBINEFILE
GB_BOOL GreyCombineFile_Probe(GB_Stream stream)
{
	GREYCOMBINEFILEHEADER fileHeader;
	GreyBit_Stream_Seek(stream, 0);
	GreyBit_Stream_Read(stream, (GB_BYTE *)&fileHeader, sizeof(GREYCOMBINEFILEHEADER));
	return fileHeader.gbfTag[0] == 'g' && fileHeader.gbfTag[1] == 'c' && fileHeader.gbfTag[2] == 't' && fileHeader.gbfTag[3] == 'f';
}

GB_Format GreyCombineFile_Format_New(GB_Library library)
{
	GB_Format format;

	format = (GB_Format)GreyBit_Malloc(library->gbMem, sizeof(GB_FormatRec));
	if (format)
	{
		format->next = 0;
		format->tag[0] = 103;
		format->tag[1] = 99;
		format->tag[2] = 102;
		format->probe = GreyCombineFile_Probe;
		format->decodernew = GreyCombineFile_Decoder_New;
#ifdef ENABLE_ENCODER
		format->encodernew = GreyCombineFile_Encoder_New;
#endif //ENABLE_ENCODER
	}
	return format;
}
#endif //ENABLE_GREYCOMBINEFILE