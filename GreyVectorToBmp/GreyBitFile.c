#include "GreyBitSystem.h"
#include "GreyBitFileDecoder.h"
#include "GreyBitFile.h"

#ifdef ENABLE_GREYBITFILE
GB_BOOL GreyBitFile_Probe(GB_Stream stream)
{
	GREYBITFILEHEADER fileHeader;
	GreyBit_Stream_Seek(stream, 0);
	GreyBit_Stream_Read(stream, (GB_BYTE *)&fileHeader, sizeof(GREYBITFILEHEADER));
	return fileHeader.gbfTag[0] == 'g' && fileHeader.gbfTag[1] == 'b' && fileHeader.gbfTag[2] == 't' && fileHeader.gbfTag[3] == 'f';
}

GB_Format GreyBitFile_Format_New(GB_Library library)
{
	GB_Format format;

	format = (GB_Format)GreyBit_Malloc(library->gbMem, sizeof(GB_FormatRec));
	if (format)
	{
		format->next = 0;
		format->tag[0] = 103;
		format->tag[1] = 98;
		format->tag[2] = 102;
		format->probe = GreyBitFile_Probe;
		format->decodernew = GreyBitFile_Decoder_New;
	}
	return format;
}
#endif //ENABLE_GREYBITFILE