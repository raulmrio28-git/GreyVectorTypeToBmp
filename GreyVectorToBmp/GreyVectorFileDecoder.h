#ifndef GREYVECTORFILEDECODER_H_
#define GREYVECTORFILEDECODER_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"
#include "GreyVectorFile.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_GREYVECTORFILE
extern GB_Decoder GreyVectorFile_Decoder_New(GB_Loader loader, GB_Stream stream);
extern GB_INT32 GreyVectorFile_Decoder_SetParam(GB_Decoder decoder, void *pParam);
extern GB_INT32 GreyVectorFile_Decoder_GetCount(GB_Decoder decoder);
extern GB_INT32 GreyVectorFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT16 GreyVectorFile_Decoder_Advance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT32 GreyVectorFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize);
extern void GreyVectorFile_Decoder_Done(GB_Decoder decoder);
#endif //ENABLE_GREYVECTORFILE

#ifdef __cplusplus
}
#endif

#endif //GREYVECTORFILEDECODER_H_