#include "UnicodeSection.h"
#include "GreyBitSystem.h"
#include "GreyVectorCommon.h"
#include "GreyVectorFileDecoder.h"

#ifdef ENABLE_GREYVECTORFILE
GB_UINT32 GreyVectorFile_Decoder_GetDataOffset(GVF_Decoder me, GB_UINT32 nCode)
{
	GB_UINT32 nOffset;
	GB_UINT16 nMinCode;
	GB_UINT16 SectionIndex;
	GB_INT32 UniIndex;

	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	SectionIndex = me->gbInfoHeader.gbiIndexSection.gbSectionOff[UniIndex];
	if (UniIndex >= UNICODE_SECTION_NUM)
		return 0;
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

GB_INT32 GreyVectorFile_Decoder_CaheItem(GVF_Decoder me, GB_UINT32 nCode, GB_Outline outline)
{
	GB_UINT16 nMinCode;
	GB_UINT16 SectionIndex;
	GB_INT32 UniIndex;

	if (me->nGreyBitsCount >= me->nCacheItem || !me->gbOffsetTable)
		return GB_FAILED;
	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	if (UniIndex >= UNICODE_SECTION_NUM)
		return GB_FAILED;
	SectionIndex = me->gbInfoHeader.gbiIndexSection.gbSectionOff[UniIndex];
	if (!SectionIndex)
		return GB_FAILED;
	me->gpGreyBits[me->nGreyBitsCount] = GreyBitType_Outline_Clone(me->gbLibrary, outline);
	UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
	SectionIndex--;
	SectionIndex += (GB_UINT16)nCode - nMinCode;
	me->gbOffsetTable[SectionIndex] = SET_RAM(me->nGreyBitsCount++);
	return GB_SUCCESS;
}

void GreyVectorFile_Decoder_ClearCache(GVF_Decoder me)
{
	int i;

	if (me->gbOutline)
		GreyBitType_Outline_Done(me->gbLibrary, me->gbOutline);
	if (me->pBuff)
		GreyBit_Free(me->gbMem, me->pBuff);
	if (me->gbWidthTable)
		GreyBit_Free(me->gbMem, me->gbWidthTable);
	if (me->gbHoriOffTable)
		GreyBit_Free(me->gbMem, me->gbHoriOffTable);
	if (me->gbOffsetTable)
		GreyBit_Free(me->gbMem, me->gbOffsetTable);
	if (me->gpGreyBits)
	{
		for (i = 0; i < me->nCacheItem; ++i)
		{
			if (me->gpGreyBits[i])
				GreyBitType_Outline_Done(me->gbLibrary, me->gpGreyBits[i]);
		}
		GreyBit_Free(me->gbMem, me->gpGreyBits);
	}
}

GB_INT32 GreyVectorFile_Decoder_InfoInit(GVF_Decoder me, GB_INT16 nMaxWidth, GB_INT16 nHeight, GB_INT16 nMaxPoints, GB_INT16 nMaxContours)
{
	me->gbOutline = GreyBitType_Outline_New(me->gbLibrary, nMaxContours, nMaxPoints);
	me->nBuffSize = GreyVector_Outline_GetSizeEx((GB_BYTE)nMaxContours, (GB_BYTE)nMaxPoints) + sizeof(GVF_OutlineRec);
	me->pBuff = (GB_BYTE *)GreyBit_Malloc(me->gbMem, me->nBuffSize);
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Decoder_ReadHeader(GVF_Decoder me)
{
	GreyBit_Stream_Seek(me->gbStream, 0);
	if (GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&me->gbFileHeader, sizeof(GREYVECTORFILEHEADER)) != sizeof(GREYVECTORFILEHEADER))
		return GB_FAILED;
	GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&me->gbInfoHeader, sizeof(GREYVECTORINFOHEADER));
	me->nItemCount = me->gbInfoHeader.gbiCount;
	GreyVectorFile_Decoder_InfoInit(me, me->gbInfoHeader.gbiWidth, me->gbInfoHeader.gbiHeight, me->gbInfoHeader.gbiMaxPoints, me->gbInfoHeader.gbiMaxContours);
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Decoder_Init(GVF_Decoder me)
{
	int nDataSize;
	int nDataSizea;
	int nDataSizeb;
	int nRet;

	GreyBit_Stream_Seek(me->gbStream, 0);
	nRet = GreyVectorFile_Decoder_ReadHeader(me);
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
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Decoder_SetParam(GB_Decoder decoder, GB_Param nParam, GB_UINT32 dwParam)
{
	GVF_Decoder me = (GVF_Decoder)decoder;
	if (dwParam)
	{
		if (nParam == GB_PARAM_CACHEITEM)
		{
			if (me->gpGreyBits)
				return GB_FAILED;
			me->nCacheItem = dwParam;
			me->gpGreyBits = (GB_Outline*)GreyBit_Malloc(me->gbMem, 4 * me->nCacheItem);
			me->nGreyBitsCount = 0;
		}
	}
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Decoder_GetHeight(GB_Decoder decoder)
{
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Decoder_GetCount(GB_Decoder decoder)
{
	GVF_Decoder me = (GVF_Decoder)decoder;
	return me->nItemCount;
}

GB_INT32 GreyVectorFile_Decoder_GetWidth(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_BYTE nWidth;
	GB_UINT16 nMinCode;
	GB_INT32 UniIndex;
	GB_INT32 WidthIdx;
	GVF_Decoder me = (GVF_Decoder)decoder;
	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	WidthIdx = me->gbInfoHeader.gbiWidthSection.gbSectionOff[UniIndex];
	if (UniIndex >= UNICODE_SECTION_NUM)
		return 0;
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

GB_INT16 GreyVectorFile_Decoder_GetHoriOff(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_BYTE nHoriOff;
	GB_UINT16 nMinCode;
	GB_INT32 UniIndex;
	GB_INT32 HoriOffIdx;
	GVF_Decoder me;

	me = (GVF_Decoder)decoder;
	UniIndex = UnicodeSection_GetIndex((GB_UINT16)nCode);
	HoriOffIdx = me->gbInfoHeader.gbiWidthSection.gbSectionOff[UniIndex];
	if (UniIndex >= UNICODE_SECTION_NUM)
		return 0;
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

GB_INT16 GreyVectorFile_Decoder_GetAdvance(GB_Decoder decoder, GB_UINT32 nCode, GB_INT16 nSize)
{
	GB_INT16 nWidth;
	GB_INT16 nAdvance;

	nWidth = (GB_INT16)GreyVectorFile_Decoder_GetWidth(decoder, nCode, nSize);
	nAdvance = GreyVectorFile_Decoder_GetHoriOff(decoder, nCode, nSize) + nWidth;
	if (nAdvance <= 0)
		return 0;
	else
		return nAdvance;
}

GB_INT32 GreyVectorFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize)
{
	GB_UINT16 Lenght; 
	GB_INT32 nWidth;
	GB_INT32 nHoriOff;
	GB_Outline outline; 
	GB_UINT32 Offset;
	GVF_Decoder me = (GVF_Decoder)decoder;
	Offset = GreyVectorFile_Decoder_GetDataOffset(me, nCode);
	nWidth = GreyVectorFile_Decoder_GetWidth(decoder, nCode, nSize);
	nHoriOff = GreyVectorFile_Decoder_GetHoriOff(decoder, nCode, nSize);
	if (!nWidth)
		return GB_FAILED;
	if (!IS_INRAM(Offset))
	{
		GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiOffGreyBits + me->gbOffDataBits + Offset);
		GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&Lenght, sizeof(Lenght));
		GreyBit_Stream_Read(me->gbStream, me->pBuff + sizeof(GVF_OutlineRec), Lenght);
		outline = GreyBitType_Outline_UpdateByGVF(me->gbOutline, GreyVector_Outline_FromData(me->pBuff));
		GreyVectorFile_Decoder_CaheItem(me, nCode, outline);
	}
	else
	{
		GET_INDEX(Offset);
		outline = me->gpGreyBits[Offset];
	}
	if (!outline)
		return GB_FAILED;
	GreyBitType_Outline_Transform(me->gbOutline, outline, nSize, me->gbInfoHeader.gbiHeight);
	if (pData)
	{
		pData->format = GB_FORMAT_OUTLINE;
		pData->data = me->gbOutline;
		pData->width = (GB_INT16)nWidth;
		pData->horioff = (GB_INT16)nHoriOff;
	}
	return GB_SUCCESS;
}

void GreyVectorFile_Decoder_Done(GB_Decoder decoder)
{
	GVF_Decoder me = (GVF_Decoder)decoder;
	GreyVectorFile_Decoder_ClearCache(me);
	GreyBit_Free(me->gbMem, decoder);
}

GB_Decoder GreyVectorFile_Decoder_New(GB_Loader loader, GB_Stream stream)
{
	GVF_Decoder decoder;

	decoder = (GVF_Decoder)GreyBit_Malloc(loader->gbMem, sizeof(GVF_DecoderRec));
	if (decoder)
	{
		decoder->gbDecoder.setparam = GreyVectorFile_Decoder_SetParam;
		decoder->gbDecoder.getcount = GreyVectorFile_Decoder_GetCount;
		decoder->gbDecoder.getwidth = GreyVectorFile_Decoder_GetWidth;
		decoder->gbDecoder.getheight = GreyVectorFile_Decoder_GetHeight;
		decoder->gbDecoder.getadvance = GreyVectorFile_Decoder_GetAdvance;
		decoder->gbDecoder.decode = GreyVectorFile_Decoder_Decode;
		decoder->gbDecoder.done = GreyVectorFile_Decoder_Done;
		decoder->gbLibrary = loader->gbLibrary;
		decoder->gbMem = loader->gbMem;
		decoder->gbStream = stream;
		decoder->nCacheItem = 0;
		decoder->nItemCount = 0;
		decoder->gbOffDataBits = sizeof(GREYVECTORFILEHEADER) + sizeof(GREYVECTORINFOHEADER);
		GreyVectorFile_Decoder_Init(decoder);
	}
	return (GB_Decoder)decoder;
}
#endif //ENABLE_GREYVECTORFILE