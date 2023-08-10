#include "GreyVectorCommon.h"

#ifdef ENABLE_GREYVECTORFILE
GVF_Outline GreyVector_Outline_New(GB_Library library, GB_INT16 n_contours, GB_INT16 n_points)
{
	GVF_Outline outline;

	outline = (GVF_Outline)GreyBit_Malloc(library->gbMem, n_contours + sizeof(GVF_PointRec) * n_points + sizeof(GVF_OutlineRec) + 2);
	if (outline)
	{
		outline->n_contours = (GB_BYTE)n_contours;
		outline->n_points = (GB_BYTE)n_points;
		outline[1].n_contours = (GB_BYTE)n_contours;
		outline[1].n_points = (GB_BYTE)n_points;
		outline->contours = &outline[1].n_points + 1;
		outline->points = (GVF_Point)(outline->contours + n_contours);
	}
	return outline;
}

GVF_Outline GreyVector_Outline_Clone(GB_Library library, GVF_Outline source)
{
	int i;
	int ia; 
	GVF_Outline outline; 

	outline = GreyVector_Outline_New(library, source->n_contours, source->n_points);
	if (outline)
	{
		for (i = 0; i < outline->n_contours; ++i)
			outline->contours[i] = source->contours[i];
		for (ia = 0; ia < outline->n_contours; ++ia)
			outline->points[ia] = source->points[ia];
	}
	return outline;
}

GB_INT32 GreyVector_Outline_GetSizeEx(GB_BYTE n_contours, GB_BYTE n_points)
{
	return n_contours + sizeof(GVF_PointRec) * n_points + sizeof(GVF_OutlineRec) + 2;
}

GVF_Outline GreyVector_Outline_GetData(GVF_Outline outline)
{
	return outline + 1;
}

GVF_Outline GreyVector_Outline_FromData(GB_BYTE *pData)
{
	GVF_Outline outline = (GVF_Outline)pData;

	outline->n_contours = outline[1].n_contours;
	outline->n_points = outline[1].n_points;
	outline->contours = &outline[1].n_points + sizeof(GB_BYTE);
	outline->points = (GVF_Point)((GB_BYTE*)outline->contours + outline->n_contours);
	return outline;
}

void GreyVector_Outline_Done(GB_Library library, GVF_Outline outline)
{
	GreyBit_Free(library->gbMem, outline);
}

GVF_Outline GreyVector_Outline_NewByGB(GB_Library library, GB_Outline source)
{
	int i; 
	int ia;
	GVF_Outline outline; 

	outline = GreyVector_Outline_New(library, source->n_contours, source->n_points);
	if (outline)
	{
		for (i = 0; i < outline->n_contours; ++i)
			outline->contours[i] =  (GB_BYTE)source->contours[i];
		for (ia = 0; ia < outline->n_points; ++ia)
		{
			outline->points[ia].x = (GB_BYTE)source->points[ia].x >> 6;
			outline->points[ia].y = (GB_BYTE)source->points[ia].y >> 6;
			outline->points[ia].x = (GB_BYTE)source->tags[ia] & 1 | (2 * outline->points[ia].x);
		}
	}
	return outline;
}

GB_Outline GreyBitType_Outline_NewByGVF(GB_Library library, GVF_Outline source)
{
	int i; 
	int ia; 
	GB_Outline outline;

	outline = GreyBitType_Outline_New(library->gbMem, source->n_contours, source->n_points);
	if (outline)
	{
		for (i = 0; i < outline->n_contours; ++i)
			outline->contours[i] = source->contours[i];
		for (ia = 0; ia < outline->n_points; ++ia)
		{
			outline->points[ia].x = source->points[ia].x >> 1 << 6;
			outline->points[ia].y = source->points[ia].y << 6;
			outline->tags[ia] = source->points[ia].x & 1;
		}
	}
	return outline;
}

GB_Outline GreyBitType_Outline_UpdateByGVF(GB_Outline outline, GVF_Outline source)
{
	int i;
	int ia;

	outline->n_contours = source->n_contours;
	outline->n_points = source->n_points;
	for (i = 0; i < outline->n_contours; ++i)
		outline->contours[i] = source->contours[i];
	for (ia = 0; ia < outline->n_points; ++ia)
	{
		outline->points[ia].x = source->points[ia].x >> 1 << 6;
		outline->points[ia].y = source->points[ia].y << 6;
		outline->tags[ia] = source->points[ia].x & 1;
	}
	return outline;
}
#endif //ENABLE_GREYVECTORFILE