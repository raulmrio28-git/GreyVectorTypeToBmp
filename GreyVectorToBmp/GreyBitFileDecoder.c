#include "UnicodeSection.h"
#include "GreyBitSystem.h"
#include "GreyBitFileDecoder.h"

#ifdef ENABLE_GREYBITFILE
GB_UINT32 GreyBitFile_Decoder_GetDataOffset(GBF_Decoder me, GB_UINT32 nCode)
{
	GB_UINT32 nOffset;
	GB_UINT16 nMinCode;
	GB_UINT16 SectionIndex;
	GB_INT32 UniIndex;

	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	if (UniIndex >= UNICODE_SECTION_NUM)
		return 0;
	SectionIndex = me->gbInfoHeader.gbiIndexSection.gbSectionOff[UniIndex];
	if (!SectionIndex)
		return 0;
	SectionIndex--;
	if (me->gbOffsetTable)
	{
		UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
		return me->gbOffsetTable[nCode - nMinCode + SectionIndex];
	}
	else
	{
		UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
		SectionIndex += (GB_UINT16)nCode - nMinCode;
		GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiOffsetTabOff + me->gbOffDataBits + 4 * SectionIndex);
		GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&nOffset, 4);
		return nOffset;
	}
}

GB_INT32 GreyBitFile_Decoder_CaheItem(GBF_Decoder me, GB_UINT32 nCode, GB_BYTE *pData, GB_INT32 nDataSize)
{
	GB_UINT16 nMinCode;
	GB_UINT16 SectionIndex;
	GB_INT32 UniIndex;

	if (me->nGreyBitsCount >= me->nCacheItem || !me->gbOffsetTable)
		return -1;
	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	if (UniIndex >= UNICODE_SECTION_NUM)
		return -1;
	SectionIndex = me->gbInfoHeader.gbiIndexSection.gbSectionOff[UniIndex];
	if (!SectionIndex)
		return -1;
	me->gpGreyBits[me->nGreyBitsCount] = (GB_BYTE*)GreyBit_Malloc(me->gbMem, nDataSize);
	me->pnGreySize[me->nGreyBitsCount] = (GB_INT16)nDataSize;
	GreyBit_Memcpy_Sys(me->gpGreyBits[me->nGreyBitsCount], pData, nDataSize);
	UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
	SectionIndex--;
	SectionIndex += (GB_UINT16)nCode - nMinCode;
	me->gbOffsetTable[SectionIndex] = SET_RAM(me->nGreyBitsCount++);
	return 0;
}

void GreyBitFile_Decoder_ClearCache(GBF_Decoder me)
{
	int i;

	if (me->gbBitmap)
		GreyBitType_Bitmap_Done(me->gbLibrary, me->gbBitmap);
	if (me->pBuff)
		GreyBit_Free(me->gbMem, me->pBuff);
	if (me->gbWidthTable)
		GreyBit_Free(me->gbMem, me->gbWidthTable);
	if (me->gbHoriOffTable)
		GreyBit_Free(me->gbMem, me->gbHoriOffTable);
	if (me->gbOffsetTable)
		GreyBit_Free(me->gbMem, me->gbOffsetTable);
	if (me->pnGreySize)
		GreyBit_Free(me->gbMem, me->pnGreySize);
	if (me->gpGreyBits)
	{
		for (i = 0; i < me->nCacheItem; ++i)
		{
			if (me->gpGreyBits[i])
				GreyBit_Free(me->gbMem, me->gpGreyBits[i]);
		}
		GreyBit_Free(me->gbMem, me->gpGreyBits);
	}
}

GB_INT32 GreyBitFile_Decoder_InfoInit(GBF_Decoder me, GB_INT16 nMaxWidth, GB_INT16 nHeight, GB_INT16 nBitCount)
{
	me->gbBitmap = GreyBitType_Bitmap_New(me->gbLibrary, nMaxWidth, nHeight, nBitCount, 0);
	me->nBuffSize = me->gbBitmap->pitch * me->gbBitmap->height;
	me->pBuff = (GB_BYTE *)GreyBit_Malloc(me->gbMem, me->nBuffSize);
	return 0;
}

