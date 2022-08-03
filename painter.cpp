#include "painter.h"

// 全局变量：方块图像（数组下标0表示未充能情况，数组下标为1表示充能情况）

IMAGE imgRod[2];							// 拉杆
IMAGE imgButton[2];							// 按钮
IMAGE imgTorche[2];							// 红石火把
IMAGE imgLight[2];							// 红石灯
IMAGE imgRelay[2];							// 红石中继器
IMAGE imgRelayRotated[2][3];				// 旋转后的红石中继器
IMAGE imgCursor;							// 鼠标（仅显示在工具栏）
IMAGE imgPowder;							// 红石粉（仅显示在工具栏）
IMAGE imgCross;								// 交叉线（仅显示在工具栏）

bool bGreen = true;
COLORREF colorPower = bGreen ? RGB(0, 240, 0) : RGB(200, 0, 0);			// 有电的颜色
COLORREF colorNoPower = bGreen ? RGB(100, 100, 100) : RGB(100, 0, 0);	// 无电的颜色

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

	loadimage(&imgRelay[0], L"./res/objs/null/relay/relay.bmp");
	loadimage(&imgRelay[1], L"./res/objs/power/relay/relay.bmp");

	// 旋转中继器
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++)
			rotateimage(&imgRelayRotated[i][j], &imgRelay[i], PI / 2 * (j + 1));

	// 确定图像大小
	nObjSize = imgLight[0].getwidth();
	nHalfObjSize = nObjSize / 2;

	// 绘制鼠标
	SetWorkingImage(&imgCursor);
	imgCursor.Resize(nObjSize, nObjSize);
	POINT pCursor[9] = { {8,24},{8,3},{21,16},{21,18},{16,18},{19,25},{18,26},{16,26},{13,20} };
	polygon(pCursor, 9);

	// 粉末
	SetWorkingImage(&imgPowder);
	imgPowder.Resize(nObjSize, nObjSize);
	POINT pPowder[4] = { {10,27},{10,12},{19,12},{19,2} };
	setlinestyle(PS_SOLID, nPowderWidth);
	setlinecolor(colorPower);
	polyline(pPowder, 4);

	// 交叉线
	SetWorkingImage(&imgCross);
	imgCross.Resize(nObjSize, nObjSize);
	setlinestyle(PS_SOLID, nPowderWidth);
	setlinecolor(colorPower);
	line(nHalfObjSize, 10, nHalfObjSize, nObjSize);
	setlinecolor(colorNoPower);
	POINT pCross[5] = { { 0, nHalfObjSize + 10 },{ 5, nHalfObjSize + 10 },
		{ nHalfObjSize, nHalfObjSize / 2 + 10 },{ nObjSize - 5, nHalfObjSize + 10 },{ nObjSize, nHalfObjSize + 10 } };
	polyline(pCross, 5);
}


