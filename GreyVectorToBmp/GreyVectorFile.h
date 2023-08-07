#include "GreyBitType.h"
#include "GreyBitSystem.h"
#include "GreyBitCodec.h"

typedef struct tagGREYVECTORFILEHEADER
{
  GB_CHAR gbfTag[4];
  GB_UINT32 gbfSize;
} GREYVECTORFILEHEADER;