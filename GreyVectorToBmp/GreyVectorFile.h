#ifndef GREYVECTORFILE_H_
#define GREYVECTORFILE_H_
#include "GreyBitType.h"
#include "UnicodeSection.h"
#include "GreyBitType_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_GREYVECTORFILE
typedef struct  _GVF_PointRec
{
	GB_BYTE x;
	GB_BYTE y;
} GVF_PointRec, *GVF_Point;

typedef struct  _GVF_OutlineRec
{
	GB_BYTE        n_contours;      /* number of contours in glyph        */
	GB_BYTE        n_points;        /* number of points in the glyph      */
	GVF_Point        points;          /* the outline's points               */
	GB_BYTE       *contours;        /* the contour end points             */
} GVF_OutlineRec, *GVF_Outline;

#pragma pack(1)
typedef struct tagGREYVECTORFILEHEADER
{
	GB_CHAR gbfTag[4];
	GB_UINT32 gbfSize;
} GREYVECTORFILEHEADER;
#pragma pack()

typedef struct tagV_SECTIONOINFO
{
	GB_UINT16 gbSectionOff[146];
} V_SECTIONOINFO;

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
	V_SECTIONOINFO gbiWidthSection;
	V_SECTIONOINFO gbiIndexSection;
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

typedef struct _GVF_EncoderRec
{
	GB_EncoderRec gbEncoder;
	GB_Library gbLibrary;
	GB_Memory gbMem;
	GB_Stream gbStream;
	GB_GvfParam gbParam;
	GB_BOOL gbInited;
	GB_INT32 nCacheItem;
	GB_INT32 nItemCount;
	GB_UINT32 gbOffDataBits;
	GREYVECTORFILEHEADER gbFileHeader;
	GREYVECTORINFOHEADER gbInfoHeader;
	GB_BYTE *gbWidthTable;
	GB_UINT32 *gbOffsetTable;
	GB_Outline *gpGreyBits;
	GB_UINT16 *pnGreySize;
} GVF_EncoderRec, *GVF_Encoder;

#define TAG_MASK 0x1
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

extern GB_Format GreyVectorFile_Format_New(GB_Library library);
extern GB_BOOL GreyVectorFile_Probe(GB_Stream stream);
#endif //ENABLE_GREYVECTORFILE

#ifdef __cplusplus
}
#endif

#endif //GREYVECTORFILE_H_