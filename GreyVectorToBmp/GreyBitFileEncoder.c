#include "UnicodeSection.h"
#include "GreyBitSystem.h"
#include "GreyBitFileEncoder.h"

#ifdef ENABLE_GREYBITFILE
#ifdef ENABLE_ENCODER
void GreyBitFile_Encoder_ClearCache(GBF_Encoder me)
{
	int i;

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

GB_INT32 GreyBitFile_Encoder_GetCount(GB_Encoder encoder)
{
	GBF_Encoder me = (GBF_Encoder)encoder;
	return me->nItemCount;
}

GB_INT32 GreyBitFile_Encoder_InfoInit(GBF_Encoder me, GB_INT16 nMaxWidth, GB_INT16 nHeight, GB_INT16 nBitCount, GB_INT16 bCompress)
{
	if (!me->gbInited || me->gbInfoHeader.gbiBitCount != nBitCount || me->gbInfoHeader.gbiHeight != nHeight)
		goto GBF_Encoder_Init;
	if (nBitCount == 1)
		return 0;
	if (nBitCount == 8 && bCompress == me->gbInfoHeader.gbiCompression)
		return 0;
GBF_Encoder_Init:
	me->gbInited = 1;
	me->gbInfoHeader.gbiBitCount = nBitCount;
	me->gbInfoHeader.gbiHeight = nHeight;
	if (nBitCount == 8)
		me->gbInfoHeader.gbiCompression = bCompress;
	else
		me->gbInfoHeader.gbiCompression = 0;
	return 0;
}

GB_INT32 GreyBitFile_Encoder_SetParam(GB_Encoder encoder, void *pParam)
{
	GBF_Encoder me = (GBF_Encoder)encoder;
	if (me->gbParam)
		return -1;
	me->gbParam = (GB_GbfParam)GreyBit_Malloc(me->gbMem, sizeof(GB_GbfParamRec));
	GreyBit_Memcpy_Sys(me->gbParam, pParam, sizeof(GB_GbfParamRec));
	GreyBitFile_Encoder_InfoInit(
		me,
		(GB_INT16)(2 * me->gbParam->decoder.nCacheItem),
		me->gbParam->encoder.nHeight,
		me->gbParam->encoder.nBitCount,
		me->gbParam->encoder.bCompress);
	return 0;
}

GB_INT32 GreyBitFile_Encoder_Delete(GB_Encoder encoder, GB_UINT32 nCode)
{
	GBF_Encoder me = (GBF_Encoder)encoder;
	me->gbOffsetTable[nCode] = 0;
	me->gbHoriOffTable[nCode] = 0;
	me->gbWidthTable[nCode] = 0;
	if (me->gpGreyBits[nCode])
	{
		GreyBit_Free(me->gbMem, me->gpGreyBits[nCode]);
		me->gpGreyBits[nCode] = 0;
	}
	me->pnGreySize[nCode] = 0;
	return 0;
}

GB_INT32 GreyBitFile_Encoder_Compress(GB_BYTE *pOutData, GB_INT32 *pnInOutLen, GB_BYTE *pInData, GB_INT32 nInDataLen)
{
	GB_INT32 nCompressLen;
	GB_INT32 nCompressLena;
	GB_INT32 i;
	GB_INT32 ia;
	GB_BYTE nNextData; 
	GB_BYTE nData;
	GB_BYTE nLen;

	nCompressLen = 0;
	nLen = 0;
	nData = *pInData >> 1;
	if (pOutData)
	{
		for (i = 1; i < nInDataLen; ++i)
		{
			nNextData = pInData[i] >> 1;
			if (nData == nNextData && nLen < MAX_LEN())
			{
				++nLen;
			}
			else if (nLen)
			{
				pOutData[nCompressLen++] = SET_LEN(nLen);
				pOutData[nCompressLen++] = nData;
				nLen = 0;
			}
			else
			{
				pOutData[nCompressLen++] = nData;
			}
			nData = nNextData;
		}
		if (nLen)
			pOutData[nCompressLen++] = SET_LEN(nLen);
		pOutData[nCompressLen] = nData;
		nCompressLena = nCompressLen + 1;
	}
	else
	{
		for (ia = 1; ia < nInDataLen; ++ia)
		{
			if (nData == pInData[ia] >> 1 && nLen < MAX_LEN())
			{
				++nLen;
			}
			else if (nLen)
			{
				nCompressLen += 2;
				nLen = 0;
			}
			else
			{
				++nCompressLen;
			}
			nData = pInData[ia] >> 1;
		}
		if (nLen)
			nCompressLena = nCompressLen + 2;
		else
			nCompressLena = nCompressLen + 1;
	}
	*pnInOutLen = nCompressLena;
	return 0;
}

GB_INT32 GreyBitFile_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData)
{
	GB_BYTE *pByteData;
	GB_INT32 nOutLen;
	GB_INT32 nInDataLen;
	GB_Bitmap bitmap;
	GBF_Encoder me = (GBF_Encoder)encoder;
	if (!pData || pData->format != GB_FORMAT_BITMAP)
		return -1;
	if (!me->gbParam || !me->gbInited)
		return -1;
	bitmap = (GB_Bitmap)pData->data;
	if (bitmap->bitcount != me->gbInfoHeader.gbiBitCount
		|| bitmap->height != me->gbInfoHeader.gbiHeight
		|| bitmap->width > 3 * bitmap->height)
	{
		return -1;
	}
	if (me->gbInfoHeader.gbiWidth < bitmap->width)
		me->gbInfoHeader.gbiWidth = bitmap->width;
	nInDataLen = bitmap->pitch * bitmap->height;
	if (me->gbInfoHeader.gbiCompression)
	{
		GreyBitFile_Encoder_Compress(0, &nOutLen, bitmap->buffer, nInDataLen);
		pByteData = (GB_BYTE *)GreyBit_Malloc(me->gbMem, nOutLen);
		GreyBitFile_Encoder_Compress(pByteData, &nOutLen, bitmap->buffer, nInDataLen);
	}
	else
	{
		nOutLen = nInDataLen;
		pByteData = (GB_BYTE *)GreyBit_Malloc(me->gbMem, nInDataLen);
		GreyBit_Memcpy_Sys(pByteData, bitmap->buffer, nInDataLen);
	}
	if (me->gpGreyBits[nCode])
		GreyBit_Free(me->gbMem, me->gpGreyBits[nCode]);
	me->gpGreyBits[nCode] = pByteData;
	me->pnGreySize[nCode] = (GB_UINT16)nOutLen;
	me->gbOffsetTable[nCode] = SET_RAM(nCode);
	me->gbWidthTable[nCode] = (GB_BYTE)bitmap->width;
	me->gbHoriOffTable[nCode] = (GB_BYTE)bitmap->horioff;
	return 0;
}

