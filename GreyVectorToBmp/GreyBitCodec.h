#ifndef GREYBITCODEC_H_
#define GREYBITCODEC_H_
#include "GreyBitType.h"
#include "GreyBitSystem.h"
#include "GreyBitType_Def.h"

#ifdef __cplusplus
extern "C" {
#endif

extern GB_INT32	GreyBit_Decoder_SetParam(GB_Decoder decoder, void *pParam);
extern GB_INT32	GreyBit_Decoder_GetCount(GB_Decoder decoder);
extern GB_INT32	GreyBit_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT32	GreyBit_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize);
extern void		GreyBit_Decoder_Done(GB_Decoder decoder);

#ifdef __cplusplus
}
#endif 

#endif //GREYBITCODEC_H_