GB_INT32 GreyBitFile_Decoder_ReadHeader(GBF_Decoder me)
{
	GreyBit_Stream_Seek(me->gbStream, 0);
	if (GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&me->gbFileHeader, sizeof(GREYBITFILEHEADER)) != sizeof(GREYBITFILEHEADER))
		return -1;
	GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&me->gbInfoHeader, sizeof(GREYBITINFOHEADER));
	me->nItemCount = me->gbInfoHeader.gbiCount;
	GreyBitFile_Decoder_InfoInit(me, me->gbInfoHeader.gbiWidth, me->gbInfoHeader.gbiHeight, me->gbInfoHeader.gbiBitCount);
	return 0;
}

GB_INT32 GreyBitFile_Decoder_Init(GBF_Decoder me)
{
	GB_INT32 nDataSize;
	GB_INT32 nDataSizea;
	GB_INT32 nDataSizeb;
	GB_INT32 nRet;

	nRet = GreyBitFile_Decoder_ReadHeader(me);
	if (nRet < 0)
		return nRet;
	nDataSize = me->gbInfoHeader.gbiHoriOffTabOff - me->gbInfoHeader.gbiWidthTabOff;
	me->gbWidthTable = (GB_BYTE*)GreyBit_Malloc(me->gbMem, nDataSize);
	GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiWidthTabOff + me->gbOffDataBits);
	GreyBit_Stream_Read(me->gbStream, me->gbWidthTable, nDataSize);
	nDataSizea = me->gbInfoHeader.gbiOffsetTabOff - me->gbInfoHeader.gbiHoriOffTabOff;
	me->gbHoriOffTable = (GB_BYTE *)GreyBit_Malloc(me->gbMem, nDataSizea);
	GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiHoriOffTabOff + me->gbOffDataBits);
	GreyBit_Stream_Read(me->gbStream, me->gbHoriOffTable, nDataSizea);
	nDataSizeb = me->gbInfoHeader.gbiOffGreyBits - me->gbInfoHeader.gbiOffsetTabOff;
	me->gbOffsetTable = (GB_UINT32 *)GreyBit_Malloc(me->gbMem, nDataSizeb);
	GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiOffsetTabOff + me->gbOffDataBits);
	GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)me->gbOffsetTable, nDataSizeb);
	return 0;
}

GB_INT32 GreyBitFile_Decoder_SetParam(GB_Decoder decoder, void *pParam)
{
	GBF_Decoder me = (GBF_Decoder)decoder;
	if (pParam)
	{
		if (me->gpGreyBits)
			return -1;
		me->nCacheItem = *(GB_INT32*)pParam;
		me->gpGreyBits = (GB_BYTE **)GreyBit_Malloc(me->gbMem, 4 * me->nCacheItem);
		me->pnGreySize = (GB_UINT16*)GreyBit_Malloc(me->gbMem, 2 * me->nCacheItem);
		me->nGreyBitsCount = 0;
	}
	return 0;
}

GB_INT32 GreyBitFile_Decoder_GetHeight(GB_Decoder decoder)
{
	return 0;
}

GB_INT32 GreyBitFile_Decoder_GetCount(GB_Decoder decoder)
{
	GBF_Decoder me = (GBF_Decoder)decoder;
	return me->nItemCount;
}