GB_INT32 GreyBitFile_Encoder_WriteAll(GBF_Encoder me)
{
	GB_INT32 nDataSize;
	GB_BYTE *pData; 
	GB_INT32 nCode; 
	GB_UINT16 nMinCode; 
	GB_UINT16 nMaxCode;
	GB_INT32 nSectionLen;
	GB_INT32 nSection;

	GreyBit_Stream_Seek(me->gbStream, 0);
	GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&me->gbFileHeader, sizeof(GREYBITFILEHEADER));
	GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&me->gbInfoHeader, sizeof(GREYBITINFOHEADER));
	for (nSection = 0; nSection < UNICODE_SECTION_NUM; ++nSection)
	{
		UnicodeSection_GetSectionInfo(nSection, &nMinCode, &nMaxCode);
		nSectionLen = nMaxCode - nMinCode + 1;
		if (me->gbInfoHeader.gbiWidthSection.gbSectionOff[nSection])
		{
			pData = (GB_BYTE *)&me->gbWidthTable[nMinCode];
			nDataSize = (GB_UINT16)nSectionLen;
			GreyBit_Stream_Write(me->gbStream, pData, nSectionLen);
		}
	}
	for (nSection = 0; nSection < UNICODE_SECTION_NUM; ++nSection)
	{
		UnicodeSection_GetSectionInfo(nSection, &nMinCode, &nMaxCode);
		nSectionLen = nMaxCode - nMinCode + 1;
		if (me->gbInfoHeader.gbiWidthSection.gbSectionOff[nSection])
		{
			pData = (GB_BYTE *)&me->gbHoriOffTable[nMinCode];
			nDataSize = nSectionLen;
			GreyBit_Stream_Write(me->gbStream, pData, nDataSize);
		}
	}
	for (nSection = 0; nSection < UNICODE_SECTION_NUM; ++nSection)
	{
		UnicodeSection_GetSectionInfo(nSection, &nMinCode, &nMaxCode);
		nSectionLen = nMaxCode - nMinCode + 1;
		if (me->gbInfoHeader.gbiIndexSection.gbSectionOff[nSection])
		{
			pData = (GB_BYTE *)&me->gbOffsetTable[nMinCode];
			nDataSize = sizeof(GB_UINT32) * (GB_UINT16)nSectionLen;
			GreyBit_Stream_Write(me->gbStream, pData, nDataSize);
		}
	}
	if (me->gbInfoHeader.gbiCompression && me->gbInfoHeader.gbiBitCount == 8)
	{
		for (nCode = 0; nCode < me->nCacheItem; ++nCode)
		{
			nDataSize = me->pnGreySize[nCode];
			if (nDataSize)
			{
				GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&nDataSize, sizeof(GB_UINT16));
				pData = me->gpGreyBits[nCode];
				GreyBit_Stream_Write(me->gbStream, pData, nDataSize);
			}
		}
	}
	else
	{
		for (nCode = 0; nCode < me->nCacheItem; ++nCode)
		{
			nDataSize = me->pnGreySize[nCode];
			if (nDataSize)
			{
				pData = me->gpGreyBits[nCode];
				GreyBit_Stream_Write(me->gbStream, pData, nDataSize);
			}
		}
	}
	return 0;
}

