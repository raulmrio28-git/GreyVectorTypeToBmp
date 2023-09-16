#include "GreyBitType.h"
#include "GreyBitType_Def.h"
#include "GreyBitCodec.h"
#include "GreyBitRaster.h"

GB_BYTE* GreyBitType_Bitmap_SwitcBuffer(GB_Bitmap bitmap, void *pNewBuf)
{
	GB_BYTE *pBuf;

	pBuf = bitmap->buffer;
	bitmap->buffer = (GB_BYTE *)pNewBuf;
	return pBuf;
}

int GreyBitType_Layout_Bold(GB_Layout layout)
{
	GB_BYTE bitMove;
	GB_INT16 nOff;
	GB_INT16 xMax;
	GB_INT16 xMaxa;
	GB_INT16 yMax;
	GB_BYTE *pDst;
	GB_BYTE *pDsta;
	GB_BYTE *pSrc;
	GB_INT32 y;
	GB_INT32 ya;
	GB_INT32 x;
	GB_INT32 xa;
	GB_Bitmap bitmap;

	bitmap = layout->gbBitmap;
	pSrc = bitmap->buffer;
	yMax = bitmap->height;
	nOff = yMax >> 5;
	if (!nOff)
		return GB_FAILED;
	if (nOff > 4)
		nOff = 4;
	if (bitmap->bitcount == 8)
	{
		pDst = &layout->gbSwitchBuf[nOff];
		xMax = bitmap->pitch - nOff;
		GreyBit_Memcpy_Sys(layout->gbSwitchBuf, bitmap->buffer, layout->nSwitchBufLen);
		for (y = 0; y < yMax; ++y)
		{
			for (x = 0; x < xMax; ++x)
			{
				if ((pSrc[x] + pDst[x]) <= 255)
					pDst[x] += pSrc[x];
				else
					pDst[x] = 0xff;
			}
			pSrc += bitmap->pitch;
			pDst += bitmap->pitch;
		}
	}
	else
	{
		if (bitmap->bitcount != 1)
			return GB_FAILED;
		pDsta = layout->gbSwitchBuf;
		xMaxa = bitmap->pitch;
		for (ya = 0; ya < yMax; ++ya)
		{
			bitMove = 0;
			for (xa = 0; xa < xMaxa; ++xa)
			{
				pDsta[xa] = (bitMove << (8 - nOff)) | (pSrc[xa] >> nOff);
				bitMove = (0xff >> (8 - nOff)) & pSrc[xa];
			}
			pSrc += bitmap->pitch;
			pDsta += bitmap->pitch;
		}
	}
	layout->gbSwitchBuf = GreyBitType_Bitmap_SwitcBuffer(bitmap, layout->gbSwitchBuf);
	return GB_SUCCESS;
}

int GreyBitType_Layout_Italic(GB_Layout layout)
{
	GB_BYTE bitMove;
	GB_INT16 nOff;
	GB_INT16 nOffa;
	GB_INT16 nHalfOffMax;
	GB_INT16 xMax;
	GB_INT16 xMaxa;
	GB_INT16 xMaxb;
	GB_INT16 yMax;
	GB_BYTE *pDst;
	GB_BYTE *pDsta;
	GB_BYTE *pSrc;
	GB_INT32 y;
	GB_INT32 ya;
	GB_INT32 x;
	GB_INT32 xa;
	GB_INT32 xb;
	GB_INT32 xc;
	GB_Bitmap bitmap;

	bitmap = layout->gbBitmap;
	pSrc = bitmap->buffer;
	yMax = bitmap->height;
	nHalfOffMax = yMax >> 3;
	if (!(yMax >> 2))
		return GB_FAILED;
	if (bitmap->bitcount == 8)
	{
		pDst = layout->gbSwitchBuf;
		GreyBit_Memset_Sys(pDst, 0, layout->nSwitchBufLen);
		for (y = 0; y < yMax; ++y)
		{
			nOff = (GB_INT16)(y >> 2) - nHalfOffMax;
			if (nOff >= 0)
			{
				xMaxa = bitmap->pitch - nOff;
				for (xa = 0; xa < xMaxa; ++xa)
					pDst[nOff + xa] = pSrc[xa];
			}
			else
			{
				xMax = bitmap->pitch;
				for (x = -nOff; x < xMax; ++x)
					pDst[nOff + x] = pSrc[x];
			}
			pSrc += bitmap->pitch;
			pDst += bitmap->pitch;
		}
	}
	else
	{
		if (bitmap->bitcount != 1)
			return GB_FAILED;
		pDsta = layout->gbSwitchBuf;
		xMaxb = bitmap->pitch;
		for (ya = 0; ya < yMax; ++ya)
		{
			nOffa = (GB_INT16)(ya >> 2) - nHalfOffMax;
			bitMove = 0;
			if (nOffa >= 0)
			{
				for (xc = 0; xc < xMaxb; ++xc)
				{
					pDsta[xc] = (bitMove << (8 - nOffa)) | (pSrc[xc] >> nOffa);
					bitMove = (255 >> (8 - nOffa)) & pSrc[xc];
				}
			}
			else
			{
				--xMaxb;
				for (xb = 0; xb < xMaxb; ++xb)
					pDsta[xb] = (((255 << (nOffa + 8)) & pSrc[xb + 1]) >> (nOffa + 8)) | (pSrc[xb] << nOffa);
				pDsta[xb] = pSrc[xb] << nOffa;
			}
			pSrc += bitmap->pitch;
			pDsta += bitmap->pitch;
		}
	}
	layout->gbSwitchBuf = GreyBitType_Bitmap_SwitcBuffer(bitmap, layout->gbSwitchBuf);
	return GB_SUCCESS;
}

