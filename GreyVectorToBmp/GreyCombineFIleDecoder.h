#ifndef GREYCOMBINEFILEDECODER_H_
#define GREYCOMBINEFILEDECODER_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"
#include "GreyCombineFile.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ENABLE_GREYCOMBINEFILE
extern GB_Decoder GreyCombineFile_Decoder_New(GB_Loader loader, GB_Stream stream);
extern GB_INT32 GreyCombineFile_Decoder_SetParam(GB_Decoder decoder, GB_Param nParam, GB_UINT32 dwParam);
extern GB_INT32 GreyCombineFile_Decoder_GetCount(GB_Decoder decoder);
extern GB_INT32 GreyCombineFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT32 GreyCombineFile_Decoder_GetHeight(GB_Decoder decoder);
extern GB_INT16 GreyCombineFile_Decoder_GetAdvance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);	
extern GB_INT32 GreyCombineFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize);
extern void GreyCombineFile_Decoder_Done(GB_Decoder decoder);
#endif //ENABLE_GREYCOMBINEFILE

#ifdef __cplusplus
}
#endif

#endif //GREYCOMBINEFILEDECODER_H_