GB_INT32 GreyBitFile_Encoder_BuildAll(GBF_Encoder me)
{
	GB_UINT16 nSize;
	GB_UINT16 nSizea;
	GB_UINT32 nCount;
	GB_UINT32 nGreyBitSize;
	GB_UINT32 nOffSetTableSize;
	GB_UINT32 nHoriOffTableSize;
	GB_UINT32 nWidthTableSize;
	GB_INT16 nCode;
	GB_INT16 nCodea;
	GB_INT16 nCodeb;
	GB_UINT16 nMinCode;
	GB_UINT16 nMaxCode;
	GB_INT16 nSectionLen;
	GB_INT16 nSection;

	nWidthTableSize = 0;
	nHoriOffTableSize = 0;
	nOffSetTableSize = 0;
	nGreyBitSize = 0;
	nCount = 0;
	for (nSection = 0; nSection < UNICODE_SECTION_NUM; ++nSection)
	{
		UnicodeSection_GetSectionInfo(nSection, &nMinCode, &nMaxCode);
		nSectionLen = nMaxCode - nMinCode + 1;
		for (nCode = nMinCode; nCode <= nMaxCode; ++nCode)
		{
			if (me->gbWidthTable[nCode])
			{
				me->gbInfoHeader.gbiWidthSection.gbSectionOff[nSection] = (GB_UINT16)nWidthTableSize + 1;
				me->gbInfoHeader.gbiIndexSection.gbSectionOff[nSection] = (GB_INT16)(nOffSetTableSize >> 2) + 1;
				nWidthTableSize += nSectionLen;
				nHoriOffTableSize += nSectionLen;
				nOffSetTableSize += sizeof(GB_UINT32) * nSectionLen;
				break;
			}
		}
	}
	if (me->gbInfoHeader.gbiCompression && me->gbInfoHeader.gbiBitCount == 8)
	{
		for (nCodea = 0; nCodea < MAX_COUNT; ++nCodea)
		{
			nSize = (GB_UINT16)me->pnGreySize[nCodea];
			if (nSize)
			{
				me->gbOffsetTable[nCodea] = nGreyBitSize;
				nGreyBitSize += nSize + 2;
				++nCount;
			}
		}
	}
	else
	{
		for (nCodeb = 0; nCodeb < MAX_COUNT; ++nCodeb)
		{
			nSizea = (GB_UINT16)me->pnGreySize[nCodeb];
			if (nSizea)
			{
				me->gbOffsetTable[nCodeb] = nGreyBitSize;
				nGreyBitSize += nSizea;
				++nCount;
			}
		}
	}
	me->gbInfoHeader.gbiCount = nCount;
	me->gbInfoHeader.gbiOffGreyBits = nOffSetTableSize + nHoriOffTableSize + nWidthTableSize;
	me->gbInfoHeader.gbiOffsetTabOff = nHoriOffTableSize + nWidthTableSize;
	me->gbInfoHeader.gbiHoriOffTabOff = nWidthTableSize;
	me->gbInfoHeader.gbiWidthTabOff = 0;
	me->gbInfoHeader.gbiSize = sizeof(GREYBITINFOHEADER);
	me->gbFileHeader.gbfTag[0] = 'g';
	me->gbFileHeader.gbfTag[1] = 'b';
	me->gbFileHeader.gbfTag[2] = 't';
	me->gbFileHeader.gbfTag[3] = 'f';
	return 0;
}

