#ifndef GREYBITTYPE_H_
#define GREYBITTYPE_H_
#ifdef __cplusplus
extern "C"{
#endif

#ifdef WIN32
//#define ENABLE_TRUETYPEFILE
#define ENABLE_ENCODER
#endif
#define ENABLE_GREYCOMBINEFILE
#define ENABLE_GREYBITFILE
#define ENABLE_GREYVECTORFILE
#define ENABLE_ITALIC
#define ENABLE_BOLD

#define GB_CURVE_TAG( flag )  ( flag & 3 )

#define GB_CURVE_TAG_ON            1
#define GB_CURVE_TAG_CONIC         0
#define GB_CURVE_TAG_CUBIC         2

#define GB_TRUE                    1
#define GB_FALSE                   0
#define GB_SUCCESS                 0
#define GB_FAILED                  1
#define GB_WIDTH_DEFAULT           0
#define GB_HORIOFF_DEFAULT         0

typedef void*           GBHANDLE;
typedef char            GB_BOOL;
typedef unsigned char   GB_BYTE;
typedef unsigned short  GB_UINT16;
typedef unsigned long   GB_UINT32;
typedef signed short    GB_INT16;
typedef signed long     GB_INT32;
typedef GB_INT16        GB_Pos;
typedef char            GB_CHAR;
typedef signed char     GB_INT8;

typedef enum {
	GB_FORMAT_NONE,
	GB_FORMAT_BITMAP,
	GB_FORMAT_OUTLINE,
	GB_FORMAT_STREAM,
	GB_FROMAT_MAX
}GB_DataFormat;

typedef enum {
	GB_PARAM_NONE,
	GB_PARAM_CACHEITEM,     // Cached item number
#ifdef ENABLE_ENCODER
	GB_PARAM_HEIGHT,        // font height
	GB_PARAM_BITCOUNT,      // GBF bit count
	GB_PARAM_COMPRESS,      // Whether of not compress
#endif
	GB_PARAM_MAX
}GB_Param;

typedef struct  _GB_DataRec
{
    GB_DataFormat   format;
    GB_INT16        width;
	GB_INT16		horioff;
    void           *data;
} GB_DataRec, *GB_Data;

typedef struct  _GB_BitmapRec
{
    GB_INT16         width;
    GB_INT16         height;
	GB_INT16         horioff;
    GB_INT16         pitch;
    GB_INT16         bitcount;
    GB_BYTE         *buffer;
} GB_BitmapRec, *GB_Bitmap;

typedef struct  _GB_PointRec
{
    GB_Pos x;
    GB_Pos y;
} GB_PointRec, *GB_Point;

typedef struct  _GB_OutlineRec
{
    GB_INT16        n_contours;      /* number of contours in glyph        */
    GB_INT16        n_points;        /* number of points in the glyph      */
    GB_Point        points;          /* the outline's points               */
    GB_BYTE*        tags;            /* the points flags                   */
    GB_INT16       *contours;        /* the contour end points             */
} GB_OutlineRec, *GB_Outline;

/*************************************************************************/
/*                                                                       */
/*                                  API                                  */
/*                                                                       */
/*************************************************************************/
// Library
extern GBHANDLE     GreyBitType_Init(void);
extern void         GreyBitType_Done(GBHANDLE library);

// Bitmap
extern GB_Bitmap    GreyBitType_Bitmap_New(GBHANDLE library, GB_INT16 nWidth, GB_INT16 nHeight, GB_INT16 bitcount, GB_BYTE *pInitBuf);
extern void         GreyBitType_Bitmap_Done(GBHANDLE library, GB_Bitmap bitmap);

// Outline
extern GB_Outline   GreyBitType_Outline_New(GBHANDLE library, GB_INT16 n_contours, GB_INT16 n_points);
extern GB_Outline   GreyBitType_Outline_Clone(GBHANDLE library, GB_Outline source);
extern int          GreyBitType_Outline_Remove(GB_Outline outline, GB_INT16 idx);
extern GB_INT32     GreyBitType_Outline_GetSize(GB_Outline outline);
extern GB_INT32     GreyBitType_Outline_GetSizeEx(GB_INT16 n_contours, GB_INT16 n_points);
extern int          GreyBitType_Outline_Transform(GB_Outline outline, GB_Outline source, GB_INT16 tosize, GB_INT16 fromsize);
extern void         GreyBitType_Outline_Done(GBHANDLE library, GB_Outline outline);

#ifdef ENABLE_ENCODER
// Creator
extern GBHANDLE     GreyBitType_Creator_New(GBHANDLE library, const GB_CHAR* filepathname);
//extern int          GreyBitType_Creator_SetParam(GBHANDLE creator, void *pParam);
extern int          GreyBitType_Creator_SetParam(GBHANDLE creator, GB_Param nParam, GB_UINT32 dwParam);
extern int          GreyBitType_Creator_DelChar(GBHANDLE creator, GB_UINT32 nCode);
extern int          GreyBitType_Creator_SaveChar(GBHANDLE creator, GB_UINT32 nCode, GB_Data pData);
extern int          GreyBitType_Creator_Flush(GBHANDLE creator);
extern void         GreyBitType_Creator_Done(GBHANDLE creator);
#endif

// Loader
extern GBHANDLE     GreyBitType_Loader_New(GBHANDLE library, const GB_CHAR* filepathname);
extern GBHANDLE     GreyBitType_Loader_New_Stream(GBHANDLE library, GBHANDLE stream, GB_INT32 size);
extern GBHANDLE     GreyBitType_Loader_New_Memory(GBHANDLE library, void *pBuf, GB_INT32 nBufSize);
extern GB_INT32     GreyBitType_Loader_GetCount(GBHANDLE loader);
extern GB_INT32     GreyBitType_Loader_GetHeight(GBHANDLE loader);
extern int          GreyBitType_Loader_SetParam(GBHANDLE loader,  GB_Param nParam, GB_UINT32 dwParam);
//extern int          GreyBitType_Loader_SetParam(GBHANDLE loader, void* pParam);
extern GB_BOOL      GreyBitType_Loader_IsExist(GBHANDLE loader, GB_UINT32 nCode);
extern void         GreyBitType_Loader_Done(GBHANDLE loader);

// Layout
extern GBHANDLE     GreyBitType_Layout_New(GBHANDLE loader, GB_INT16 nSize, GB_INT16 nBitCount, GB_BOOL bBold, GB_BOOL bItalic);
extern GB_INT32     GreyBitType_Layout_GetWidth(GBHANDLE layout, GB_UINT32 nCode);
extern int          GreyBitType_Layout_LoadChar(GBHANDLE layout, GB_UINT32 nCode, GB_Bitmap *pBmp);
extern void         GreyBitType_Layout_Done(GBHANDLE layout);

#ifdef __cplusplus
}
#endif 
#endif
