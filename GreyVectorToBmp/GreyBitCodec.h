#ifndef GREYBITCODEC_H_
#define GREYBITCODEC_H_
#include "GreyBitType.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GB_DecoderRec GB_DecoderRec, *GB_Decoder;

typedef GB_INT32   (*GB_SETPARAM)(GB_Decoder decoder, void *pParam);
typedef GB_INT32   (*GB_GETCOUNT)(GB_Decoder decoder);
typedef GB_INT32   (*GB_GETWIDTH)(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
typedef GB_INT32   (*GB_DECODE)(GB_Decoder decoder, GB_UINT32 nCode, GB_DataRec *pData, GB_INT16 nSize);
typedef void	   (*GB_DONE)(GB_Decoder decoder);

struct _GB_DecoderRec
{
  GB_SETPARAM setparam;
  GB_GETCOUNT getcount;
  GB_GETWIDTH getwidth;
  GB_DECODE decode;
  GB_DONE done;
};

extern GB_INT32	GreyBit_Decoder_SetParam(GB_Decoder decoder, void *pParam);
extern GB_INT32	GreyBit_Decoder_GetCount(GB_Decoder decoder);
extern GB_INT32	GreyBit_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize);
extern GB_INT32	GreyBit_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_DataRec *pData, GB_INT16 nSize);
extern void		GreyBit_Decoder_Done(GB_Decoder decoder);

#ifdef __cplusplus
}
#endif 

#endif //GREYBITCODEC_H_