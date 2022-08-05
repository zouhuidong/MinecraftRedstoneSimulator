#include "painter.h"

// 全局变量：方块图像（数组下标0表示未充能情况，数组下标为1表示充能情况）

IMAGE imgRod[2];							// 拉杆
IMAGE imgButton[2];							// 按钮
IMAGE imgTorche[2];							// 红石火把
IMAGE imgLight[2];							// 红石灯
IMAGE imgRelay[2][4];						// 旋转后的红石中继器
IMAGE imgCursor;							// 鼠标（仅显示在工具栏）

// 红石粉都是 0 表示横向，1 表示竖向
// 红石粉线条（两种状态，两种样式，两个方向）
IMAGE imgPowderLine[2][2][2];
IMAGE imgPowderSingle[2];					// 单个红石粉
IMAGE imgPowderCenter[2];					// 红石连接中心
IMAGE imgPowderEdge[2][2];					// 红石连接边缘（两个方向）

// 交叉线
// 0 - 双向都不充能
// 1 - 仅水平方向充能
// 2 - 仅竖直方向充能
// 3 - 双向都充能
IMAGE imgCross[4];

//bool bGreen = true;
//COLORREF colorPower = bGreen ? RGB(0, 240, 0) : RGB(200, 0, 0);			// 有电的颜色
//COLORREF colorNoPower = bGreen ? RGB(100, 100, 100) : RGB(100, 0, 0);	// 无电的颜色

// 红石线粗
int nPowderWidth = 7;

// 红石地图输出坐标
int nMapOutX = 0, nMapOutY = 20;

// 标尺宽高
int nRulerWidth = 20;
int nRulerHeight = 20;

// 物体大小
int nObjSize;
int nHalfObjSize;


// 加载方块图像
void loadimages()
{
	loadimage(&imgRod[0], L"./res/objs/null/rod/rod.bmp");
	loadimage(&imgRod[1], L"./res/objs/power/rod/rod.bmp");

	loadimage(&imgButton[0], L"./res/objs/null/button/button.bmp");
	loadimage(&imgButton[1], L"./res/objs/power/button/button.bmp");

	loadimage(&imgTorche[0], L"./res/objs/null/torche/torche.bmp");
	loadimage(&imgTorche[1], L"./res/objs/power/torche/torche.bmp");

	loadimage(&imgLight[0], L"./res/objs/null/light/light.bmp");
	loadimage(&imgLight[1], L"./res/objs/power/light/light.bmp");

	loadimage(&imgRelay[0][0], L"./res/objs/null/relay/relay.bmp");
	loadimage(&imgRelay[1][0], L"./res/objs/power/relay/relay.bmp");

	// 旋转中继器
	for (int i = 0; i < 2; i++)
		for (int j = 1; j < 4; j++)
			rotateimage(&imgRelay[i][j], &imgRelay[i][0], PI / 2 * j);

	loadimage(&imgPowderLine[0][0][0], L"./res/objs/null/powder/powder_line_1.bmp");
	loadimage(&imgPowderLine[0][1][0], L"./res/objs/null/powder/powder_line_2.bmp");
	loadimage(&imgPowderLine[1][0][0], L"./res/objs/power/powder/powder_line_1.bmp");
	loadimage(&imgPowderLine[1][1][0], L"./res/objs/power/powder/powder_line_2.bmp");
	loadimage(&imgPowderSingle[0], L"./res/objs/null/powder/powder_single.bmp");
	loadimage(&imgPowderSingle[1], L"./res/objs/power/powder/powder_single.bmp");
	loadimage(&imgPowderCenter[0], L"./res/objs/null/powder/powder_center.bmp");
	loadimage(&imgPowderCenter[1], L"./res/objs/power/powder/powder_center.bmp");
	loadimage(&imgPowderEdge[0][0], L"./res/objs/null/powder/powder_edge.bmp");
	loadimage(&imgPowderEdge[1][0], L"./res/objs/power/powder/powder_edge.bmp");

	// 红石线
	for (int i = 0; i < 2; i++)		// 充能状态
		for (int j = 0; j < 2; j++)	// 线样式
			rotateimage(&imgPowderLine[i][j][1], &imgPowderLine[i][j][0], PI / 2);
	// 红石连接边缘
	for (int i = 0; i < 2; i++)
		rotateimage(&imgPowderEdge[i][1], &imgPowderEdge[i][0], -PI / 2);

	loadimage(&imgCross[0], L"./res/objs/null/cross/cross.bmp");
	loadimage(&imgCross[1], L"./res/objs/power/cross/cross_horizon.bmp");
	loadimage(&imgCross[2], L"./res/objs/power/cross/cross_upright.bmp");
	loadimage(&imgCross[3], L"./res/objs/power/cross/cross_both.bmp");

	////// 加载文件完毕

	// 确定图像大小
	nObjSize = imgLight[0].getwidth();
	nHalfObjSize = nObjSize / 2;

	// 绘制鼠标
	SetWorkingImage(&imgCursor);
	imgCursor.Resize(nObjSize, nObjSize);
	POINT pCursor[9] = { {8,24},{8,3},{21,16},{21,18},{16,18},{19,25},{18,26},{16,26},{13,20} };
	polygon(pCursor, 9);
}

