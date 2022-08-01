#include "HiStart.h"

#include <graphics.h>
#include <time.h>

// ����ȫ�ֱ���
POINT* g_pDst;     // �㼯(Ŀ��)
POINT* g_pSrc;     // �㼯(Դ)
int g_nWidth;      // ���ֵĿ��
int g_nHeight;     // ���ֵĸ߶�
int g_nCount;      // �㼯�����ĵ������

// �Ƿ�Ϊ��Ե��
bool isEdgePoint(int x, int y)
{
	//return getpixel(x, y) == WHITE;

	//POINT t[4] = { {0,1}, {0,-1}, {1,0}, {-1,0} };
	//POINT t[4] = { {0,2}, {0,-2}, {2,0}, {-2,0} };
	POINT t[4] = { {0,4}, {0,-1}, {4,0}, {-1,0} };
	if (getpixel(x, y) != WHITE)
	{
		return false;
	}
	for (int i = 0; i < 4; i++)
	{
		if (getpixel(x + t[i].x, y + t[i].y) != WHITE)
		{
			return true;
		}
	}
	return false;
}

// ��ȡĿ��㼯
void GetDstPoints()
{
	// ������ʱ��ͼ����
	IMAGE img;
	SetWorkingImage(&img);

	// ����Ŀ���ַ���
	TCHAR s[] = _T("EasyX");

	// ����Ŀ���ַ����Ŀ�ߣ���������ʱ��ͼ����ĳߴ�
	setcolor(WHITE);
	setfont(100, 0, _T("Arial"), 5, 0, 1000, true, false, false);
	g_nWidth = textwidth(s);
	g_nHeight = textheight(s);
	Resize(&img, g_nWidth, g_nHeight);

	// ���Ŀ���ַ����� img ����
	outtextxy(0, 0, s);

	// ���㹹��Ŀ���ַ����ĵ������
	int x, y;
	g_nCount = 0;
	for (x = 0; x < g_nWidth; x++)
		for (y = 0; y < g_nHeight; y++)
			if (isEdgePoint(x, y))
				g_nCount++;

	// ����Ŀ������
	g_pDst = new POINT[g_nCount];
	int i = 0;
	for (x = 0; x < g_nWidth; x++)
		for (y = 0; y < g_nHeight; y++)
			if (isEdgePoint(x, y))
			{
				g_pDst[i].x = x + (640 - g_nWidth) / 2;
				g_pDst[i].y = y + (480 - g_nHeight) / 2 - 20;
				i++;
			}

	// �ָ�����Ļ�Ļ�ͼ����
	SetWorkingImage(NULL);
}

// ��ȡԴ�㼯
void GetSrcPoints()
{
	// �����������
	srand((unsigned int)time(NULL));

	// ���������Դ����
	g_pSrc = new POINT[g_nCount];
	for (int i = 0; i < g_nCount; i++)
	{
		g_pSrc[i].x = rand() % 640;
		g_pSrc[i].y = rand() % 480;
	}
}

// ȫ��ģ������(������Ļ��һ�к����һ��)
void Blur(DWORD* pMem)
{
	for (int i = 640; i < 640 * 479; i++)
	{
		pMem[i] = RGB(
			(GetRValue(pMem[i]) + GetRValue(pMem[i - 640]) + GetRValue(pMem[i - 1]) + GetRValue(pMem[i + 1]) + GetRValue(pMem[i + 640])) / 5.f,
			(GetGValue(pMem[i]) + GetGValue(pMem[i - 640]) + GetGValue(pMem[i - 1]) + GetGValue(pMem[i + 1]) + GetGValue(pMem[i + 640])) / 5.1f,
			(GetBValue(pMem[i]) + GetBValue(pMem[i - 640]) + GetBValue(pMem[i - 1]) + GetBValue(pMem[i + 1]) + GetBValue(pMem[i + 640])) / 5.1f);
	}
}

// ������
void HiEasyX::RenderStartScene()
{
	initgraph(640, 480);				// ������ͼ����
	setbkcolor(RGB(80, 80, 80));
	cleardevice();
	setbkmode(TRANSPARENT);

	DWORD* pBuf = GetImageBuffer();

	GetDstPoints();						// ��ȡĿ��㼯
	GetSrcPoints();						// ��ȡԴ�㼯

	// ����
	int x, y;
	for (int i = 2; i <= 256; i += 2)
	{
		//int gray = i - 1 - 50;		// ��
		//int gray = i - 1 + 10;		// ��
		int gray = i - 1 + 100;			// ����
		if (gray < 0) gray = 0;
		if (gray > 255) gray = 255;
		COLORREF c = RGB(gray, gray, gray);
		Blur(pBuf);                     // ȫ��ģ������

		for (int d = 0; d < g_nCount; d++)
		{
			x = g_pSrc[d].x + (g_pDst[d].x - g_pSrc[d].x) * i / 256;
			y = g_pSrc[d].y + (g_pDst[d].y - g_pSrc[d].y) * i / 256;
			pBuf[y * 640 + x] = c;        // ֱ�Ӳ�����ʾ����������
			//putpixel(g_pDst[d].x, g_pDst[d].y, WHITE);
		}

		LPCTSTR lpszText = L"Animation Author :  ������";
		outtextxy(getwidth() - textwidth(lpszText), getheight() - textheight(lpszText), lpszText);

		Sleep(10);                    	// ��ʱ
	}

	// �����ڴ�
	delete g_pDst;
	delete g_pSrc;

	Sleep(2000);
	ShowWindow(GetHWnd(), SW_HIDE);
}
