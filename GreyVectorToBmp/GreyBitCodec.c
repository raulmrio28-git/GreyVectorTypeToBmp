#include "GreyBitCodec.h"

GB_INT32	GreyBit_Decoder_SetParam(GB_Decoder decoder, void *pParam)
{
  return decoder->setparam(decoder, pParam);
}

GB_INT32	GreyBit_Decoder_GetCount(GB_Decoder decoder)
{
  return decoder->getcount(decoder);
}

GB_INT32	GreyBit_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
  return decoder->getwidth(decoder, nCode, nSize);
}

GB_INT32	GreyBit_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_DataRec *pData, GB_INT16 nSize)
{
  return decoder->decode(decoder, nCode, pData, nSize);
}

void GreyBit_Decoder_Done(GB_Decoder decoder)
{
  decoder->done(decoder);
}