GB_INT32 GreyBitFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_BYTE nWidth;
	GB_UINT16 nMinCode;
	GB_INT32 UniIndex;
	GB_INT32 WidthIdx;
	GBF_Decoder me; 

	me = (GBF_Decoder)decoder;
	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	if (UniIndex >= UNICODE_SECTION_NUM)
		return 0;
	WidthIdx = me->gbInfoHeader.gbiWidthSection.gbSectionOff[UniIndex];
	if (!WidthIdx)
		return 0;
	WidthIdx--;
	if (me->gbWidthTable)
	{
		UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
		nWidth = me->gbWidthTable[nCode - nMinCode + WidthIdx];
	}
	else
	{
		UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
		WidthIdx += nCode - nMinCode;
		GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiWidthTabOff + me->gbOffDataBits + WidthIdx);
		GreyBit_Stream_Read(me->gbStream, &nWidth, sizeof(GB_BYTE));
	}
	return nSize * nWidth / me->gbInfoHeader.gbiHeight;
}

GB_INT16 GreyBitFile_Decoder_GetHoriOff(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_BYTE nHoriOff;
	GB_UINT16 nMinCode;
	GB_INT32 UniIndex;
	GB_INT32 HoriOffIdx;
	GBF_Decoder me;

	me = (GBF_Decoder)decoder;
	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	if (UniIndex >= UNICODE_SECTION_NUM)
		return 0;
	HoriOffIdx = me->gbInfoHeader.gbiWidthSection.gbSectionOff[UniIndex];
	if (!HoriOffIdx)
		return 0;
	HoriOffIdx--;
	if (me->gbHoriOffTable)
	{
		UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
		nHoriOff = me->gbHoriOffTable[nCode - nMinCode + HoriOffIdx];
	}
	else
	{
		UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
		HoriOffIdx += nCode - nMinCode;
		GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiHoriOffTabOff + me->gbOffDataBits + HoriOffIdx);
		GreyBit_Stream_Read(me->gbStream, &nHoriOff, sizeof(GB_BYTE));
	}
	return nSize * nHoriOff / me->gbInfoHeader.gbiHeight;
}

GB_INT16 GreyBitFile_Decoder_GetAdvance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_INT16 nWidth;
	GB_INT16 nAdvance;

	nWidth = (GB_INT16)GreyBitFile_Decoder_GetWidth(decoder, nCode, nSize);
	nAdvance = GreyBitFile_Decoder_GetHoriOff(decoder, nCode, nSize) + nWidth;
	if (nAdvance <= 0)
		return 0;
	else
		return nAdvance;
}

GB_INT32 GreyBitFile_Decoder_Decompress(GB_BYTE *pOutData, GB_INT32 *pnInOutLen, GB_BYTE *pInData, GB_INT32 nInDataLen)
{
	GB_INT32 nDecompressLen; 
	GB_INT32 j; 
	GB_INT32 ja; 
	GB_INT32 i;
	GB_INT32 ia;
	GB_BYTE nData; 
	GB_BYTE nDataa;
	GB_BYTE nLen;

	nLen = 0;
	nDecompressLen = 0;
	if (pOutData)
	{
		for (i = 0; i < nInDataLen; ++i)
		{
			nData = pInData[i];
			if (nLen)
			{
				for (j = 0; j < nLen; ++j)
					pOutData[nDecompressLen++] = (nData << 1) | 1;
				nLen = 0;
			}
			else if (IS_LEN(nData))
			{
				nLen = GET_LEN(nData);
			}
			else
			{
				pOutData[nDecompressLen++] = (nData << 1) | 1;
			}
		}
	}
	else
	{
		for (ia = 0; ia < nInDataLen; ++ia)
		{
			nDataa = pInData[ia];
			if (nLen)
			{
				for (ja = 0; ja < nLen; ++ja)
					++nDecompressLen;
				nLen = 0;
			}
			else if (IS_LEN(nDataa))
			{
				nLen = GET_LEN(nDataa);
			}
			else
			{
				++nDecompressLen;
			}
		}
	}
	if (nLen)
		return -1;
	*pnInOutLen = nDecompressLen;
	return 0;
}

