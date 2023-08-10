#include "GreyBitSystem.h"
#include "GreyVectorFileDecoder.h"
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
  }
  return format;
}
#endif //ENABLE_GREYVECTORFILE