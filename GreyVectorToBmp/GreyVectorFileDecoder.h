#ifndef GREYVECTORFILEDECODER_H_
#define GREYVECTORFILEDECODER_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"

#pragma pack(1)
typedef struct tagGREYVECTORFILEHEADER
{
	GB_CHAR gbfTag[4];
	GB_UINT32 gbfSize;
} GREYVECTORFILEHEADER;
#pragma pack()

#pragma pack(1)
typedef struct tagGREYVECTORINFOHEADER
{
	GB_UINT32 gbiSize;
	GB_UINT32 gbiCount;
	GB_INT16 gbiMaxPoints;
	GB_INT16 gbiMaxContours;
	GB_INT16 gbiWidth;
	GB_INT16 gbiHeight;
	GB_UINT32 gbiWidthTabOff;
	GB_UINT32 gbiOffsetTabOff;
	GB_UINT32 gbiOffGreyBits;
	SECTIONOINFO gbiWidthSection;
	SECTIONOINFO gbiIndexSection;
} GREYVECTORINFOHEADER;
#pragma pack()

typedef struct _GVF_DecoderRec
{
	GB_DecoderRec gbDecoder;
	GB_Library gbLibrary;
	GB_Memory gbMem;
	GB_Stream gbStream;
	GB_Outline gbOutline;
	GB_INT32 nCacheItem;
	GB_INT32 nItemCount;
	GB_BYTE *pBuff;
	GB_INT32 nBuffSize;
	GB_UINT32 gbOffDataBits;
	GREYVECTORFILEHEADER gbFileHeader;
	GREYVECTORINFOHEADER gbInfoHeader;
	GB_BYTE *gbWidthTable;
	GB_UINT32 *gbOffsetTable;
	GB_Outline *gpGreyBits;
	GB_INT32 nGreyBitsCount;
} GVF_DecoderRec, *GVF_Decoder;

extern GB_Decoder GreyVectorFile_Decoder_New(GB_Loader loader, GB_Stream stream);
extern GB_INT32 GreyVectorFile_Decoder_SetParam(GB_Decoder decoder, void *pParam);
extern GB_INT32 GreyVectorFile_Decoder_GetCount(GB_Decoder decoder);
extern GB_INT32 GreyVectorFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT32 GreyVectorFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize);
extern void GreyVectorFile_Decoder_Done(GB_Decoder decoder);

#endif //GREYVECTORFILEDECODER_H_