#pragma once

#include "redstone.h"

#include "HiEasyX/HiMacro.h"

extern int nMapOutX, nMapOutY;
extern int nRulerWidth, nRulerHeight;
extern int nObjSize, nHalfObjSize;

// 加载方块图像
void loadimages();

// 得到地图画面
void GetRsMapImage(
	IMAGE* pImg,			// 输出绘制的地图
	IMAGE* pImgRulerX,		// 输出 X 轴标尺（如果选择绘制标尺）
	IMAGE* pImgRulerY,		// 输出 Y 轴标尺（如果选择绘制标尺）
	RsMap* map,				// 地图
	POINT* pChange,			// 变更点数组
	int nChangeCount,		// 变更点数量
	bool redraw,			// 是否重新绘制地图
	bool resize,			// 地图尺寸是否更新
	bool bShowXY,			// 是否显示坐标
	bool bShowRuler			// 是否以标尺形式显示坐标
);

// 绘制工具栏
void DrawToolBar_painter(int indexSelected);
