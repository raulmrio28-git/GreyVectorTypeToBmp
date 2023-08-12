#ifndef GREYBITFILE_H_
#define GREYBITFILE_H_
#include "GreyBitType.h"
#include "UnicodeSection.h"
#include "GreyBitType_Def.h"

#ifdef ENABLE_GREYBITFILE
#pragma pack(1)
typedef struct tagGREYBITFILEHEADER
{
	GB_CHAR gbfTag[4];
} GREYBITFILEHEADER;
#pragma pack()

typedef struct tagB_SECTIONOINFO
{
	GB_UINT16 gbSectionOff[146];
} B_SECTIONOINFO;

#pragma pack(1)
typedef struct tagGREYBITINFOHEADER
{
	GB_UINT32 gbiSize;
	GB_UINT32 gbiCount;
	GB_INT16 gbiBitCount;
	GB_INT16 gbiCompression;
	GB_INT16 gbiWidth;
	GB_INT16 gbiHeight;
	GB_UINT32 gbiWidthTabOff;
	GB_UINT32 gbiHoriOffTabOff;
	GB_UINT32 gbiOffsetTabOff;
	GB_UINT32 gbiOffGreyBits;
	B_SECTIONOINFO gbiWidthSection;
	B_SECTIONOINFO gbiIndexSection;
} GREYBITINFOHEADER;
#pragma pack()

typedef struct GBF_DecoderRec
{
	GB_DecoderRec gbDecoder;
	GB_Library gbLibrary;
	GB_Memory gbMem;
	GB_Stream gbStream;
	GB_Bitmap gbBitmap;
	GB_INT32 nCacheItem;
	GB_INT32 nItemCount;
	GB_BYTE *pBuff;
	GB_INT32 nBuffSize;
	GB_UINT32 gbOffDataBits;
	GREYBITFILEHEADER gbFileHeader;
	GREYBITINFOHEADER gbInfoHeader;
	GB_BYTE *gbWidthTable;
	GB_BYTE *gbHoriOffTable;
	GB_UINT32 *gbOffsetTable;
	GB_BYTE **gpGreyBits;
	GB_INT32 nGreyBitsCount;
	GB_INT16 *pnGreySize;
} GBF_DecoderRec, *GBF_Decoder;

typedef struct GBF_EncoderRec
{
	GB_EncoderRec gbEncoder;
	GB_Library gbLibrary;
	GB_Memory gbMem;
	GB_Stream gbStream;
	GB_GbfParam gbParam;
	GB_BOOL gbInited;
	GB_INT32 nCacheItem;
	GB_INT32 nItemCount;
	GB_UINT32 gbOffDataBits;
	GREYBITFILEHEADER gbFileHeader;
	GREYBITINFOHEADER gbInfoHeader;
	GB_BYTE *gbWidthTable;
	GB_BYTE *gbHoriOffTable;
	GB_UINT32 *gbOffsetTable;
	GB_BYTE **gpGreyBits;
	GB_UINT16 *pnGreySize;
} GBF_EncoderRec, *GBF_Encoder;

#define MAX_COUNT 0x10000
#define LEN_MASK 0x80
#define MAX_LEN() (LEN_MASK-1)
#define GET_LEN(d) (((d)&(~LEN_MASK))+1)
#define SET_LEN(d) ((d)|LEN_MASK)
#define IS_LEN(d) ((d)&LEN_MASK)
#define RAM_MASK 0x80000000
#define IS_INRAM(d) ((d)&RAM_MASK)
#define SET_RAM(d) ((d)|RAM_MASK)
#define GET_INDEX(d) ((d)&(~RAM_MASK))

extern GB_Format GreyBitFile_Format_New(GB_Library library);
extern GB_BOOL GreyBitFile_Probe(GB_Stream stream);
#endif //ENABLE_GREYBITFILE

#endif //GREYBITFILE_H_