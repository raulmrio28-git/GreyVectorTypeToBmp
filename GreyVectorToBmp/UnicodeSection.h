#ifndef UNICODESECTION_H_
#define UNICODESECTION_H_
#include "GreyBitType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagUNICODESECTION
{
	GB_UINT16 nMinCode;
	GB_UINT16 nMaxCode;
} UNICODESECTION;

extern GB_INT32	UnicodeSection_GetIndex(GB_UINT16 nCode);
extern GB_INT32	UnicodeSection_GetSectionNum(GB_INT32 index);
extern void		UnicodeSection_GetSectionInfo(GB_INT32 index, GB_UINT16 *pMinCode, GB_UINT16 *pMaxCode);

#ifdef __cplusplus
}
#endif 

#endif //UNICODESECTION_H_
