// GreyVectorToBmp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "GreyBitType.h"
#include <iostream>

#define BIGNUMBER_FONT_SIZE 30
#define NORMAL_FONT_SIZE    18
#define LARGE_FONT_SIZE     24
#define SMALL_FONT_SIZE     8

static GBHANDLE g_pLibrary = NULL;
static GBHANDLE g_pLoader = NULL;

struct IFont {
	unsigned int                dwRefs;
	short                 wSize;
	bool               bBold;
	bool               bItalic;
	GBHANDLE              pLayout;
};

static IFont gFontNormal = { 0, NORMAL_FONT_SIZE,   false, false, NULL };
static IFont gFontNormalBold = { 0, NORMAL_FONT_SIZE,   false, false, NULL };
static IFont gFontLarge = { 0, LARGE_FONT_SIZE,    false, false, NULL };
static IFont gFontBigNumber = { 0, BIGNUMBER_FONT_SIZE,false, false, NULL };
static IFont gFontSmall = { 0, SMALL_FONT_SIZE,    false, false, NULL };

void GreyBitFont_Init(char* fn)
{
	if (g_pLibrary == NULL) {
		g_pLibrary = GreyBitType_Init();
	}

	if (g_pLibrary) {
		if (g_pLoader == NULL) {
			g_pLoader = GreyBitType_Loader_New(g_pLibrary, fn);
		}
	}
}

static void OEMFont_Create(IFont *pMe)
{
	if (g_pLoader && pMe->pLayout == NULL) {
		pMe->pLayout = GreyBitType_Layout_New(g_pLoader, pMe->wSize, 8, pMe->bBold, pMe->bItalic);
	}
}

static void OEMFont_Destroy(IFont *pMe)
{
	if (pMe->pLayout)
	{
		GreyBitType_Layout_Done(pMe->pLayout);
		pMe->pLayout = NULL;
	}
}

void GreyBitBrewFont_Done(void)
{
	OEMFont_Destroy(&gFontNormal);
	OEMFont_Destroy(&gFontNormalBold);
	OEMFont_Destroy(&gFontLarge);
	OEMFont_Destroy(&gFontBigNumber);
	OEMFont_Destroy(&gFontSmall);

	if (g_pLoader) {
		GreyBitType_Loader_Done(g_pLoader);
		g_pLoader = NULL;
	}

	if (g_pLibrary) {
		GreyBitType_Done(g_pLibrary);
		g_pLibrary = NULL;
	}
}

unsigned int OEMFont_AddRef(IFont *pMe)
{
	return ++pMe->dwRefs;
}

unsigned int OEMFont_Release(IFont *pMe)
{
	unsigned int dwRefs = --pMe->dwRefs;

	if (0 == dwRefs) {
		OEMFont_Destroy(pMe);
	}
	return dwRefs;
}

#define CGreyBit_COLORSCHEME565_GET_R(color)       ((unsigned short)((color) >> 11))
#define CGreyBit_COLORSCHEME565_GET_G(color)       ((unsigned short)((unsigned short)((color) << 5) >> 10))    /* (((color) >> 5) & 0x003F)                 */
#define CGreyBit_COLORSCHEME565_GET_B(color)       ((unsigned short)((unsigned short)((color) << 11) >> 11))   /* ((color) & 0x001F)                        */
#define CGreyBit_COLORSCHEME565_GET_RGB(r,g,b)     ((unsigned short)(((r) << 11) | ((g) << 5) | (b)))

#define CGreyBit_COLORSCHEME888_GET_R(color)       (color.R)
#define CGreyBit_COLORSCHEME888_GET_G(color)       (color.G)    /* (((color) >> 5) & 0x003F)                 */
#define CGreyBit_COLORSCHEME888_GET_B(color)       (color.B)   /* ((color) & 0x001F)                        */

#pragma pack(1)
typedef struct tagRGBA
{
	unsigned char R;
	unsigned char B;
	unsigned char G;
	unsigned char A;
} RGBA;

typedef struct tagBMP
{
	unsigned int width;
	unsigned int height;
	unsigned char *rawp;
} BMP;
#pragma pack()

typedef struct tagRECT
{
	unsigned short x, y;
	unsigned short dx, dy;
} RECT;

