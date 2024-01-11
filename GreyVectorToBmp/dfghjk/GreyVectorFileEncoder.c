#include "UnicodeSection.h"
#include "GreyBitSystem.h"
#include "GreyVectorCommon.h"
#include "GreyVectorFileEncoder.h"

#ifdef ENABLE_GREYVECTORFILE
#ifdef ENABLE_ENCODER
void GreyVectorFile_Encoder_ClearCache(GVF_Encoder me)
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

GB_INT32 GreyVectorFile_Encoder_GetCount(GB_Encoder encoder)
{
	GVF_Encoder me = (GVF_Encoder)encoder;
	return me->nItemCount;
}

GB_INT32 GreyVectorFile_Encoder_InfoInit(GVF_Encoder me, GB_INT16 nHeight)
{
	int i;

	if (me->gbInited)
	{
		if (me->gbInfoHeader.gbiHeight == nHeight)
			return GB_SUCCESS;
		GB_MEMSET(me->gbWidthTable, 0, MAX_COUNT);
		GB_MEMSET(me->gbHoriOffTable, 0, MAX_COUNT);
		GB_MEMSET(me->gbOffsetTable, 0, sizeof(GB_UINT32)*MAX_COUNT);
		for (i = 0; i < me->nCacheItem; ++i)
		{
			if (me->gpGreyBits[i])
			{
				GreyBitType_Outline_Done(me->gbLibrary, me->gpGreyBits[i]);
				me->gpGreyBits[i] = 0;
				me->pnGreySize[i] = 0;
			}
		}
	}
	me->gbInited = 1;
	me->gbInfoHeader.gbiHeight = nHeight;
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Encoder_SetParam(GB_Encoder encoder, GB_Param nParam, GB_UINT32 dwParam)
{
	GVF_Encoder me = (GVF_Encoder)encoder;
	if (nParam == GB_PARAM_HEIGHT)
	{
		if (dwParam)
			me->nHeight = (GB_UINT16)dwParam;
	}
	GreyVectorFile_Encoder_InfoInit(me, me->nHeight);
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Encoder_Delete(GB_Encoder encoder, GB_UINT32 nCode)
{
	GVF_Encoder me = (GVF_Encoder)encoder;
	me->gbOffsetTable[nCode] = 0;
	me->gbHoriOffTable[nCode] = GB_HORIOFF_DEFAULT;
	me->gbWidthTable[nCode] = GB_WIDTH_DEFAULT;
	if (me->gpGreyBits[nCode])
	{
		GreyBitType_Outline_Done(me->gbLibrary, me->gpGreyBits[nCode]);
		me->gpGreyBits[nCode] = 0;
		me->pnGreySize[nCode] = 0;
	}
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Encoder_Encode(GB_Encoder encoder, GB_UINT32 nCode, GB_Data pData)
{
	GB_INT16 nWidth;
	GB_Outline outline; 
	GB_Outline source;
	GVF_Encoder me = (GVF_Encoder)encoder;
	if (!pData || pData->format != GB_FORMAT_OUTLINE)
		return GB_FAILED;
	if (!me->nHeight || !me->gbInited)
		return GB_FAILED;
	nWidth = pData->width;
	source = (GB_Outline)pData->data;
	if (source->n_points > 255)
		return GB_FAILED;
	if (me->gbInfoHeader.gbiWidth < nWidth)
		me->gbInfoHeader.gbiWidth = nWidth;
	if (me->gbInfoHeader.gbiMaxPoints < source->n_points)
		me->gbInfoHeader.gbiMaxPoints = source->n_points;
	if (me->gbInfoHeader.gbiMaxContours < source->n_contours)
		me->gbInfoHeader.gbiMaxContours = source->n_contours;
	outline = GreyBitType_Outline_Clone(me->gbLibrary, source);
	if (me->gpGreyBits[nCode])
		GreyBitType_Outline_Done(me->gbLibrary, me->gpGreyBits[nCode]);
	me->gpGreyBits[nCode] = outline;
	me->pnGreySize[nCode] = (GB_UINT16)GreyVector_Outline_GetSizeEx((GB_BYTE)outline->n_contours, (GB_BYTE)outline->n_points);
	me->gbOffsetTable[nCode] = SET_RAM(nCode);
	me->gbWidthTable[nCode] = (GB_BYTE)nWidth;
	me->gbHoriOffTable[nCode] = (GB_BYTE)pData->horioff;
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Encoder_WriteAll(GVF_Encoder me)
{
	GVF_Outline outline;
	GB_INT32 nDataSize;
	GB_BYTE *pData;
	GB_INT32 nCode;
	GB_UINT16 nMinCode;
	GB_UINT16 nMaxCode;
	GB_INT32 nSectionLen;
	GB_INT32 nSection;

	GreyBit_Stream_Seek(me->gbStream, 0);
	GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&me->gbFileHeader, sizeof(GREYVECTORFILEHEADER));
	GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&me->gbInfoHeader, sizeof(GREYVECTORINFOHEADER));
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
	for (nCode = 0; nCode < me->nCacheItem; ++nCode)
	{
		nDataSize = me->pnGreySize[nCode];
		if (nDataSize)
		{
			outline = (GVF_Outline)GreyVector_Outline_NewByGB(me->gbLibrary, me->gpGreyBits[nCode]);
			GreyBit_Stream_Write(me->gbStream, (GB_BYTE*)&nDataSize, 2);
			pData = (GB_CHAR*)GreyVector_Outline_GetData(outline);
			GreyBit_Stream_Write(me->gbStream, pData, nDataSize);
			GreyVector_Outline_Done(me->gbLibrary, outline);
		}
	}
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Encoder_BuildAll(GVF_Encoder me)
{
	GB_INT32 nSize;
	GB_UINT32 nCount;
	GB_UINT32 nGreyBitSize;
	GB_UINT32 nOffSetTableSize;
	GB_UINT32 nHoriOffTableSize;
	GB_UINT32 nWidthTableSize;
	GB_UINT16 nCode;
	GB_UINT16 nCodea;
	GB_UINT16 nMinCode;
	GB_UINT16 nMaxCode;
	GB_UINT16 nSectionLen;
	GB_UINT16 nSection;

	nWidthTableSize = 0;
	nOffSetTableSize = 0;
	nHoriOffTableSize = 0;
	nGreyBitSize = 0;
	nCount = 0;
	for (nSection = 0; nSection < UNICODE_SECTION_NUM; ++nSection)
	{
		UnicodeSection_GetSectionInfo(nSection, &nMinCode, &nMaxCode);
		nSectionLen = nMaxCode - nMinCode + 1;
		for (nCode = nMinCode; nCode <= nMaxCode; nCode++)
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
	for (nCodea = 0; nCodea < MAX_COUNT; nCodea++)
	{
		nSize = me->pnGreySize[nCodea];
		if (nSize)
		{
			me->gbOffsetTable[nCodea] = nGreyBitSize;
			nGreyBitSize += nSize + 2;
			++nCount;
		}
	}
	me->gbInfoHeader.gbiCount = nCount;
	me->gbInfoHeader.gbiOffGreyBits = nOffSetTableSize + nHoriOffTableSize + nWidthTableSize;
	me->gbInfoHeader.gbiOffsetTabOff = nHoriOffTableSize + nWidthTableSize;
	me->gbInfoHeader.gbiHoriOffTabOff = nWidthTableSize;
	me->gbInfoHeader.gbiWidthTabOff = 0;
	me->gbInfoHeader.gbiSize = sizeof(GREYVECTORINFOHEADER);
	me->gbFileHeader.gbfTag[0] = 'g';
	me->gbFileHeader.gbfTag[1] = 'v';
	me->gbFileHeader.gbfTag[2] = 't';
	me->gbFileHeader.gbfTag[3] = 'f';
	return GB_SUCCESS;
}

GB_INT32 GreyVectorFile_Encoder_Flush(GB_Encoder encoder)
{
	GVF_Encoder me = (GVF_Encoder)encoder;

	GreyVectorFile_Encoder_BuildAll(me);
	GreyVectorFile_Encoder_WriteAll(me);
	return GB_SUCCESS;
}

void GreyVectorFile_Encoder_Done(GB_Encoder encoder)
{
	GVF_Encoder me = (GVF_Encoder)encoder;
	if (me->nHeight)
	{
		GreyVectorFile_Encoder_Flush(&me->gbEncoder);
		me->nHeight = 0;
	}
	GreyVectorFile_Encoder_ClearCache(me);
	GreyBit_Free(me->gbMem, encoder);
}

GB_INT32 GreyVectorFile_Encoder_Init(GVF_Encoder me)
{
	me->gbWidthTable = (GB_BYTE *)GreyBit_Malloc(me->gbMem, MAX_COUNT);
	me->gbHoriOffTable = (GB_BYTE *)GreyBit_Malloc(me->gbMem, MAX_COUNT);
	me->gbOffsetTable = (GB_UINT32 *)GreyBit_Malloc(me->gbMem, sizeof(GB_UINT32)*MAX_COUNT);
	me->gpGreyBits = (GB_Outline *)GreyBit_Malloc(me->gbMem, sizeof(GB_Outline)*MAX_COUNT);
	me->pnGreySize = (GB_UINT16 *)GreyBit_Malloc(me->gbMem, sizeof(GB_UINT16)*MAX_COUNT);
	me->nCacheItem = MAX_COUNT;
	GB_MEMSET(me->gbWidthTable, 0, MAX_COUNT);
	GB_MEMSET(me->gbHoriOffTable, 0, MAX_COUNT);
	return GB_SUCCESS;
}

GB_Encoder GreyVectorFile_Encoder_New(GB_Creator creator, GB_Stream stream)
{
	GVF_Encoder codec;

	codec = (GVF_Encoder)GreyBit_Malloc(creator->gbMem, sizeof(GVF_Encoder));
	if (codec)
	{
		codec->gbEncoder.getcount = GreyVectorFile_Encoder_GetCount;
		codec->gbEncoder.setparam = GreyVectorFile_Encoder_SetParam;
		codec->gbEncoder.remove = GreyVectorFile_Encoder_Delete;
		codec->gbEncoder.encode = GreyVectorFile_Encoder_Encode;
		codec->gbEncoder.flush = GreyVectorFile_Encoder_Flush;
		codec->gbEncoder.done = GreyVectorFile_Encoder_Done;
		codec->gbLibrary = creator->gbLibrary;
		codec->gbMem = creator->gbMem;
		codec->gbStream = stream;
		codec->nCacheItem = 0;
		codec->nItemCount = 0;
		codec->gbOffDataBits = sizeof(GREYVECTORFILEHEADER) + sizeof(GREYVECTORINFOHEADER);
		GreyVectorFile_Encoder_Init(codec);
	}
	return (GB_Encoder)codec;
}
#endif //ENABLE_ENCODER
#endif //ENABLE_GREYVECTORFILE