GB_INT32 GreyBitFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize)
{
	GB_UINT16 Lenght;
	GB_INT16 nHoriOff;
	GB_INT16 nWidth; 
	GB_INT32 nInDataLen; 
	GB_INT32 nDataLen;
	GB_BYTE *pByteData; 
	GB_UINT32 Offset;
	GBF_Decoder me = (GBF_Decoder)decoder;
	Offset = GreyBitFile_Decoder_GetDataOffset(me, nCode);
	nWidth = (GB_INT16)GreyBitFile_Decoder_GetWidth(decoder, nCode, me->gbInfoHeader.gbiHeight);
	nHoriOff = GreyBitFile_Decoder_GetHoriOff(decoder, nCode, nSize);
	if (!nWidth)
		return -1;
	me->gbBitmap->width = nWidth;
	me->gbBitmap->pitch = (GB_INT16)(me->gbBitmap->bitcount * 8 * nWidth + 63) >> 6;
	me->gbBitmap->horioff = nHoriOff;
	nDataLen = me->gbBitmap->pitch * me->gbBitmap->height;
	if (!IS_INRAM(Offset))
	{
		GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiOffGreyBits + me->gbOffDataBits + Offset);
		if (me->gbInfoHeader.gbiCompression && me->gbInfoHeader.gbiBitCount == 8)
		{
			GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&Lenght, sizeof(GB_UINT16));
			nInDataLen = Lenght;
		}
		else
		{
			nInDataLen = nDataLen;
		}
		GreyBit_Stream_Read(me->gbStream, me->pBuff, nInDataLen);
		pByteData = me->pBuff;
		GreyBitFile_Decoder_CaheItem(me, nCode, pByteData, nInDataLen);
	}
	else
	{
		GET_INDEX(Offset);
		pByteData = me->gpGreyBits[Offset];
		nInDataLen = me->pnGreySize[Offset];
	}
	if (!pByteData)
		return -1;
	if (me->gbInfoHeader.gbiCompression)
		GreyBitFile_Decoder_Decompress(me->gbBitmap->buffer, &nDataLen, pByteData, nInDataLen);
	else
		GreyBit_Memcpy_Sys(me->gbBitmap->buffer, pByteData, nDataLen);
	if (pData)
	{
		pData->format = GB_FORMAT_BITMAP;
		pData->data = me->gbBitmap;
		pData->width = (GB_INT16)nWidth;
		pData->horioff = nHoriOff;
	}
	return 0;
}

void GreyBitFile_Decoder_Done(GB_Decoder decoder)
{
	GBF_Decoder me = (GBF_Decoder)decoder;
	GreyBitFile_Decoder_ClearCache(me);
	GreyBit_Free(me->gbMem, decoder);
}

GB_Decoder GreyBitFile_Decoder_New(GB_Loader loader, GB_Stream stream)
{
	GBF_Decoder decoder; 

	decoder = (GBF_Decoder)GreyBit_Malloc(loader->gbMem, 708);
	if (decoder)
	{
		decoder->gbDecoder.setparam = GreyBitFile_Decoder_SetParam;
		decoder->gbDecoder.getcount = GreyBitFile_Decoder_GetCount;
		decoder->gbDecoder.getwidth = GreyBitFile_Decoder_GetWidth;
		decoder->gbDecoder.getheight = GreyBitFile_Decoder_GetHeight;
		decoder->gbDecoder.getadvance = GreyBitFile_Decoder_GetAdvance;
		decoder->gbDecoder.decode = GreyBitFile_Decoder_Decode;
		decoder->gbDecoder.done = GreyBitFile_Decoder_Done;
		decoder->gbLibrary = loader->gbLibrary;
		decoder->gbMem = loader->gbMem;
		decoder->gbStream = stream;
		decoder->nCacheItem = 0;
		decoder->nItemCount = 0;
		decoder->gbOffDataBits = sizeof(GREYBITFILEHEADER)+sizeof(GREYBITINFOHEADER);
		GreyBitFile_Decoder_Init(decoder);
	}
	return (GB_Decoder)decoder;
}
#endif //ENABLE_GREYBITFILE