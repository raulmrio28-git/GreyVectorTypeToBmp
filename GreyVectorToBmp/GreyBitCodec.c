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

GB_INT32	GreyBit_Decoder_GetAdvance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	return decoder->getadvance(decoder, nCode, nSize);
}

GB_INT32	GreyBit_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize)
{
  return decoder->decode(decoder, nCode, pData, nSize);
}

void GreyBit_Decoder_Done(GB_Decoder decoder)
{
  decoder->done(decoder);
}

#ifdef ENABLE_ENCODER
GB_INT32	GreyBit_Encoder_GetCount(GB_Encoder encoder)
{
	return encoder->getcount(encoder);
}

GB_INT32	GreyBit_Encoder_SetParam(GB_Encoder encoder, void *pParam)
{
	return encoder->setparam(encoder, pParam);
}

GB_INT32	GreyBit_Encoder_Delete(GB_Encoder encoder, GB_UINT32 nCode)
{
	return encoder->remove(encoder, nCode);
}

GB_INT32	GreyBit_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData)
{
	return encoder->encode(encoder, nCode, pData);
}

GB_INT32	GreyBit_Encoder_Flush(GB_Encoder encoder)
{
	return encoder->flush(encoder);
}

void		GreyBit_Encoder_Done(GB_Encoder encoder)
{
	encoder->done(encoder);
}
#endif //ENABLE_ENCODER