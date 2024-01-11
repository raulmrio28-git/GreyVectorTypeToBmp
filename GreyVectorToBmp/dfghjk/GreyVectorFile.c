#include "GreyBitSystem.h"
#include "GreyVectorFileDecoder.h"
#ifdef ENABLE_ENCODER
#include "GreyVectorFileEncoder.h"
#endif //ENABLE_ENCODER
#include "GreyVectorFile.h"

#ifdef ENABLE_GREYVECTORFILE
GB_BOOL GreyVectorFile_Probe(GB_Stream stream)
{
  GREYVECTORFILEHEADER fileHeader; 
  GreyBit_Stream_Seek(stream, 0);
  GreyBit_Stream_Read(stream, (GB_BYTE *)&fileHeader, sizeof(GREYVECTORFILEHEADER));
  return fileHeader.gbfTag[0] == 'g' && fileHeader.gbfTag[1] == 'v' && fileHeader.gbfTag[2] == 't' && fileHeader.gbfTag[3] == 'f';
}

GB_Format GreyVectorFile_Format_New(GB_Library library)
{
  GB_Format format;

  format = (GB_Format)GreyBit_Malloc(library->gbMem, sizeof(GB_FormatRec));
  if ( format )
  {
    format->next = 0;
    format->tag[0] = 103;
    format->tag[1] = 118;
    format->tag[2] = 102;
    format->probe = GreyVectorFile_Probe;
    format->decodernew = GreyVectorFile_Decoder_New;
#ifdef ENCODER_SUPPORT
	format->encodernew = GreyVectorFile_Encoder_New;
#endif //ENCODER_SUPPORT
  }
  return format;
}
#endif //ENABLE_GREYVECTORFILE