void GreyBitType_Layout_BMP8Scale8(GB_BYTE *pBitsDst, GB_INT16 wWidthDst, GB_INT16 wHeightDst, GB_INT16 nPitchDst, GB_BYTE *pBitsSrc, GB_INT16 wWidthSrc, GB_INT16 wHeightSrc, GB_INT16 nPitchSrc)
{
	GB_INT32 j; 
	GB_INT32 i; 

	for (i = 0; i < wHeightDst; ++i)
	{
		for (j = 0; j < wWidthDst; ++j)
			pBitsDst[j] = pBitsSrc[((GB_UINT32)((wHeightSrc << 10) / wHeightDst * i) >> 10) * nPitchSrc
			+ ((GB_UINT32)((wWidthSrc << 10) / wWidthDst * j) >> 10)];
		pBitsDst += nPitchDst;
	}
}

void GreyBitType_Layout_BMP1Scale1(GB_BYTE*pBitsDst, GB_INT16 wWidthDst, GB_INT16 wHeightDst, GB_INT16 nPitchDst, GB_BYTE*pBitsSrc, GB_INT16 wWidthSrc, GB_INT16 wHeightSrc, GB_INT16 nPitchSrc)
{
	GB_INT32 j;
	GB_INT32 ja;
	GB_INT32 i;

	for (i = 0; i < wHeightDst; ++i)
	{
		for (j = 0; j < nPitchDst; ++j)
			pBitsDst[j] = 0;
		for (ja = 0; ja < wWidthDst; ++ja)
			pBitsDst[ja >> 3] |= (((GB_INT32)pBitsSrc[((GB_UINT32)((wHeightSrc << 10) / wHeightDst * i) >> 10) * nPitchSrc
				+ ((GB_UINT32)((wWidthSrc << 10) / wWidthDst * ja) >> 13)] >> (7 - ((GB_UINT32)((wWidthSrc << 10) / wWidthDst * ja) >> 10) % 8)) & 1) << (7 - ja % 8);
		pBitsDst += nPitchDst;
	}
}

void GreyBitType_Layout_BMP8Scale1(GB_BYTE*pBitsDst, GB_INT16 wWidthDst, GB_INT16 wHeightDst, GB_INT16 nPitchDst, GB_BYTE*pBitsSrc, GB_INT16 wWidthSrc, GB_INT16 wHeightSrc, GB_INT16 nPitchSrc)
{
	GB_INT32 j; 
	GB_INT32 ja; 
	GB_INT32 i; 

	for (i = 0; i < wHeightDst; ++i)
	{
		for (j = 0; j < nPitchDst; ++j)
			pBitsDst[j] = 0;
		for (ja = 0; ja < wWidthDst; ++ja)
			pBitsDst[ja >> 3] |= (GB_INT32)pBitsSrc[((GB_UINT32)((wHeightSrc << 10) / wHeightDst * i) >> 10) * nPitchSrc
			+ ((GB_UINT32)((wWidthSrc << 10) / wWidthDst * ja) >> 10)] >> 7 << (7 - ja % 8);
		pBitsDst += nPitchDst;
	}
}

void GreyBitType_Layout_BMP8To1(GB_BYTE*pBitsDst, GB_INT16 wWidthDst, GB_INT16 wHeightDst, GB_INT16 nPitchDst, GB_BYTE*pBitsSrc, GB_INT16 wWidthSrc, GB_INT16 wHeightSrc, GB_INT16 nPitchSrc)
{
	GB_INT32 j;
	GB_INT32 ja; 
	GB_INT32 i; 

	for (i = 0; i < wHeightDst; ++i)
	{
		for (j = 0; j < nPitchDst; ++j)
			pBitsDst[j] = 0;
		for (ja = 0; ja < wWidthDst; ++ja)
			pBitsDst[ja >> 3] |= (GB_INT32)pBitsSrc[ja] >> 7 << (7 - ja % 8);
		pBitsDst += nPitchDst;
		pBitsSrc += nPitchSrc;
	}
}

