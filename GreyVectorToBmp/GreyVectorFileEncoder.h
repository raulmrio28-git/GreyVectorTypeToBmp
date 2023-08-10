#ifndef GREYVECTORFILEENCODER_H_
#define GREYVECTORFILEENCODER_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"
#include "GreyVectorFile.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_GREYVECTORFILE
#ifdef ENABLE_ENCODER
extern GB_Encoder GreyVectorFile_Encoder_New(GB_Creator creator, GB_Stream stream);
extern GB_INT32 GreyVectorFile_Encoder_SetParam(GB_Encoder encoder, void *pParam);
extern GB_INT32 GreyVectorFile_Encoder_GetCount(GB_Encoder encoder);
extern GB_INT32 GreyVectorFile_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData);
extern void GreyVectorFile_Encoder_Done(GB_Encoder encoder);
#endif //ENABLE_ENCODER
#endif //ENABLE_GREYVECTORFILE

#ifdef __cplusplus
}
#endif

#endif //GREYVECTORFILEENCODER_H_