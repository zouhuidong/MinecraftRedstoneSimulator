#include "HiFunc.h"

#include <iostream>
#include <time.h>


// ��ȡͼ��ߴ�
// ���Է���ش��� IMAGE ָ��Ϊ�գ���ָ������ͼ���ڵ����
void GetImageSize(IMAGE* pImg, int& width, int& height)
{
	// ��ͨ IMAGE ָ��
	if (pImg)
	{
		width = pImg->getwidth();
		height = pImg->getheight();
	}

	// nullptr ��Ӧ��ͼ����
	else
	{
		IMAGE* pOld = GetWorkingImage();
		SetWorkingImage();
		width = getwidth();
		height = getheight();
		SetWorkingImage(pOld);
	}
}

HBITMAP Image2Bitmap(IMAGE* img)
{
	return CreateBitmap(img->getwidth(), img->getheight(), 1, 32, (void*)GetImageBuffer(img));
}

HICON Bitmap2Icon(HBITMAP hBmp)
{
	BITMAP bmp = {};
	GetObject(hBmp, sizeof(BITMAP), &bmp);

	HBITMAP hbmMask = CreateCompatibleBitmap(GetDC(nullptr), bmp.bmWidth, bmp.bmHeight);

	ICONINFO ii = { 0 };
	ii.fIcon = TRUE;
	ii.hbmColor = hBmp;
	ii.hbmMask = hbmMask;

	HICON hIcon = CreateIconIndirect(&ii);
	DeleteObject(hbmMask);

	return hIcon;
}

void HpSleep(int ms)
{
	static clock_t oldclock = clock();		// ��̬��������¼��һ�� tick
	static int c = CLOCKS_PER_SEC / 1000;

	oldclock += ms * c;						// ���� tick

	if (clock() > oldclock)					// ����Ѿ���ʱ��������ʱ
		oldclock = clock();
	else
		while (clock() < oldclock)			// ��ʱ
			Sleep(1);						// �ͷ� CPU ����Ȩ������ CPU ռ����
//			Sleep(0);						// ���߾��ȡ����� CPU ռ����
}

bool isInRect(int x, int y, RECT rct)
{
	if (rct.left > rct.right)    std::swap(rct.left, rct.right);
	if (rct.top > rct.bottom)    std::swap(rct.top, rct.bottom);
	return x >= rct.left && x <= rct.right && y >= rct.top && y <= rct.bottom;
}

UINT GetExMessageType(ExMessage msg)
{
	switch (msg.message)
	{
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDBLCLK:
		return EM_MOUSE;
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		return EM_KEY;
		break;
	case WM_CHAR:
		return EM_CHAR;
		break;
	case WM_ACTIVATE:
	case WM_MOVE:
	case WM_SIZE:
		return EM_WINDOW;
		break;
	default:
		return 0;
		break;
	}
}
