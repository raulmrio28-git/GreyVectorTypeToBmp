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
extern GB_INT32 GreyBit_Decoder_GetHeight(GB_Decoder decoder);
extern GB_INT16	GreyBit_Decoder_GetAdvance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT32	GreyBit_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize);
extern void		GreyBit_Decoder_Done(GB_Decoder decoder);

#ifdef ENABLE_ENCODER
extern GB_INT32	GreyBit_Encoder_SetParam(GB_Encoder encoder, void *pParam);
extern GB_INT32	GreyBit_Encoder_GetCount(GB_Encoder encoder);
extern GB_INT32	GreyBit_Encoder_Delete(GB_Encoder encoder, GB_UINT32 nCode);
extern GB_INT32	GreyBit_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData);
extern GB_INT32 GreyBit_Encoder_Flush(GB_Encoder encoder);
extern void		GreyBit_Encoder_Done(GB_Encoder encoder);
#endif //ENABLE_ENCODER

#ifdef __cplusplus
}
#endif 

#endif //GREYBITCODEC_H_