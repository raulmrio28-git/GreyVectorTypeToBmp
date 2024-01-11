#include "UnicodeSection.h"
#include "GreyBitSystem.h"
#include "GreyCombineFileDecoder.h"

GB_INT32 GreyCombineFile_Decoder_SetParam(GB_Decoder decoder, GB_Param nParam, GB_UINT32 dwParam)
{
	GB_INT32 nCurrItem;
	GB_INT32 nCurrCount = 0;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader && GreyBit_Decoder_SetParam(gbCurrLoader->gbDecoder, nParam, dwParam) == GB_SUCCESS)
			return GB_SUCCESS;
	}
	return GB_FAILED;
}

GB_INT32 GreyCombineFile_Decoder_GetCount(GB_Decoder decoder)
{
	GB_INT32 nCurrItem;
	GB_INT32 nCurrCount = 0;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader)
			nCurrCount += GreyBit_Decoder_GetCount(gbCurrLoader->gbDecoder);
	}
	return nCurrCount;
}

GB_INT32 GreyCombineFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_INT32 nCurrItem;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader && (me->gbFileHeader.gbfInfo[nCurrItem].gbiHeight == nSize && GreyBitType_Loader_IsExist(me->gbLoader[nCurrItem], nCode)))
			break;
	}
	return GreyBit_Decoder_GetWidth(gbCurrLoader->gbDecoder, nCode, nSize);
}

GB_INT32 GreyCombineFile_Decoder_GetHeight(GB_Decoder decoder)
{
	return 0;
}

GB_INT16  GreyCombineFile_Decoder_GetAdvance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_INT32 nCurrItem;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader && (me->gbFileHeader.gbfInfo[nCurrItem].gbiHeight == nSize && GreyBitType_Loader_IsExist(me->gbLoader[nCurrItem], nCode)))
			break;
	}
	return GreyBit_Decoder_GetAdvance(gbCurrLoader->gbDecoder, nCode, nSize);
}

GB_INT32 GreyCombineFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize)
{
	GB_INT32 nCurrItem; 
	GB_Loader gbCurrLoader; 
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader && (me->gbFileHeader.gbfInfo[nCurrItem].gbiHeight == nSize && GreyBitType_Loader_IsExist(me->gbLoader[nCurrItem], nCode)))
			break;
	}
	return GreyBit_Decoder_Decode(gbCurrLoader->gbDecoder, nCode, pData, nSize);
}

void GreyCombineFile_Decoder_Done(GB_Decoder decoder)
{
	GB_INT32 nCurrItem;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < GCF_ITEM_MAX; ++nCurrItem)
	{
		if (me->gbLoader[nCurrItem])
		{
			GreyBitType_Loader_Done(me->gbLoader[nCurrItem]);
			me->gbLoader[nCurrItem] = 0;
		}
	}
	GreyBit_Free(me->gbMem, decoder);
}

GB_Decoder GreyCombineFile_Decoder_New(GB_Loader loader, GB_Stream stream)
{
	GCF_Decoder decoder;
	GB_INT32 i = 0;

	decoder = (GCF_Decoder)GreyBit_Malloc(loader->gbMem, sizeof(GCF_DecoderRec));
	if (decoder)
	{
		decoder->gbDecoder.setparam = GreyCombineFile_Decoder_SetParam;
		decoder->gbDecoder.getcount = GreyCombineFile_Decoder_GetCount;
		decoder->gbDecoder.getwidth = GreyCombineFile_Decoder_GetWidth;
		decoder->gbDecoder.getheight = GreyCombineFile_Decoder_GetHeight;
		decoder->gbDecoder.getadvance = GreyCombineFile_Decoder_GetAdvance;
		decoder->gbDecoder.decode = GreyCombineFile_Decoder_Decode;
		decoder->gbDecoder.done = GreyCombineFile_Decoder_Done;
		decoder->gbLibrary = loader->gbLibrary;
		decoder->gbMem = loader->gbMem;
		decoder->gbStream = stream;
		GB_MEMSET(decoder->gbLoader, 0, sizeof(GB_Loader)*GCF_ITEM_MAX);
		GreyBit_Stream_Seek(decoder->gbStream, 0);
		if (GreyBit_Stream_Read(decoder->gbStream, (GB_BYTE*)&decoder->gbFileHeader, sizeof(GREYCOMBINEFILEHEADER)) == sizeof(GREYCOMBINEFILEHEADER))
		{
			for (i = 0; i < GCF_ITEM_MAX; i++)
			{
				if (decoder->gbFileHeader.gbfInfo[i].gbiDataSize)
				{
					GreyBit_Stream_Offset(decoder->gbStream, decoder->gbFileHeader.gbfInfo[i].gbiDataOff, 0);
					decoder->gbLoader[i] = (GB_Loader)GreyBitType_Loader_New_Stream(decoder->gbLibrary, decoder->gbStream, decoder->gbFileHeader.gbfInfo[i].gbiDataSize);
				}
			}
		}
	}
	return (GB_Decoder)decoder;
}