void GreyBitType_Layout_BMP1Scale8(GB_BYTE*pBitsDst, GB_INT16 wWidthDst, GB_INT16 wHeightDst, GB_INT16 nPitchDst, GB_BYTE*pBitsSrc, GB_INT16 wWidthSrc, GB_INT16 wHeightSrc, GB_INT16 nPitchSrc)
{
	GB_INT32 j;
	GB_INT32 i; 

	for (i = 0; i < wHeightDst; ++i)
	{
		for (j = 0; j < wWidthDst; ++j)
			pBitsDst[j] = -((((GB_INT32)pBitsSrc[((GB_UINT32)((wHeightSrc << 10) / wHeightDst * i) >> 10) * nPitchSrc
				+ ((GB_UINT32)((wWidthSrc << 10) / wWidthDst * j) >> 13)] >> (7 - ((GB_UINT32)((wWidthSrc << 10) / wWidthDst * j) >> 10) % 8)) & 1) != 0);
		pBitsDst += nPitchDst;
	}
}

void GreyBitType_Layout_BMP1To8(GB_BYTE*pBitsDst, GB_INT16 wWidthDst, GB_INT16 wHeightDst, GB_INT16 nPitchDst, GB_BYTE*pBitsSrc, GB_INT16 wWidthSrc, GB_INT16 wHeightSrc, GB_INT16 nPitchSrc)
{
	GB_INT32 j; 
	GB_INT32 i;

	for (i = 0; i < wHeightDst; ++i)
	{
		for (j = 0; j < wWidthDst; ++j)
			pBitsDst[j] = -((((GB_INT32)pBitsSrc[j >> 3] >> (7 - j % 8)) & 1) != 0);
		pBitsDst += nPitchDst;
		pBitsSrc += nPitchSrc;
	}
}

int GreyBitType_Layout_ScaleBitmap(GB_Bitmap dst, GB_Bitmap src)
{
	if (dst->bitcount == src->bitcount)
	{
		if (dst->bitcount == 8)
		{
			dst->width = src->width * dst->height / src->height;
			dst->horioff = src->horioff * dst->height / src->height;
			dst->pitch = dst->width;
			GreyBitType_Layout_BMP8Scale8(
				dst->buffer,
				dst->width,
				dst->height,
				dst->pitch,
				src->buffer,
				src->width,
				src->height,
				src->pitch);
		}
		else if (dst->bitcount == 1)
		{
			dst->width = src->width * dst->height / src->height;
			dst->horioff = src->horioff * dst->height / src->height;
			dst->pitch = dst->width >> 3;
			if (!dst->pitch)
				dst->pitch = 1;
			GreyBitType_Layout_BMP1Scale1(
				dst->buffer,
				dst->width,
				dst->height,
				dst->pitch,
				src->buffer,
				src->width,
				src->height,
				src->pitch);
		}
	}
	else if (dst->bitcount == 8 && src->bitcount == 1)
	{
		dst->width = src->width * dst->height / src->height;
		dst->horioff = src->horioff * dst->height / src->height;
		dst->pitch = dst->width;
		if (dst->height == src->height)
			GreyBitType_Layout_BMP1To8(
				dst->buffer,
				dst->width,
				dst->height,
				dst->pitch,
				src->buffer,
				src->width,
				src->height,
				src->pitch);
		else
			GreyBitType_Layout_BMP1Scale8(
				dst->buffer,
				dst->width,
				dst->height,
				dst->pitch,
				src->buffer,
				src->width,
				src->height,
				src->pitch);
	}
	else if (dst->bitcount == 1 && src->bitcount == 8)
	{
		dst->width = src->width * dst->height / src->height;
		dst->horioff = src->horioff * dst->height / src->height;
		dst->pitch = dst->width >> 3;
		if (!dst->pitch)
			dst->pitch = 1;
		if (dst->height == src->height)
			GreyBitType_Layout_BMP8To1(
				dst->buffer,
				dst->width,
				dst->height,
				dst->pitch,
				src->buffer,
				src->width,
				src->height,
				src->pitch);
		else
			GreyBitType_Layout_BMP8Scale1(
				dst->buffer,
				dst->width,
				dst->height,
				dst->pitch,
				src->buffer,
				src->width,
				src->height,
				src->pitch);
	}
	return GB_SUCCESS;
}