// 得到地图画面
void GetRsMapImage(
	IMAGE* pImg,			// 输出绘制的地图
	IMAGE* pImgRulerX,		// 输出 X 轴标尺（如果选择绘制标尺）
	IMAGE* pImgRulerY,		// 输出 Y 轴标尺（如果选择绘制标尺）
	RsMap* map,				// 地图
	bool redraw,			// 是否重新绘制地图
	bool resize,			// 地图尺寸是否更新
	bool bShowXY,			// 是否显示坐标
	bool bShowRuler			// 是否以标尺形式显示坐标
)
{
	// 图像属性

	// 地图像素宽高
	int nMapCanvasWidth = map->w * nObjSize;
	int nMapCanvasHeight = map->h * nObjSize;

	// 存储上次的图像
	static IMAGE imgMap;
	static IMAGE imgXRuler, imgYRuler;

	IMAGE* pOld = GetWorkingImage();

	if (redraw || resize)
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

			// 绘制地图方块
			for (int x = 0; x < map->w; x++)
			{
				//if (isOverscreen_X(x, nObjSize))		continue;

				for (int y = 0; y < map->h; y++)
				{
					//if (isOverscreen_Y(y, nObjSize))	continue;

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

					switch (me.nType)
					{
					case RS_NULL:	break;
					case RS_POWDER:
					{
						// 该红石粉是否连接上周围物体
						bool bConnect = false;
						//SetWorkingImage(&powder);

						if (me.bPowered)
						{
							setfillcolor(colorPower);
							setlinecolor(colorPower);
						}
						else
						{
							setfillcolor(colorNoPower);
							setlinecolor(colorNoPower);
						}

						//fillcircle(draw_x + nHalfObjSize, draw_y + nHalfObjSize, nPowderWidth / 2 - 1);
						setlinestyle(PS_SOLID, nPowderWidth);

						int distance = 2;

						// 实时绘制红石粉
						if (y - 1 >= 0 && up.nType != RS_NULL)			// line to up
						{
							if (up.nType != RS_RELAY || up.nTowards == RS_TO_UP || up.nTowards == RS_TO_DOWN)
							{
								line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x + nHalfObjSize, draw_y + distance);
								bConnect = true;
							}
						}
						if (y + 1 < map->h && down.nType != RS_NULL)	// line to down
						{
							if (down.nType != RS_RELAY || down.nTowards == RS_TO_UP || down.nTowards == RS_TO_DOWN)
							{
								line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x + nHalfObjSize, draw_y + nObjSize - distance);
								bConnect = true;
							}
						}
						if (x - 1 >= 0 && left.nType != RS_NULL)		// line to left
						{
							if (left.nType != RS_RELAY || left.nTowards == RS_TO_LEFT || left.nTowards == RS_TO_RIGHT)
							{
								line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x + distance, draw_y + nHalfObjSize);
								bConnect = true;
							}
						}
						if (x + 1 < map->w && right.nType != RS_NULL)	// line to right
						{
							if (right.nType != RS_RELAY || right.nTowards == RS_TO_LEFT || right.nTowards == RS_TO_RIGHT)
							{
								line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x + nObjSize - distance, draw_y + nHalfObjSize);
								bConnect = true;
							}
						}

						if (!bConnect)
						{
							fillcircle(draw_x + nHalfObjSize, draw_y + nHalfObjSize, nPowderWidth);
						}

						//putimage(l * nObjSize, l * nObjSize, &powder);
					}
					break;

					case RS_ROD:		putimage(draw_x, draw_y, &imgRod[me.bPowered]);		break;
					case RS_BUTTON:		putimage(draw_x, draw_y, &imgButton[me.bPowered]);	break;
					case RS_TORCHE:		putimage(draw_x, draw_y, &imgTorche[me.bPowered]);	break;
					case RS_LIGHT:		putimage(draw_x, draw_y, &imgLight[me.bPowered]);		break;

					case RS_RELAY:
					{
						IMAGE* p = NULL;
						switch (me.nTowards)
						{
						case RS_TO_UP:		p = &imgRelay[me.bPowered];				break;
						case RS_TO_LEFT:	p = &imgRelayRotated[me.bPowered][0];		break;
						case RS_TO_DOWN:	p = &imgRelayRotated[me.bPowered][1];		break;
						case RS_TO_RIGHT:	p = &imgRelayRotated[me.bPowered][2];		break;
						}
						putimage(draw_x, draw_y, p);
					}
					break;

					case RS_CROSS:
					{
						setlinestyle(PS_SOLID, nPowderWidth);

						if (me.bUprightPowered)
						{
							setfillcolor(colorPower);
							setlinecolor(colorPower);
						}
						else
						{
							setfillcolor(colorNoPower);
							setlinecolor(colorNoPower);
						}

						line(draw_x + nHalfObjSize, draw_y, draw_x + nHalfObjSize, draw_y + nObjSize);	// 竖向电路

						if (me.bHorizonPowered)
						{
							setfillcolor(colorPower);
							setlinecolor(colorPower);
						}
						else
						{
							setfillcolor(colorNoPower);
							setlinecolor(colorNoPower);
						}

						// 交叉线绘制点位
						POINT pCrossHLine[3] = {
							{ draw_x,draw_y + nHalfObjSize },
							{ draw_x + nHalfObjSize,draw_y + nHalfObjSize / 2 },
							{ draw_x + nObjSize,draw_y + nHalfObjSize }
						};

						// 横向电路（弯曲）
						polyline(pCrossHLine, 3);
					}
					break;

					}

					// 输出坐标模式
					if (bShowXY && !bShowRuler)
					{
						TCHAR strX[12] = { 0 };
						TCHAR strY[12] = { 0 };
						wsprintf(strX, L"x:%d", x);
						wsprintf(strY, L"y:%d", y);
						outtextxy(x * nObjSize, y * nObjSize, strX);
						outtextxy(x * nObjSize, y * nObjSize + textheight('0'), strY);
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
					//if (isOverscreen_X(x, nObjSize))	continue;

					int l = x * nObjSize;
					if (x == map->w)	l--;
					line(l, 0, l, getheight());
				}
				for (int y = 0; y <= map->h; y++)
				{
					//if (isOverscreen_Y(y, nObjSize))	continue;

					int l = y * nObjSize;
					if (y == map->h)	l--;
					line(0, l, getwidth(), l);
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

// 绘制工具栏
void DrawToolBar_painter(int indexSelected)
{
	cleardevice();
	setlinestyle(PS_SOLID, 2);
	IMAGE* pImg[8] = { &imgCursor,&imgPowder,&imgRod[0],&imgButton[0],
		&imgTorche[0],&imgLight[0],&imgRelay[0],&imgCross };
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