// 获取某个点应该使用的红石线类型
int GetPowderLineStyle(int x, int y)
{
	return (x + y) & 1;
}

// 绘制单个物体到当前画布
// 不绘制网格和坐标
void DrawSingleObject(RsMap* map, int x, int y)
{
	// 方块，周围方块
	RsObj me = map->map[y][x];
	RsObj up, down, left, right;
	if (y - 1 >= 0)			up = map->map[y - 1][x];
	if (y + 1 < map->h)		down = map->map[y + 1][x];
	if (x - 1 >= 0)			left = map->map[y][x - 1];
	if (x + 1 < map->w)		right = map->map[y][x + 1];

	// 当前方块绘制位置
	int draw_x = x * nObjSize;
	int draw_y = y * nObjSize;

	// 绘制线间隔（因为粗线条会绘制到更外面一点）
	int line_distance = 2;

	switch (me.nType)
	{
	case RS_NULL:	break;
	case RS_POWDER:
	{
		// 红石粉连接状态（xxx_connected）
		bool left_c = false;
		bool right_c = false;
		bool up_c = false;
		bool down_c = false;
		int count_c = 0;

		if (y - 1 >= 0 && up.nType != RS_NULL)
		{
			if (up.nType != RS_RELAY || up.nTowards == RS_TO_UP || up.nTowards == RS_TO_DOWN)
			{
				up_c = true;
				count_c++;
			}
		}
		if (y + 1 < map->h && down.nType != RS_NULL)
		{
			if (down.nType != RS_RELAY || down.nTowards == RS_TO_UP || down.nTowards == RS_TO_DOWN)
			{
				down_c = true;
				count_c++;
			}
		}
		if (x - 1 >= 0 && left.nType != RS_NULL)
		{
			if (left.nType != RS_RELAY || left.nTowards == RS_TO_LEFT || left.nTowards == RS_TO_RIGHT)
			{
				left_c = true;
				count_c++;
			}
		}
		if (x + 1 < map->w && right.nType != RS_NULL)
		{
			if (right.nType != RS_RELAY || right.nTowards == RS_TO_LEFT || right.nTowards == RS_TO_RIGHT)
			{
				right_c = true;
				count_c++;
			}
		}

		// 绘制
		if (!count_c)
		{
			putimage(draw_x, draw_y, &imgPowderSingle[me.bPowered]);
		}

		// 横向
		else if ((left_c || right_c) && !(up_c || down_c))
		{
			putimage(draw_x, draw_y, &imgPowderLine[me.bPowered][GetPowderLineStyle(x, y)][0]);
		}

		// 竖向
		else if (!(left_c || right_c) && (up_c || down_c))
		{
			putimage(draw_x, draw_y, &imgPowderLine[me.bPowered][GetPowderLineStyle(x, y)][1]);
		}

		// 横向和竖向
		else
		{
			int edge_len = imgPowderEdge[0][0].getwidth();		// 边缘线大小
			int distance = (nObjSize - edge_len) / 2;			// 间隙大小
			
			// 中心粉末
			putimage(draw_x, draw_y, &imgPowderCenter[me.bPowered]);

			// 边缘粉末
			if (left_c)
				putimage(draw_x, draw_y + distance, &imgPowderEdge[me.bPowered][0]);
			if (right_c)
				putimage(draw_x + nObjSize - edge_len, draw_y + distance, &imgPowderEdge[me.bPowered][0]);
			if (up_c)
				putimage(draw_x + distance, draw_y, &imgPowderEdge[me.bPowered][1]);
			if (down_c)
				putimage(draw_x + distance, draw_y + nObjSize - edge_len, &imgPowderEdge[me.bPowered][1]);
		}

	}
	break;

	case RS_ROD:		putimage(draw_x, draw_y, &imgRod[me.bPowered]);		break;
	case RS_BUTTON:		putimage(draw_x, draw_y, &imgButton[me.bPowered]);	break;
	case RS_TORCHE:		putimage(draw_x, draw_y, &imgTorche[me.bPowered]);	break;
	case RS_LIGHT:		putimage(draw_x, draw_y, &imgLight[me.bPowered]);	break;

	case RS_RELAY:
	{
		IMAGE* p = nullptr;
		switch (me.nTowards)
		{
		case RS_TO_UP:		p = &imgRelay[me.bPowered][0];		break;
		case RS_TO_LEFT:	p = &imgRelay[me.bPowered][1];		break;
		case RS_TO_DOWN:	p = &imgRelay[me.bPowered][2];		break;
		case RS_TO_RIGHT:	p = &imgRelay[me.bPowered][3];		break;
		}
		putimage(draw_x, draw_y, p);
	}
	break;

	case RS_CROSS:
	{
		if(me.bHorizonPowered && me.bUprightPowered)
			putimage(draw_x, draw_y, &imgCross[3]);
		else if(me.bHorizonPowered)
			putimage(draw_x, draw_y, &imgCross[1]);
		else if (me.bUprightPowered)
			putimage(draw_x, draw_y, &imgCross[2]);
		else
			putimage(draw_x, draw_y, &imgCross[0]);
	}
	break;

	}
}

