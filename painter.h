#pragma once

#include "redstone.h"

#include "HiEasyX/HiMacro.h"

extern int nMapOutX, nMapOutY;
extern int nRulerWidth, nRulerHeight;
extern int nObjSize, nHalfObjSize;

// ���ط���ͼ��
void loadimages();

// �õ���ͼ����
void GetRsMapImage(
	IMAGE* pImg,			// ������Ƶĵ�ͼ
	IMAGE* pImgRulerX,		// ��� X ���ߣ����ѡ����Ʊ�ߣ�
	IMAGE* pImgRulerY,		// ��� Y ���ߣ����ѡ����Ʊ�ߣ�
	RsMap* map,				// ��ͼ
	bool redraw,			// �Ƿ����»��Ƶ�ͼ
	bool resize,			// ��ͼ�ߴ��Ƿ����
	bool bShowXY,			// �Ƿ���ʾ����
	bool bShowRuler			// �Ƿ��Ա����ʽ��ʾ����
);

// ���ƹ�����
void DrawToolBar_painter(int indexSelected);