GBHANDLE GreyBitType_Layout_New(GBHANDLE loader, GB_INT16 nSize, GB_INT16 nBitCount, GB_BOOL bBold, GB_BOOL bItalic)
{
	GB_Layout me;
	GB_Loader my_loader = (GB_Loader)loader;

	me = (GB_Layout)GreyBit_Malloc(my_loader->gbMem, sizeof(GB_LayoutRec));
	if (me)
	{
		me->gbLibrary = my_loader->gbLibrary;
		me->gbMem = my_loader->gbMem;
		me->gbStream = my_loader->gbStream;
		me->gbDecoder = my_loader->gbDecoder;
		me->dwCode = 0xffff;
		me->nSize = nSize;
		me->nBitCount = nBitCount;
		me->bBold = bBold;
		me->bItalic = bItalic;
		me->gbBitmap = GreyBitType_Bitmap_New(me->gbLibrary, 2 * nSize, nSize, nBitCount, 0);
		if (nBitCount != 8)
			me->gbBitmap8 = GreyBitType_Bitmap_New(me->gbLibrary, 2 * nSize, nSize, 8, 0);
#ifdef ENABLE_GREYVECTORFILE
		me->gbRaster = (void *)GreyBit_Raster_New(me->gbLibrary, 0);
#endif //ENABLE_GREYVECTORFILE
		me->nSwitchBufLen = me->gbBitmap->height * me->gbBitmap->pitch;
		me->gbSwitchBuf = (GB_BYTE *)GreyBit_Malloc(me->gbMem, me->nSwitchBufLen);
	}
	return me;
}

GB_INT32 GreyBitType_Layout_GetWidth(GBHANDLE layout, GB_UINT32 nCode)
{
	GB_Layout me = (GB_Layout)layout;

	return GreyBit_Decoder_GetAdvance(me->gbDecoder, nCode, me->nSize);
}

int GreyBitType_Layout_LoadChar(GBHANDLE layout, GB_UINT32 nCode, GB_Bitmap *pBmp)
{
	GB_DataRec data;
	GB_Bitmap bitmap;
	GB_Layout me = (GB_Layout)layout;

	if (!me->gbBitmap)
		return GB_FAILED;
	if (me->dwCode != nCode)
	{
		if (GreyBit_Decoder_Decode(me->gbDecoder, nCode, &data, me->nSize) != GB_SUCCESS)
			return GB_FAILED;
#ifdef ENABLE_GREYVECTORFILE
		if (data.format == GB_FORMAT_BITMAP)
		{
			bitmap = (GB_Bitmap)data.data;
		}
		else
		{
			if (data.format != GB_FORMAT_OUTLINE)
				return GB_FAILED;
			if (me->gbBitmap8)
				bitmap = me->gbBitmap8;
			else
				bitmap = me->gbBitmap;
			bitmap->width = data.width;
			bitmap->pitch = data.width;
			bitmap->horioff = data.horioff;
			GreyBit_Memset_Sys(bitmap->buffer, 0, bitmap->height * bitmap->pitch);
			GreyBit_Raster_Render(me->gbRaster, bitmap, data.data);
		}
#else
		bitmap = (GB_Bitmap)data.data;
#endif //ENABLE_GREYVECTORFILE
		if (bitmap->bitcount == me->gbBitmap->bitcount && bitmap->height == me->gbBitmap->height)
		{
			me->gbBitmap->pitch = bitmap->pitch;
			me->gbBitmap->width = bitmap->width;
			me->gbBitmap->horioff = bitmap->horioff;
			bitmap->buffer = (GB_BYTE *)GreyBitType_Bitmap_SwitcBuffer(me->gbBitmap, bitmap->buffer);
		}
		else
		{
			GreyBitType_Layout_ScaleBitmap(me->gbBitmap, bitmap);
		}
		if (me->bBold)
			GreyBitType_Layout_Bold(me);
		if (me->bItalic)
			GreyBitType_Layout_Italic(me);
		me->dwCode = nCode;
	}
	if (pBmp)
		*pBmp = me->gbBitmap;
	return GB_SUCCESS;
}

void GreyBitType_Layout_Done(GBHANDLE layout)
{
	GB_Layout me = (GB_Layout)layout;

	if (me->gbBitmap)
	{
		if (me->gbSwitchBuf)
			GreyBit_Free(me->gbMem, me->gbSwitchBuf);
		GreyBitType_Bitmap_Done(me->gbLibrary, me->gbBitmap);
	}
	if (me->gbBitmap8)
		GreyBitType_Bitmap_Done(me->gbLibrary, me->gbBitmap8);
#ifdef ENABLE_GREYVECTORFILE
	if (me->gbRaster)
		GreyBit_Raster_Done(me->gbRaster);
#endif //ENABLE_GREYVECTORFILE
	GreyBit_Free(me->gbMem, me);
}