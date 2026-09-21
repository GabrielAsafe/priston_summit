#include "systems/ui/core/UILinker.h"

vector<LOADEDIMAGE*> vLoadedImages;

namespace
{
	// All 2D UI assets were upscaled 4x. Keep that extra resolution in the
	// texture, but expose the original 1x size to layout/hit-testing.
	const UINT UI_ASSET_SCALE = 4;

	int GetLogicalUISize(UINT sourceSize, UINT residentSize)
	{
		const UINT authoredSize = sourceSize ? sourceSize : residentSize;
		if (!authoredSize)
			return 0;

		const UINT logicalSize = authoredSize / UI_ASSET_SCALE;
		return logicalSize ? static_cast<int>(logicalSize) : 1;
	}
}

void ClearLoadedImages()
{
	vLoadedImages.clear();
}

LOADEDIMAGE * GetUIImage(char * path)
{
	for (auto pImage : vLoadedImages)
	{
		if (!lstrcmpi(pImage->ImagePath, path))
			return pImage;
	}
	return nullptr;
}

LPDIRECT3DTEXTURE9 LoadUIImage(char * path)
{
	LOADEDIMAGE* pImage = GetUIImage(path);
	if (pImage)
		return pImage->Image;

	pImage = new LOADEDIMAGE;
	if (pImage)
	{
		pImage->SourceWidth = 0;
		pImage->SourceHeight = 0;
		pImage->Image = LoadDibSurfaceOffscreenWithInfo(path, &pImage->SourceWidth, &pImage->SourceHeight);
		lstrcpy(pImage->ImagePath, path);
		vLoadedImages.push_back(pImage);
		return pImage->Image;
	}

	return nullptr;
}

UIImage::UIImage(int iID, char * path, int x, int y, int w, int h)
{
	setID(iID);
	setX(x);
	setY(y);
	startX = 0;
	startY = 0;

	Image = LoadUIImage(path);

	if (Image)
	{
		D3DSURFACE_DESC ImageDesc;
		Image->GetLevelDesc(0, &ImageDesc);

		LOADEDIMAGE *pLoadedImage = GetUIImage(path);
		const UINT sourceWidth = pLoadedImage ? pLoadedImage->SourceWidth : 0;
		const UINT sourceHeight = pLoadedImage ? pLoadedImage->SourceHeight : 0;
		const int logicalWidth = GetLogicalUISize(sourceWidth, ImageDesc.Width);
		const int logicalHeight = GetLogicalUISize(sourceHeight, ImageDesc.Height);

		// Width/height are layout dimensions. A caller can override either one;
		// otherwise use the pre-upscale 1x size from the source file.
		setWidth(w > 0 ? w : logicalWidth);
		setHeight(h > 0 ? h : logicalHeight);

		// trueWidth/trueHeight are the actual resident texture dimensions. They
		// may be below the source dimensions because the runtime texture policy
		// can cap very large files (e.g. a 2240px asset resident at 2048px).
		trueWidth = ImageDesc.Width;
		trueHeight = ImageDesc.Height;
	}
}

void UIImage::Render(int iX, int iY, int iWidth, int iHeight, int iSourceX, int iSourceY)
{
	if (!isVisible() || !Image || iWidth <= 0 || iHeight <= 0)
		return;

	const int cX = getX();
	const int cY = getY();
	const int cWidth = getWidth();
	const int cHeight = getHeight();
	if (cWidth <= 0 || cHeight <= 0)
		return;

	// Work entirely in logical (1x) UI coordinates for layout and clipping.
	int logicalLeft = cX < iSourceX ? (iSourceX - cX) : 0;
	int logicalTop = cY < iSourceY ? (iSourceY - cY) : 0;
	int logicalRight = cWidth;
	int logicalBottom = cHeight;

	if (cX + logicalRight > iWidth)
		logicalRight = iWidth - cX;
	if (cY + logicalBottom > iHeight)
		logicalBottom = iHeight - cY;

	if (logicalLeft < 0) logicalLeft = 0;
	if (logicalTop < 0) logicalTop = 0;
	if (logicalRight > cWidth) logicalRight = cWidth;
	if (logicalBottom > cHeight) logicalBottom = cHeight;

	if (logicalRight <= logicalLeft || logicalBottom <= logicalTop)
		return;

	const int iRenderX = cX + iX + logicalLeft;
	const int iRenderY = cY + iY + logicalTop;
	const int iRenderWidth = logicalRight - logicalLeft;
	const int iRenderHeight = logicalBottom - logicalTop;

	RECT srcRect;
	if (eDrawStyle == EDRAW_Adjust)
	{
		// Map the visible logical region onto the full resident texture. This
		// keeps the high-resolution asset while drawing it at the old UI size.
		srcRect.left = startX + static_cast<LONG>((static_cast<long long>(logicalLeft) * trueWidth) / cWidth);
		srcRect.top = startY + static_cast<LONG>((static_cast<long long>(logicalTop) * trueHeight) / cHeight);
		srcRect.right = startX + static_cast<LONG>((static_cast<long long>(logicalRight) * trueWidth) / cWidth);
		srcRect.bottom = startY + static_cast<LONG>((static_cast<long long>(logicalBottom) * trueHeight) / cHeight);
	}
	else
	{
		srcRect.left = startX + logicalLeft;
		srcRect.top = startY + logicalTop;
		srcRect.right = startX + logicalRight;
		srcRect.bottom = startY + logicalBottom;
	}

	RECT destRect = { iRenderX, iRenderY, iRenderX + iRenderWidth, iRenderY + iRenderHeight };
	DrawTexture2D(Image, destRect, srcRect, TRUE);

	if (isMousePos(iRenderX, iRenderY, iRenderWidth, iRenderHeight) && fHover)
		fHover();
}

void UIImage::SetImage(char * path)
{
	Image = LoadUIImage(path);

	if (Image)
	{
		D3DSURFACE_DESC ImageDesc;
		Image->GetLevelDesc(0, &ImageDesc);

		LOADEDIMAGE *pLoadedImage = GetUIImage(path);
		const UINT sourceWidth = pLoadedImage ? pLoadedImage->SourceWidth : 0;
		const UINT sourceHeight = pLoadedImage ? pLoadedImage->SourceHeight : 0;

		setWidth(GetLogicalUISize(sourceWidth, ImageDesc.Width));
		setHeight(GetLogicalUISize(sourceHeight, ImageDesc.Height));

		trueWidth = ImageDesc.Width;
		trueHeight = ImageDesc.Height;
	}
}




