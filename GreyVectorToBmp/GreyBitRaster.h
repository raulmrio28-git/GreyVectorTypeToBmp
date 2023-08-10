#ifndef GREYBITRASTER_H_
#define GREYBITRASTER_H_
#include "GreyBitType.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_GREYVECTORFILE
#define DEFAULT_POOL_SIZE (16384)

typedef struct GB_BBox_
{
	GB_Pos  xMin, yMin;
	GB_Pos  xMax, yMax;
} GB_BBox;

typedef struct _RST_Vector
{
	int x;
	int y;
} RST_Vector;

#define GB_Vector GB_PointRec

typedef int(*GB_Outline_MoveToFunc)(const GB_Vector* to, void* user);
#define GB_Outline_MoveTo_Func GB_Outline_MoveToFunc

typedef int(*GB_Outline_LineToFunc)(const GB_Vector* to, void* user);
#define GB_Outline_LineTo_Func GB_Outline_LineToFunc

typedef int(*GB_Outline_ConicToFunc)(const GB_Vector* control, const GB_Vector* to, void* user);
#define GB_Outline_ConicTo_Func GB_Outline_ConicToFunc

typedef int(*GB_Outline_CubicToFunc)(const GB_Vector* control1, const GB_Vector* control2, const GB_Vector* to, void* user);
#define GB_Outline_CubicTo_Func GB_Outline_CubicToFunc

typedef struct  GB_Outline_Funcs_
{
	GB_Outline_MoveToFunc   move_to;
	GB_Outline_LineToFunc   line_to;
	GB_Outline_ConicToFunc  conic_to;
	GB_Outline_CubicToFunc  cubic_to;

	int                     shift;
	GB_Pos                  delta;

} GB_Outline_Funcs;

typedef struct GB_Span_
{
	short           x;
	unsigned short  len;
	unsigned char   coverage;
} GB_Span;

typedef void(*GB_SpanFunc)(int y, int count, const GB_Span* spans,void* user);
#define GB_Raster_Span_Func GB_SpanFunc

extern void* GreyBit_Raster_New(GB_Library library, int nPoolSize);
extern int GreyBit_Raster_Render(void *raster, GB_Bitmap tobitmap, GB_Outline fromoutline);
extern void GreyBit_Raster_Done(void *raster);
#endif //ENABLE_GREYVECTORFILE

#ifdef __cplusplus
}
#endif

#endif //GREYBITRASTER_H_