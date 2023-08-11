#ifndef GREYCOMBINEFILE_H_
#define GREYCOMBINEFILE_H_
#include "GreyBitType.h"
#include "UnicodeSection.h"
#include "GreyBitType_Def.h"

#ifdef ENABLE_GREYCOMBINEFILE

#define GCF_ITEM_MAX 5
#define GCF_BUF_SIZE 1024

#pragma pack(1)
typedef struct tagGREYCOMBINEITEMINFO
{
	GB_UINT32 gbiHeight;
	GB_UINT32 gbiDataOff;
	GB_UINT32 gbiDataSize;
} GREYCOMBINEITEMINFO;
#pragma pack()

#pragma pack(1)
typedef struct tagGREYCOMBINEFILEHEADER
{
	GB_CHAR gbfTag[4];
	GREYCOMBINEITEMINFO gbfInfo[GCF_ITEM_MAX];
} GREYCOMBINEFILEHEADER;
#pragma pack()

typedef struct _GCF_DecoderRec
{
	GB_DecoderRec gbDecoder;
	GB_Library gbLibrary;
	GB_Memory gbMem;
	GB_Stream gbStream;
	GREYCOMBINEFILEHEADER gbFileHeader;
	GB_Loader gbLoader[GCF_ITEM_MAX];
} GCF_DecoderRec, *GCF_Decoder;

#ifdef ENABLE_ENCODER
typedef struct _GCF_EncoderRec
{
	GB_EncoderRec gbEncoder;
	GB_Library gbLibrary;
	GB_Memory gbMem;
	GB_Stream gbStream;
	GREYCOMBINEFILEHEADER gbFileHeader;
	GB_Creator gbCreator[GCF_ITEM_MAX];
} GCF_EncoderRec, *GCF_Encoder;
#endif //ENABLE_ENCODER

extern GB_Format GreyCombineFile_Format_New(GB_Library library);
extern GB_BOOL GreyCombineFile_Probe(GB_Stream stream);
#endif //ENABLE_GREYCOMBINEFILE

#endif //GREYCOMBINEFILE_H_