#define SETAEERECT(prc,l,t,w,h)   (prc)->x=(unsigned short)(l),(prc)->y=(unsigned short)(t),(prc)->dx=(unsigned short)(w),(prc)->dy=(unsigned short)(h)

static bool GetCombineOffSet(IFont *pMe, unsigned short ch1, unsigned short ch2, int *pOff)
{
	bool bRet = true;

	// Hindi
	switch (ch2) {
	case 0x093F:
	case 0x094E:
		if (pOff)
		{
			bRet = true;
			*pOff = -(GreyBitType_Layout_GetWidth(pMe->pLayout, ch2) * 3) / 5;
		}
		break;
	default:
		break;
	}

	// Hebrew
	//switch()
	return bRet;
}

static short Rgba_CvtToRgb565(RGBA color)
{
	return (color.B >> 3) | ((color.G >> 2) << 5) | ((color.R >> 3) << 11);
}

static void DrawChar(IFont *pMe, unsigned char *pBmp, int nPitch, const wchar_t *pcText, int nChars,
	int x, int xMin, int xMax, int sy, int oy, int dy, RGBA clrText, RGBA clrBack,
	bool bTransparency, int *pOutWidth)
{
	int xSrc, i;
	unsigned char xWidth, xWidthOrig, xxDisp, *sp, *pFontData;
	int bytes_per_row, dispWidth = 0;
	RGBA *dp, *dpBase, cText, cBack;
	unsigned int y1;
	wchar_t ch;
	int bmp_offset;
	GB_Bitmap charBmp;
	unsigned char foreR, foreG, foreB, backR, backG, backB, diffR, diffG, diffB;

	bmp_offset = sy * nPitch;

	cText = clrText;
	cBack = clrBack;

	nPitch >>= 1;

	foreR = CGreyBit_COLORSCHEME888_GET_R(cText);
	foreG = CGreyBit_COLORSCHEME888_GET_G(cText);
	foreB = CGreyBit_COLORSCHEME888_GET_B(cText);
	backR = CGreyBit_COLORSCHEME888_GET_R(cBack);
	backG = CGreyBit_COLORSCHEME888_GET_G(cBack);
	backB = CGreyBit_COLORSCHEME888_GET_B(cBack);
	diffR = foreR - backR;
	diffG = foreG - backG;
	diffB = foreB - backB;

	for (i = 0; i < nChars && pcText[i]; i++)
	{
		if (x > xMax)
		{
			break;
		}

		ch = pcText[i];
		if (ch < ' ') continue;

		if (0 != GreyBitType_Layout_LoadChar(pMe->pLayout, ch, &charBmp))
		{
			continue;
		}

		xWidth = (unsigned char)charBmp->width;
		bytes_per_row = charBmp->pitch;

		// Clip x coordinate
		xWidthOrig = xWidth;
		//x += charBmp->horioff;
		xSrc = 0;

		if (x < xMin)
		{
			if ((x + xWidth) < xMin)
			{
				x += xWidth;
				continue;
			}

			xSrc = xMin - x;
			xWidth -= xSrc;
			x = xMin;
		}
		else if ((x + xWidth) > xMax)
		{
			xWidth = xMax - x + 1;
		}

		xxDisp = (xWidth > xWidthOrig) ? xWidthOrig : xWidth;

		pFontData = charBmp->buffer + oy * bytes_per_row;
		dp = dpBase = (RGBA*)(pBmp + bmp_offset + (x*4));

		y1 = dy;
		while (y1--) {
			unsigned char alpha = 255;
			unsigned int x1 = xxDisp;
			sp = pFontData;
			sp += xSrc;

			while (x1--) {
				switch (*sp) {
				case 0:
					*dp = cBack;
					break;
				case 0xFF:
					*dp = cText;
					break;
				default:
					foreR = ((diffR * (*sp)) / 256) + backR;
					foreG = ((diffG * (*sp)) / 256) + backG;
					foreB = ((diffB * (*sp)) / 256) + backB;
					*dp = { *sp, *sp, *sp, *sp };
					break;
				}
				dp++;
				sp++;
			}  // for loop

			pFontData += bytes_per_row;
			dp = dpBase += nPitch;
		}


		dispWidth += xWidth;
		x += xWidth;
	}

	*pOutWidth = dispWidth;
}

