#include "GreyBitSystem.h"
#include "GreyVectorFileDecoder.h"
#include "GreyVectorFile.h"

GB_BOOL GreyVectorFile_Probe(GB_Stream stream)
{
  GREYVECTORFILEHEADER fileHeader; 

  GreyBit_Stream_Seek(stream, 0);
  GreyBit_Stream_Read(stream, (GB_BYTE *)&fileHeader, 8);
  return GB_MEMCMP("gvtf", fileHeader.gbfTag, 4) != 0;
}

GB_Format GreyVectorFile_Format_New(GB_Library library)
{
  GB_Format format;

  format = (GB_Format)GreyBit_Malloc(library->gbMem, 16);
  if ( format )
  {
    format->next = 0;
    format->tag[0] = 103;
    format->tag[1] = 118;
    format->tag[2] = 102;
    format->probe = GreyVectorFile_Probe;
    format->decodernew = GreyVectorFile_Decoder_New;
  }
  return format;
}