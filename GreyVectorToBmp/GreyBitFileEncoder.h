#ifndef GREYBITFILEENCODER_H_
#define GREYBITFILEENCODER_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"
#include "GreyBitFile.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_GREYBITFILE
#ifdef ENABLE_ENCODER
extern GB_Encoder GreyBitFile_Encoder_New(GB_Creator creator, GB_Stream stream);
extern GB_INT32 GreyBitFile_Encoder_SetParam(GB_Encoder encoder, void *pParam);
extern GB_INT32 GreyBitFile_Encoder_GetCount(GB_Encoder encoder);
extern GB_INT32 GreyBitFile_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData);
extern void GreyBitFile_Encoder_Done(GB_Encoder encoder);
#endif //ENABLE_ENCODER
#endif //ENABLE_GREYBITFILE

#ifdef __cplusplus
}
#endif

#endif //GREYBITFILEENCODER_H_