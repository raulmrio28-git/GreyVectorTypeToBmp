#include "UnicodeSection.h"
#include "GreyBitType.h"
#include "GreyBitSystem.h"
#include "GreyCombineFileEncoder.h"

#ifdef ENABLE_GREYCOMBINEFILE
#ifdef ENABLE_ENCODER

GB_INT32 nCurrItemCount = 0;
GB_UINT32 nOffset = 0;

extern GBHANDLE GreyBitType_Creator_New_Memory(GBHANDLE library, void *pBuf, GB_INT32 nBufSize);

GB_INT32 GreyCombineFile_Encoder_GetCount(GB_Encoder encoder)
{
	return nCurrItemCount;
}

GB_INT32 GreyCombineFile_Encoder_SetParam(GB_Encoder encoder, void *pParam)
{
	return 0;
}

GB_INT32 GreyCombineFile_Encoder_Delete(GB_Encoder encoder, GB_UINT32 nCode)
{
	GCF_Encoder me = (GCF_Encoder)encoder;
	nCurrItemCount--;
	me->gbFileHeader.gbfInfo[nCurrItemCount].gbiHeight = 0;
	me->gbFileHeader.gbfInfo[nCurrItemCount].gbiDataSize = 0;
	me->gbFileHeader.gbfInfo[nCurrItemCount].gbiDataOff = 0;
	me->gbCreator[nCurrItemCount] = 0;
	return 0;
}

GB_INT32 GreyCombineFile_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData)
{
	GCF_Encoder me = (GCF_Encoder)encoder;
	if (!pData || pData->format != GB_FORMAT_STREAM)
		return -1;
	me->gbCreator[nCurrItemCount] = (GB_Stream)pData->data;
	if (!me->gbCreator[nCurrItemCount]->size)
		return -1;
	if (nCurrItemCount >= GCF_ITEM_MAX)
		return -1;
	me->gbFileHeader.gbfInfo[nCurrItemCount].gbiDataSize = me->gbCreator[nCurrItemCount]->size;
	me->gbFileHeader.gbfInfo[nCurrItemCount].gbiDataOff = nOffset;
	nOffset += me->gbFileHeader.gbfInfo[nCurrItemCount].gbiDataSize;
	nCurrItemCount++;
	return 0;
}

GB_INT32 GreyCombineFile_Encoder_WriteAll(GCF_Encoder me)
{
	GB_INT32 nCurrItem;
	GB_BYTE *pTmp = 0;
	GreyBit_Stream_Seek(me->gbStream, 0);
	GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&me->gbFileHeader, sizeof(GREYCOMBINEFILEHEADER));
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; nCurrItem++)
	{
		if (me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize)
		{
			pTmp = (GB_BYTE*)GreyBit_Malloc(me->gbMem, me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize);
			GreyBit_Stream_Seek(me->gbCreator[nCurrItem], 0);
			GreyBit_Stream_Read(me->gbCreator[nCurrItem], pTmp, me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize);
			GreyBit_Stream_Write(me->gbStream, pTmp, me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize);
			GreyBit_Free(me->gbMem, pTmp);
		}
	}
	return 0;
}

GB_INT32 GreyCombineFile_Encoder_BuildAll(GCF_Encoder me)
{
	GB_INT32 nCurrItem = 0;

	nOffset = sizeof(GREYCOMBINEFILEHEADER);
	me->gbFileHeader.gbfTag[0] = 'g';
	me->gbFileHeader.gbfTag[1] = 'c';
	me->gbFileHeader.gbfTag[2] = 't';
	me->gbFileHeader.gbfTag[3] = 'f';
	return 0;
}

GB_INT32 GreyCombineFile_Encoder_Flush(GB_Encoder encoder)
{
	GB_INT32 nCurrItem = 0;
	GCF_Encoder me = (GCF_Encoder)encoder;

	GreyCombineFile_Encoder_BuildAll(me);
	GreyCombineFile_Encoder_WriteAll(me);
	return 0;
}

void GreyCombineFile_Encoder_Done(GB_Encoder encoder)
{
	GB_INT32 nCurrItem = 0;
	GCF_Encoder me = (GCF_Encoder)encoder;
	GreyCombineFile_Encoder_Flush(encoder);
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; nCurrItem++)
	{
		if (me->gbCreator[nCurrItem])
		{
			GreyBit_Stream_Done(me->gbCreator[nCurrItem]);
			me->gbCreator[nCurrItem] = 0;
		}
	}
	GreyBit_Free(me->gbMem, encoder);
}

GB_Encoder GreyCombineFile_Encoder_New(GB_Creator creator, GB_Stream stream)
{
	GCF_Encoder codec;

	codec = (GCF_Encoder)GreyBit_Malloc(creator->gbMem, sizeof(GCF_Encoder));
	if (codec)
	{
		codec->gbEncoder.getcount = GreyCombineFile_Encoder_GetCount;
		codec->gbEncoder.setparam = GreyCombineFile_Encoder_SetParam;
		codec->gbEncoder.remove = GreyCombineFile_Encoder_Delete;
		codec->gbEncoder.encode = GreyCombineFile_Encoder_Encode;
		codec->gbEncoder.flush = GreyCombineFile_Encoder_Flush;
		codec->gbEncoder.done = GreyCombineFile_Encoder_Done;
		codec->gbLibrary = creator->gbLibrary;
		codec->gbMem = creator->gbMem;
		codec->gbStream = stream;
		GreyBit_Memset_Sys(codec->gbCreator, 0, sizeof(GB_Stream)*GCF_ITEM_MAX);
	}
	return (GB_Encoder)codec;
}
#endif //ENABLE_ENCODER
#endif //ENABLE_GREYCOMBINEFILE