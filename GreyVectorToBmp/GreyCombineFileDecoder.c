#include "UnicodeSection.h"
#include "GreyBitSystem.h"
#include "GreyCombineFileDecoder.h"

GB_INT32 GreyCombineFile_Decoder_SetParam(GB_Decoder decoder, void* dwParam)
{
	GB_INT32 nCurrItem;
	GB_INT32 nCurrCount = 0;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < 5; ++nCurrItem)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader)
			GreyBit_Decoder_SetParam(gbCurrLoader->gbDecoder, dwParam);
	}
	return nCurrCount;
}

GB_INT32 GreyCombineFile_Decoder_GetCount(GB_Decoder decoder)
{
	GB_INT32 nCurrItem;
	GB_INT32 nCurrCount = 0;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (nCurrItem = 0; nCurrItem < 5; ++nCurrItem)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader)
			nCurrCount += GreyBit_Decoder_GetCount(gbCurrLoader->gbDecoder);
	}
	return nCurrCount;
}

GB_INT32 GreyCombineFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_INT32 nCurrItem = 0;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	while (1)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader)
		{
			if (me->gbFileHeader.gbfInfo[nCurrItem].gbiHeight == nSize && GreyBitType_Loader_IsExist(me->gbLoader[nCurrItem], nCode))
			{
				break;
			}
		}
		if (++nCurrItem >= GCF_ITEM_MAX)
			return 0;
	}
	return GreyBit_Decoder_GetWidth(gbCurrLoader->gbDecoder, nCode, nSize);
}

GB_INT32 GreyCombineFile_Decoder_GetHeight(GB_Decoder decoder)
{
	return 0;
}

GB_INT16  GreyCombineFile_Decoder_GetAdvance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	int nCurrItem = 0;
	GB_Loader gbCurrLoader;
	GCF_Decoder me = (GCF_Decoder)decoder;

	while (1)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader)
		{
			if (me->gbFileHeader.gbfInfo[nCurrItem].gbiHeight == nSize && GreyBitType_Loader_IsExist(me->gbLoader[nCurrItem], nCode))
			{
				break;
			}
		}
		if (++nCurrItem >= GCF_ITEM_MAX)
			return 0;
	}
	return GreyBit_Decoder_GetAdvance(gbCurrLoader->gbDecoder, nCode, nSize);
}

GB_INT32 GreyCombineFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize)
{
	int nCurrItem = 0; 
	GB_Loader gbCurrLoader; 
	GCF_Decoder me = (GCF_Decoder)decoder;

	while (1)
	{
		gbCurrLoader = me->gbLoader[nCurrItem];
		if (gbCurrLoader)
		{
			if (me->gbFileHeader.gbfInfo[nCurrItem].gbiHeight == nSize && GreyBitType_Loader_IsExist(me->gbLoader[nCurrItem], nCode))
			{
				break;
			}
		}
		if (++nCurrItem >= GCF_ITEM_MAX)
			return 0;
	}
	return GreyBit_Decoder_Decode(gbCurrLoader->gbDecoder, nCode, pData, nSize);
}

void GreyCombineFile_Decoder_Done(GB_Decoder decoder)
{
	int i;
	GCF_Decoder me = (GCF_Decoder)decoder;

	for (i = 0; i < GCF_ITEM_MAX; ++i)
	{
		if (me->gbLoader[i])
		{
			GreyBitType_Loader_Done(me->gbLoader[i]);
			me->gbLoader[i] = 0;
		}
	}
	GreyBit_Free(me->gbMem, decoder);
}

GB_Decoder GreyCombineFile_Decoder_New(GB_Loader loader, GB_Stream stream)
{
	GCF_Decoder decoder;
	GB_INT32 i;

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
		GreyBit_Memset_Sys(decoder->gbLoader, 0, sizeof(GB_Loader)*GCF_ITEM_MAX);
		GreyBit_Stream_Seek(decoder->gbStream, 0);
		if (GreyBit_Stream_Read(decoder->gbStream, (GB_BYTE*)&decoder->gbFileHeader, sizeof(GREYCOMBINEFILEHEADER)) == sizeof(GREYCOMBINEFILEHEADER))
		{
			for (i = 0; i < GCF_ITEM_MAX; i++)
			{
				if (decoder->gbFileHeader.gbfInfo->gbiDataOff)
				{
					GreyBit_Stream_Offset(decoder->gbStream, decoder->gbFileHeader.gbfInfo->gbiDataOff, 0);
					decoder->gbLoader[i] = (GB_Loader)GreyBitType_Loader_New_Stream(decoder->gbLibrary, decoder->gbStream, decoder->gbFileHeader.gbfInfo->gbiDataSize);
				}
			}
		}
	}
	return (GB_Decoder)decoder;
}