// 绘制单个物体的坐标到当前画布
void DrawCoordinateOfSingleObject(RsMap* map, int x, int y)
{
	TCHAR strX[12] = { 0 };
	TCHAR strY[12] = { 0 };
	wsprintf(strX, L"x:%d", x);
	wsprintf(strY, L"y:%d", y);
	outtextxy(x * nObjSize, y * nObjSize, strX);
	outtextxy(x * nObjSize, y * nObjSize + textheight('0'), strY);
}

void GetRsMapImage(
	IMAGE* pImg,
	IMAGE* pImgRulerX,
	IMAGE* pImgRulerY,
	RsMap* map,
	POINT* pChange,
	int nChangeCount,
	bool redraw,
	bool resize,
	bool bShowXY,
	bool bShowRuler
)
{
	int map_size = map->w * map->h;

	// 图像属性

	// 地图像素宽高
	int nMapCanvasWidth = map->w * nObjSize;
	int nMapCanvasHeight = map->h * nObjSize;

	// 存储上次的图像
	static IMAGE imgMap;
	static IMAGE imgXRuler, imgYRuler;

	IMAGE* pOld = GetWorkingImage();

	if (resize)
	{
		SetWorkingImage(&imgMap);

		imgMap.Resize(nMapCanvasWidth, nMapCanvasHeight);
		cleardevice();

		SetWorkingImage(pOld);
	}
	if (resize)
	{
		imgXRuler.Resize(nMapCanvasWidth, nRulerHeight);
		imgYRuler.Resize(nRulerWidth, nMapCanvasHeight);
	}

	//// X 坐标超出屏幕判定
	//auto isOverscreen_X = [offset_x, zoom, nDrawArea_w](int x, int nObjSize) {
	//	return ((x + 1) * nObjSize + offset_x) * zoom < 0
	//		|| (x * nObjSize + offset_x) * zoom > nDrawArea_w;
	//};

	//// Y 坐标超出屏幕判定
	//auto isOverscreen_Y = [offset_y, zoom, nDrawArea_h](int y, int nObjSize) {
	//	return ((y + 1) * nObjSize + offset_y) * zoom < 0
	//		|| (y * nObjSize + offset_y) * zoom > nDrawArea_h;
	//};

	// 绘制
	{
		// 重绘地图
		if (redraw || resize)
		{
			SetWorkingImage(&imgMap);
			settextcolor(WHITE);
			settextstyle(10, 0, L"黑体");
			setbkmode(TRANSPARENT);

			// 不显示坐标时，背景色微调，便于辨认
			if (!bShowXY || !bShowRuler)
			{
				setbkcolor(RGB(20, 20, 20));
				cleardevice();
			}

			// 全部重绘模式
			if (nChangeCount == map_size || resize)
			{
				// 绘制地图方块
				for (int x = 0; x < map->w; x++)
				{
					for (int y = 0; y < map->h; y++)
					{
						DrawSingleObject(map, x, y);

						// 【输出坐标】模式
						if (bShowXY && !bShowRuler)
						{
							DrawCoordinateOfSingleObject(map, x, y);
						}
					}
				}

				// 网格
				if (bShowXY && bShowRuler)
				{
					// 网格线形
					setlinecolor(GRAY);
					setlinestyle(PS_DASH, 1);

					for (int x = 0; x <= map->w; x++)
					{
						int l = x * nObjSize;
						if (x == map->w)	l--;
						line(l, 0, l, getheight());
					}
					for (int y = 0; y <= map->h; y++)
					{
						int l = y * nObjSize;
						if (y == map->h)	l--;
						line(0, l, getwidth(), l);
					}
				}
			}

			// 部分重绘模式
			else
			{
				for (int i = 0; i < nChangeCount; i++)
				{
					int x = pChange[i].x;
					int y = pChange[i].y;

					clearrectangle(x * nObjSize, y * nObjSize, (x + 1) * nObjSize, (y + 1) * nObjSize);

					DrawSingleObject(map, x, y);

					// 【输出坐标】模式
					if (bShowXY && !bShowRuler)
					{
						DrawCoordinateOfSingleObject(map, x, y);
					}

					// 网格
					if (bShowXY && bShowRuler)
					{
						// 网格线形
						setlinecolor(GRAY);
						setlinestyle(PS_DASH, 1);
						rectangle(x * nObjSize, y * nObjSize, (x + 1) * nObjSize, (y + 1) * nObjSize);
					}
				}
			}

			// 大小改变
			if (resize)
			{
				// 重绘标尺
				if (bShowRuler)
				{
					// x 轴标尺
					SetWorkingImage(&imgXRuler);
					settextstyle(12, 0, L"黑体");
					setbkmode(TRANSPARENT);
					setbkcolor(BLUE);
					cleardevice();
					rectangle(0, 0, getwidth() - 1, getheight());

					for (int x = 0; x < map->w; x++)
					{
						//if (isOverscreen_X(x, nObjSize))	continue;

						line(x * nObjSize, 0, x * nObjSize, getheight());
						TCHAR str[6] = { 0 };
						wsprintf(str, L"%d", x);
						outtextxy(x * nObjSize + 5, 5, str);
					}

					// y 轴标尺
					SetWorkingImage(&imgYRuler);
					settextstyle(10, 0, L"黑体");
					setbkmode(TRANSPARENT);
					setbkcolor(BLUE);
					cleardevice();
					rectangle(0, 0, getwidth(), getheight() - 1);

					for (int y = 0; y < map->h; y++)
					{
						//if (isOverscreen_Y(y, nObjSize))	continue;

						line(0, y * nObjSize, getwidth(), y * nObjSize);
						TCHAR str[6] = { 0 };
						wsprintf(str, L"%d", y);
						outtextxy(5, y * nObjSize + 5, str);
					}
				}
			}
		}

		// 绘制完毕
		SetWorkingImage(pOld);

		*pImg = imgMap;
		*pImgRulerX = imgXRuler;
		*pImgRulerY = imgYRuler;
	}
}

// 绘制工具栏
void DrawToolBar_painter(int indexSelected)
{
	cleardevice();
	setlinestyle(PS_SOLID, 2);
	IMAGE* pImg[8] = { &imgCursor,&imgPowderCenter[0],&imgRod[0],&imgButton[0],
		&imgTorche[0],&imgLight[0],&imgRelay[0][0],&imgCross[2]};
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			int id = i * 2 + j;
			putimage(j * nObjSize + 1, i * nObjSize + 1, pImg[id]);
			if (indexSelected == id)
			{
				int x = j * nObjSize;
				int y = i * nObjSize;
				rectangle(x, y, x + nObjSize, y + nObjSize);
			}
		}
	}
}