#define IDF_TEXT_TRANSPARENT 0x1
#define IDF_TEXT_UNDERLINE 0x2
#define IDF_TEXT_INVERTED 0x4
#define IDF_ALIGNHORZ_MASK 0x18
#define IDF_ALIGN_RIGHT 0x8
#define IDF_ALIGN_CENTER 0x10
#define IDF_ALIGNVERT_MASK 0x60
#define IDF_ALIGN_BOTTOM 0x20
#define IDF_ALIGN_MIDDLE 0x40

static int OEMFont_MeasureText(IFont *pMe, const wchar_t *pcText, int nChars, int nMaxWidth, int *pnCharFits, int *pnPixels)
{
	{
		int nRet = 0;
		int nRealStrLen, nFits, nTotalWidth = 0;
		wchar_t ch, ch1;
		int xCombOff = 0;

		if (pMe->pLayout == NULL) {
			return 1;
		}

		// Let's perform some sanity checks first
		if (!pcText) {
			return 0;
		}

		nRealStrLen = wcslen(pcText);

		if (nChars < 0 || nRealStrLen < nChars)
		{
			nChars = nRealStrLen;
		}

		if (nMaxWidth <= 0)
		{
			nMaxWidth = 0x0FFFFFFF;
		}

		for (nFits = 0; nFits < nChars; nFits++)
		{
			ch = *pcText++;
			if (ch < ' ') {
				continue;
			}

			if (xCombOff == 0)
			{
				ch1 = (nFits + 1) < nChars ? (*(pcText + 1)) : 0;
				if (GetCombineOffSet(pMe, ch, ch1, &xCombOff))
				{
					ch = ch1;
					ch1 = *pcText;
				}

				nTotalWidth += GreyBitType_Layout_GetWidth(pMe->pLayout, ch) + xCombOff;
			}
			else
			{
				nTotalWidth += GreyBitType_Layout_GetWidth(pMe->pLayout, ch1);
				xCombOff = 0;
			}

			//nTotalWidth += GreyBitType_Layout_GetWidth(pMe->pLayout, ch);
			if (nTotalWidth >= nMaxWidth)
			{
				nTotalWidth = nMaxWidth;
				break;
			}
		}

		if (!pnCharFits)
		{
			pnCharFits = &nFits;
		}
		else
		{
			*pnCharFits = nFits;
		}

		if (pnPixels)
		{
			*pnPixels = nTotalWidth;
		}

		return nRet;
	}
}

static int DrawTextEx(IFont *pMe, BMP *pDst, const wchar_t * pcText, int nChars,
	int x, int y, const RECT* prcBackground, unsigned int dwFlags, RGBA clrText,
	RGBA clrBack, RGBA clrFrame)
{
	int sx;
	int xMin, xMax, yMin, yMax;
	bool bTransparent, bUnderline;
	int sy, dy, oy;
	int dispWidth;
	int result = 0;

	xMin = prcBackground->x;
	xMax = prcBackground->x + prcBackground->dx - 1;
	yMin = prcBackground->y;
	yMax = prcBackground->y + prcBackground->dy - 1;

	if (x < xMin)
	{
		sx = xMin;
	}
	else
	{
		sx = x;
	}

	// clip y coordinate
	if (y > yMax)
	{
		return 0;
	}

	if (y < yMin)
	{
		sy = yMin;
		oy = yMin - y;
	}
	else
	{
		sy = y;
		oy = 0;
	}

	dy = pMe->wSize - oy;

	if (dy <= 0)   //
	{
		return 0;
	}

	if (dwFlags & IDF_TEXT_TRANSPARENT)
	{
		bTransparent = true;
	}
	else
	{
		bTransparent = false;
	}

	if (dwFlags & IDF_TEXT_UNDERLINE)  // last line will be overwritten by underline
	{
		bUnderline = true;
		dy--;
	}
	else
	{
		bUnderline = false;
	}

	if ((sy + dy) > yMax)
	{
		dy = yMax - sy + 1;
		bUnderline = false;
	}

	if (dy > 0)
	{

		DrawChar(pMe, pDst->rawp, 256, pcText, nChars, x, xMin, xMax, sy, oy, dy, clrText, clrBack, bTransparent, &dispWidth);
	}
	else if (bUnderline)
	{
		int nCharFits;
		OEMFont_MeasureText(pMe, pcText, nChars, xMax - xMin + 1, &nCharFits, &dispWidth);
	}

	if (bUnderline)
	{
		//result = IBITMAP_DrawHScanline(pDst, y + pMe->wSize - 1, sx, sx + dispWidth - 1, clrText, AEE_RO_COPY);
	}

	return result;
}

