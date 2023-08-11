#ifndef GREYCOMBINEFILEENCODER_H_
#define GREYCOMBINEFILEENCODER_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"
#include "GreyCombineFile.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_GREYCOMBINEFILE
#ifdef ENABLE_ENCODER
extern GB_Encoder GreyCombineFile_Encoder_New(GB_Creator creator, GB_Stream stream);
extern GB_INT32 GreyCombineFile_Encoder_SetParam(GB_Encoder encoder, void *pParam);
extern GB_INT32 GreyCombineFile_Encoder_GetCount(GB_Encoder encoder);
extern GB_INT32 GreyCombineFile_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData);
extern void GreyCombineFile_Encoder_Done(GB_Encoder encoder);
#endif //ENABLE_ENCODER
#endif //ENABLE_GREYCOMBINEFILE

#ifdef __cplusplus
}
#endif

#endif //GREYCOMBINEFILEENCODER_H_