GB_INT32 GreyBitFile_Encoder_Flush(GB_Encoder encoder)
{
	GBF_Encoder me = (GBF_Encoder)encoder;

	GreyBitFile_Encoder_BuildAll(me);
	GreyBitFile_Encoder_WriteAll(me);
	if (me->gbParam)
	{
		GreyBit_Free(me->gbMem, me->gbParam);
		me->gbParam = 0;
	}
	return 0;
}

void GreyBitFile_Encoder_Done(GB_Encoder encoder)
{
	GBF_Encoder me = (GBF_Encoder)encoder;
	if (me->gbParam)
	{
		GreyBitFile_Encoder_Flush(&me->gbEncoder);
		GreyBit_Free(me->gbMem, me->gbParam);
	}
	GreyBitFile_Encoder_ClearCache(me);
	GreyBit_Free(me->gbMem, encoder);
}

GB_INT32 GreyBitFile_Encoder_Init(GBF_Encoder me)
{
	me->gbWidthTable = (GB_BYTE *)GreyBit_Malloc(me->gbMem, MAX_COUNT);
	me->gbHoriOffTable = (GB_BYTE *)GreyBit_Malloc(me->gbMem, MAX_COUNT);
	me->gbOffsetTable = (GB_UINT32 *)GreyBit_Malloc(me->gbMem, sizeof(GB_UINT32)*MAX_COUNT);
	me->gpGreyBits = (GB_BYTE **)GreyBit_Malloc(me->gbMem, sizeof(GB_BYTE*)*MAX_COUNT);
	me->pnGreySize = (GB_UINT16 *)GreyBit_Malloc(me->gbMem, sizeof(GB_UINT16)*MAX_COUNT);
	me->nCacheItem = MAX_COUNT;
	GreyBit_Memset_Sys(me->gbWidthTable, 0, MAX_COUNT);
	GreyBit_Memset_Sys(me->gbHoriOffTable, 0, MAX_COUNT);
	return 0;
}

GB_Encoder GreyBitFile_Encoder_New(GB_Creator creator, GB_Stream stream)
{
	GBF_Encoder codec;

	codec = (GBF_Encoder)GreyBit_Malloc(creator->gbMem, sizeof(GBF_Encoder));
	if (codec)
	{
		codec->gbEncoder.getcount = GreyBitFile_Encoder_GetCount;
		codec->gbEncoder.setparam = GreyBitFile_Encoder_SetParam;
		codec->gbEncoder.remove = GreyBitFile_Encoder_Delete;
		codec->gbEncoder.encode = GreyBitFile_Encoder_Encode;
		codec->gbEncoder.flush = GreyBitFile_Encoder_Flush;
		codec->gbEncoder.done = GreyBitFile_Encoder_Done;
		codec->gbLibrary = creator->gbLibrary;
		codec->gbMem = creator->gbMem;
		codec->gbStream = stream;
		codec->nCacheItem = 0;
		codec->nItemCount = 0;
		codec->gbOffDataBits = sizeof(GREYBITFILEHEADER)+sizeof(GREYBITINFOHEADER);
		GreyBitFile_Encoder_Init(codec);
	}
	return (GB_Encoder)codec;
}
#endif //ENABLE_ENCODER
#endif //ENABLE_GREYBITFILE