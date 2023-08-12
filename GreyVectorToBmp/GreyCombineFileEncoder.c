#include "UnicodeSection.h"
#include "GreyBitType.h"
#include "GreyBitSystem.h"
#include "GreyCombineFileEncoder.h"

#ifdef ENABLE_GREYCOMBINEFILE
#ifdef ENABLE_ENCODER

GB_INT32 nCurrItemCount = 0;
extern GBHANDLE GreyBitType_Creator_New_Memory(GBHANDLE library, void *pBuf, GB_INT32 nBufSize);

GB_INT32 GreyCombineFile_Encoder_GetCount(GB_Encoder encoder)
{
	GB_INT32 nCurrItem;
	GB_INT32 nCurrCount;
	GCF_Encoder me = (GCF_Encoder)encoder;

	nCurrCount = 0;
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		if (me->gbCreator[nCurrItem])
			nCurrCount += GreyBit_Encoder_GetCount(me->gbCreator[nCurrItem]->gbEncoder);
	}
	return nCurrCount;
}

GB_INT32 GreyCombineFile_Encoder_SetParam(GB_Encoder encoder, void *pParam)
{
	GB_INT32 nCurrItem;
	GB_INT32 nReturn;
	GCF_Encoder me = (GCF_Encoder)encoder;
	
	nReturn = 0;
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		if (me->gbCreator[nCurrItem])
			nReturn += GreyBitType_Creator_SetParam(me->gbCreator[nCurrItem], pParam);
	}
	return nReturn;
}

GB_INT32 GreyCombineFile_Encoder_Delete(GB_Encoder encoder, GB_UINT32 nCode)
{
	GB_INT32 nCurrItem;
	GB_INT32 nReturn;
	GCF_Encoder me = (GCF_Encoder)encoder;

	nReturn = 0;
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		if (me->gbCreator[nCurrItem])
			nReturn += GreyBitType_Creator_DelChar(me->gbCreator[nCurrItem], nCode);
	}
	return nReturn;
}

GB_INT32 GreyCombineFile_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData)
{
	GB_INT32 nCurrItem;
	GB_INT32 nReturn;
	GCF_Encoder me = (GCF_Encoder)encoder;

	nReturn = 0;
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		if (me->gbCreator[nCurrItem])
			nReturn += GreyBitType_Creator_SaveChar(me->gbCreator[nCurrItem], nCode, pData);
	}
	return nReturn;
}

GB_INT32 GreyCombineFile_Encoder_WriteAll(GCF_Encoder me)
{
	GB_INT32 nCurrItem;
	GB_BYTE *pTmp = 0;
	GreyBit_Stream_Seek(me->gbStream, 0);
	GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&me->gbFileHeader, sizeof(GREYCOMBINEFILEHEADER));
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; nCurrItem++)
	{
		if (me->gbCreator[nCurrItem])
		{
			pTmp = (GB_BYTE*)GreyBit_Malloc(me->gbMem, me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize);
			GreyBit_Stream_Seek(me->gbCreator[nCurrItem]->gbStream, 0);
			GreyBit_Stream_Read(me->gbCreator[nCurrItem]->gbStream, pTmp, me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize);
			GreyBit_Stream_Write(me->gbStream, pTmp, me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize);
			GreyBit_Free(me->gbMem, pTmp);
		}	
	}
	return 0;
}

GB_INT32 GreyCombineFile_Encoder_BuildAll(GCF_Encoder me)
{
	GB_UINT32 nOffset;
	GB_INT32 nCurrItem = 0;

	nOffset = sizeof(GREYCOMBINEFILEHEADER);
	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; nCurrItem++)
	{
		if (me->gbCreator[nCurrItem])
		{
			me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize = me->gbCreator[nCurrItem]->gbStream->size;
			me->gbFileHeader.gbfInfo[nCurrItem].gbiDataOff = nOffset;
			nOffset += me->gbFileHeader.gbfInfo[nCurrItem].gbiDataSize;
		}
	}
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

	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; nCurrItem++)
	{
		if (me->gbCreator[nCurrItem])
			GreyBitType_Creator_Flush(me->gbCreator[nCurrItem]);
	}
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
			GreyBitType_Creator_Done(me->gbCreator[nCurrItem]);
			me->gbCreator[nCurrItem] = 0;
		}
	}
	GreyBit_Free(me->gbMem, encoder);
}

GB_BOOL GreyCombineFile_Encoder_New_Creator(GB_Encoder encoder, GB_Library library)
{
	GCF_Encoder me = (GCF_Encoder)encoder;
	GB_CHAR *pBuf;
	if (me)
	{
		pBuf = (GB_CHAR*)GreyBit_Malloc(me->gbMem, GCF_BUF_SIZE * GCF_BUF_SIZE * 4);
		if (nCurrItemCount >= GCF_ITEM_MAX)
			return GB_FALSE;
		me->gbCreator[nCurrItemCount] = GreyBitType_Creator_New_Memory(library, pBuf, GCF_BUF_SIZE * GCF_BUF_SIZE * 4);
		if (me->gbCreator[nCurrItemCount])
		{
			nCurrItemCount++;
			return GB_TRUE;
		}	
	}
	return GB_FALSE;
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
		GreyBit_Memset_Sys(codec->gbCreator, 0, sizeof(GB_Creator)*GCF_ITEM_MAX);
	}
	return (GB_Encoder)codec;
}
#endif //ENABLE_ENCODER
#endif //ENABLE_GREYCOMBINEFILE