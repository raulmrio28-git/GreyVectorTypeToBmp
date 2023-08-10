#ifndef GREYBITFILEDECODER_H_
#define GREYBITFILEDECODER_H_
#include "GreyBitType.h"
#include "GreyBitCodec.h"
#include "GreyBitFile.h"

#ifdef ENABLE_GREYBITFILE
extern GB_Decoder GreyBitFile_Decoder_New(GB_Loader loader, GB_Stream stream);
extern GB_INT32 GreyBitFile_Decoder_SetParam(GB_Decoder decoder, void *pParam);
extern GB_INT32 GreyBitFile_Decoder_GetCount(GB_Decoder decoder);
extern GB_INT32 GreyBitFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT32 GreyBitFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize);
extern void GreyBitFile_Decoder_Done(GB_Decoder decoder);
#endif //ENABLE_GREYBITFILE

#endif //GREYBITFILEDECODER_H_