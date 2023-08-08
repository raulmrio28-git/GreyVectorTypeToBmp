#ifndef GREYVECTORCOMMON_H_
#define GREYVECTORCOMMON_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"
#include "GreyVectorFile.h"

extern GVF_Outline GreyVector_Outline_New(GB_Library library, GB_INT16 n_contours, GB_INT16 n_points);
extern GVF_Outline GreyVector_Outline_Clone(GB_Library library, GVF_Outline source);
extern GB_INT32 GreyVector_Outline_GetSizeEx(GB_BYTE n_contours, GB_BYTE n_points);
extern GVF_Outline GreyVector_Outline_GetData(GVF_Outline outline);
extern GVF_Outline GreyVector_Outline_FromData(GB_BYTE *pData);
extern void GreyVector_Outline_Done(GB_Library library, GVF_Outline outline);
extern GVF_Outline GreyVector_Outline_NewByGB(GB_Library library, GB_Outline source);
extern GB_Outline GreyBitType_Outline_NewByGVF(GB_Library library, GVF_Outline source);
extern GB_Outline GreyBitType_Outline_UpdateByGVF(GB_Outline outline, GVF_Outline source);

#endif //GREYVECTORCOMMON_H_