static int OEMFont_DrawText(IFont *pMe, BMP *pDst, int x, int y, const wchar_t *pcText, int nChars,
	RGBA foreground, RGBA background, const RECT *prcClip, unsigned int dwFlags)
{
	RECT rca;
	RGBA clrBack, clrText, clrFrame;
	RECT * prcBackRect = (RECT *)prcClip;
	int nWidth, nHeight;

	if (pMe->pLayout == NULL) {
		return 1;
	}

	// If no text, return immediately
	if (!pcText)
	{
		return 0;
	}

	// Calculate the string length
	if (nChars < 0)
	{
		nChars = wcslen(pcText);
	}

	// If no background rect, the full rectangle is used
	if (!prcClip)
	{
		prcBackRect = &rca;
		prcClip = &rca;
		SETAEERECT(prcBackRect, 0, 0, pDst->width, pDst->height);
	}

	// Get the text width and height
	nHeight = pMe->wSize;

	// Text is horizontally aligned
	if (dwFlags & IDF_ALIGNHORZ_MASK)
	{
		OEMFont_MeasureText(pMe, pcText, nChars, 0, NULL, &nWidth);

		x = prcBackRect->x;

		if (dwFlags & IDF_ALIGN_RIGHT)
		{
			x += (prcBackRect->dx - nWidth);
		}
		else if (dwFlags & IDF_ALIGN_CENTER)
		{
			x += ((prcBackRect->dx - nWidth) >> 1);
		}
	}

	// Text is vertically aligned
	if (dwFlags & IDF_ALIGNVERT_MASK)
	{
		y = prcBackRect->y;

		if (dwFlags & IDF_ALIGN_BOTTOM)
		{
			y += (prcBackRect->dy - nHeight);
		}
		else if (dwFlags & IDF_ALIGN_MIDDLE)
		{
			y += ((prcBackRect->dy - nHeight) >> 1);
		}
	}

	// check the x,y and rcclip
	if (x > (prcBackRect->x + prcBackRect->dx))
	{
		//Overflow the end x coordinate
		return 0;
	}

	if (y > (prcBackRect->y + prcBackRect->dy))
	{
		//Overflow the end y coordinate
		return 0;
	}

	if (prcBackRect->y > (y + nHeight))
	{
		//Overflow the start y coordinate
		return 0;
	}

	// Set correct colors
	if (dwFlags & IDF_TEXT_INVERTED)
	{
		clrBack = foreground;
		clrText = background;
	}
	else
	{
		clrBack = background;
		clrText = foreground;
	}

	clrFrame = { 255, 255, 255, 255 };

	return DrawTextEx(pMe, pDst, pcText, nChars, x, y, prcBackRect, dwFlags, clrText, clrBack, clrFrame);
}

int main(int argc, char* argv[])
{
	wchar_t text[] = L"Place your text here...";
	BMP mybitmap;
	mybitmap.width = 128;
	mybitmap.height = 160;
	mybitmap.rawp = (unsigned char*)malloc(mybitmap.width * mybitmap.height * sizeof(RGBA));
	if (!argv[1]) {
		printf("Arguments: %s <gbf/gcf file>\n", argv[0]);
		return 1;
	}
	GreyBitFont_Init(argv[1]);
	OEMFont_Create(&gFontNormal);
	OEMFont_DrawText(&gFontNormal, &mybitmap, 0, 0, text, wcslen(text), { 255, 255, 255, 255 }, { 0, 0, 0, 0}, 0, 0);
	GreyBitBrewFont_Done();
}

