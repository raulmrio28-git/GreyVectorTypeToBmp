#include "UnicodeSection.h"
#include "GreyBitSystem.h"
#include "GreyVectorCommon.h"
#include "GreyVectorFileDecoder.h"

GB_UINT32 GreyVectorFile_Decoder_GetDataOffset(GVF_Decoder me, GB_UINT32 nCode)
{
	GB_UINT32 nOffset;
	GB_UINT16 nMinCode;
	GB_UINT16 SectionIndex;
	GB_INT32 UniIndex;

	UniIndex = UnicodeSection_GetIndex(nCode);
	if (UniIndex >= 146)
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
		SectionIndex += nCode - nMinCode;
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
		return -1;
	UniIndex = UnicodeSection_GetIndex(nCode);
	if (UniIndex >= 146)
		return -1;
	SectionIndex = me->gbInfoHeader.gbiIndexSection.gbSectionOff[UniIndex];
	if (!SectionIndex)
		return -1;
	me->gpGreyBits[me->nGreyBitsCount] = GreyBitType_Outline_Clone(me->gbLibrary, outline);
	UnicodeSection_GetSectionInfo(UniIndex, &nMinCode, 0);
	SectionIndex--;
	SectionIndex += nCode - nMinCode;
	me->gbOffsetTable[SectionIndex] = me->nGreyBitsCount++ | 0x80000000;
	return 0;
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
	me->nBuffSize = GreyVector_Outline_GetSizeEx(nMaxContours, nMaxPoints) + 12;
	me->pBuff = (GB_BYTE *)GreyBit_Malloc(me->gbMem, me->nBuffSize);
	return 0;
}

GB_INT32 GreyVectorFile_Decoder_ReadHeader(GVF_Decoder me)
{
	GreyBit_Stream_Seek(me->gbStream, 0);
	if (GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&me->gbFileHeader, 8) != 8)
		return -1;
	if (me->gbFileHeader.gbfSize != me->gbStream->size)
		return -1;
	GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&me->gbInfoHeader, 612);
	me->nItemCount = me->gbInfoHeader.gbiCount;
	GreyVectorFile_Decoder_InfoInit(me, me->gbInfoHeader.gbiWidth, me->gbInfoHeader.gbiHeight, me->gbInfoHeader.gbiMaxPoints, me->gbInfoHeader.gbiMaxContours);
	return 0;
}

GB_INT32 GreyVectorFile_Decoder_Init(GVF_Decoder me)
{
	int nDataSize;
	int nDataSizea;
	int nRet;

	nRet = GreyVectorFile_Decoder_ReadHeader(me);
	if (nRet < 0)
		return nRet;
	nDataSize = me->gbInfoHeader.gbiOffsetTabOff - me->gbInfoHeader.gbiWidthTabOff;
	me->gbWidthTable = (GB_BYTE *)GreyBit_Malloc(me->gbMem, nDataSize);
	GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiWidthTabOff + me->gbOffDataBits);
	GreyBit_Stream_Read(me->gbStream, me->gbWidthTable, nDataSize);
	nDataSizea = me->gbInfoHeader.gbiOffGreyBits - me->gbInfoHeader.gbiOffsetTabOff;
	me->gbOffsetTable = (GB_UINT32 *)GreyBit_Malloc(me->gbMem, nDataSizea);
	GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiOffsetTabOff + me->gbOffDataBits);
	GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)me->gbOffsetTable, nDataSizea);
	return 0;
}

GB_INT32 GreyVectorFile_Decoder_SetParam(GB_Decoder decoder, void *pParam)
{
	GVF_Decoder me = (GVF_Decoder)decoder;
	if (pParam)
	{
		if (me->gpGreyBits)
			return -1;
		me->nCacheItem = &pParam;
		me->gpGreyBits = (GB_Outline*)GreyBit_Malloc(me->gbMem, 4 * me->nCacheItem);
		me->nGreyBitsCount = 0;
	}
	return 0;
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
	UniIndex = UnicodeSection_GetIndex(nCode);
	if (UniIndex >= 146)
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
		GreyBit_Stream_Read(me->gbStream, &nWidth, 1);
	}
	return (nSize * nWidth / me->gbInfoHeader.gbiHeight);
}

GB_INT32 GreyVectorFile_Decoder_Decode(GB_Decoder decoder, GB_UINT32 nCode, GB_Data pData, GB_INT16 nSize)
{
	GB_UINT16 Lenght; 
	GB_INT32 nWidth;
	GB_Outline outline; 
	GB_UINT32 Offset;
	GVF_Decoder me = (GVF_Decoder)decoder;
	Offset = GreyVectorFile_Decoder_GetDataOffset(me, nCode);
	nWidth = GreyVectorFile_Decoder_GetWidth(decoder, nCode, nSize);
	if (!nWidth)
		return -1;
	if ((Offset & 0x80000000) == 0)
	{
		GreyBit_Stream_Seek(me->gbStream, me->gbInfoHeader.gbiOffGreyBits + me->gbOffDataBits + Offset);
		GreyBit_Stream_Read(me->gbStream, (GB_BYTE*)&Lenght, 2);
		GreyBit_Stream_Read(me->gbStream, me->pBuff + 12, Lenght);
		outline = GreyBitType_Outline_UpdateByGVF(me->gbOutline, GreyVector_Outline_FromData(me->pBuff));
		GreyVectorFile_Decoder_CaheItem(me, nCode, outline);
	}
	else
	{
		Offset &= 0x7FFFFFFF;
		outline = me->gpGreyBits[Offset];
	}
	if (!outline)
		return -1;
	GreyBitType_Outline_Transform(me->gbOutline, outline, nSize, me->gbInfoHeader.gbiHeight);
	if (pData)
	{
		pData->format = GB_FORMAT_OUTLINE;
		pData->data = me->gbOutline;
		pData->width = nWidth;
	}
	return 0;
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
		decoder->gbDecoder.decode = GreyVectorFile_Decoder_Decode;
		decoder->gbDecoder.done = GreyVectorFile_Decoder_Done;
		decoder->gbLibrary = loader->gbLibrary;
		decoder->gbMem = loader->gbMem;
		decoder->gbStream = stream;
		decoder->nCacheItem = 0;
		decoder->nItemCount = 0;
		decoder->gbOffDataBits = 620;
		GreyVectorFile_Decoder_Init(decoder);
	}
	return (GB_Decoder)decoder;
}