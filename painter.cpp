#include "painter.h"

// ȫ�ֱ���������ͼ�������±�0��ʾδ��������������±�Ϊ1��ʾ���������

IMAGE imgRod[2];							// ����
IMAGE imgButton[2];							// ��ť
IMAGE imgTorche[2];							// ��ʯ���
IMAGE imgLight[2];							// ��ʯ��
IMAGE imgRelay[2][4];						// ��ת��ĺ�ʯ�м���
IMAGE imgCursor;							// ��꣨����ʾ�ڹ�������

// ��ʯ�۶��� 0 ��ʾ����1 ��ʾ����
// ��ʯ������������״̬��������ʽ����������
IMAGE imgPowderLine[2][2][2];
IMAGE imgPowderSingle[2];					// ������ʯ��
IMAGE imgPowderCenter[2];					// ��ʯ��������
IMAGE imgPowderEdge[2][2];					// ��ʯ���ӱ�Ե����������

// ������
// 0 - ˫�򶼲�����
// 1 - ��ˮƽ�������
// 2 - ����ֱ�������
// 3 - ˫�򶼳���
IMAGE imgCross[4];

//bool bGreen = true;
//COLORREF colorPower = bGreen ? RGB(0, 240, 0) : RGB(200, 0, 0);			// �е����ɫ
//COLORREF colorNoPower = bGreen ? RGB(100, 100, 100) : RGB(100, 0, 0);	// �޵����ɫ

// ��ʯ�ߴ�
int nPowderWidth = 7;

// ��ʯ��ͼ�������
int nMapOutX = 0, nMapOutY = 20;

// ��߿��
int nRulerWidth = 20;
int nRulerHeight = 20;

// �����С
int nObjSize;
int nHalfObjSize;


// ���ط���ͼ��
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

	// ��ת�м���
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

	// ��ʯ��
	for (int i = 0; i < 2; i++)		// ����״̬
		for (int j = 0; j < 2; j++)	// ����ʽ
			rotateimage(&imgPowderLine[i][j][1], &imgPowderLine[i][j][0], PI / 2);
	// ��ʯ���ӱ�Ե
	for (int i = 0; i < 2; i++)
		rotateimage(&imgPowderEdge[i][1], &imgPowderEdge[i][0], -PI / 2);

	loadimage(&imgCross[0], L"./res/objs/null/cross/cross.bmp");
	loadimage(&imgCross[1], L"./res/objs/power/cross/cross_horizon.bmp");
	loadimage(&imgCross[2], L"./res/objs/power/cross/cross_upright.bmp");
	loadimage(&imgCross[3], L"./res/objs/power/cross/cross_both.bmp");

	////// �����ļ����

	// ȷ��ͼ���С
	nObjSize = imgLight[0].getwidth();
	nHalfObjSize = nObjSize / 2;

	// �������
	SetWorkingImage(&imgCursor);
	imgCursor.Resize(nObjSize, nObjSize);
	POINT pCursor[9] = { {8,24},{8,3},{21,16},{21,18},{16,18},{19,25},{18,26},{16,26},{13,20} };
	polygon(pCursor, 9);
}

// ��ȡĳ����Ӧ��ʹ�õĺ�ʯ������
int GetPowderLineStyle(int x, int y)
{
	return (x + y) & 1;
}

// ���Ƶ������嵽��ǰ����
// ���������������
void DrawSingleObject(RsMap* map, int x, int y)
{
	// ���飬��Χ����
	RsObj me = map->map[y][x];
	RsObj up, down, left, right;
	if (y - 1 >= 0)			up = map->map[y - 1][x];
	if (y + 1 < map->h)		down = map->map[y + 1][x];
	if (x - 1 >= 0)			left = map->map[y][x - 1];
	if (x + 1 < map->w)		right = map->map[y][x + 1];

	// ��ǰ�������λ��
	int draw_x = x * nObjSize;
	int draw_y = y * nObjSize;

	// �����߼������Ϊ����������Ƶ�������һ�㣩
	int line_distance = 2;

	switch (me.nType)
	{
	case RS_NULL:	break;
	case RS_POWDER:
	{
		// ��ʯ������״̬��xxx_connected��
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

		// ����
		if (!count_c)
		{
			putimage(draw_x, draw_y, &imgPowderSingle[me.bPowered]);
		}

		// ����
		else if ((left_c || right_c) && !(up_c || down_c))
		{
			putimage(draw_x, draw_y, &imgPowderLine[me.bPowered][GetPowderLineStyle(x, y)][0]);
		}

		// ����
		else if (!(left_c || right_c) && (up_c || down_c))
		{
			putimage(draw_x, draw_y, &imgPowderLine[me.bPowered][GetPowderLineStyle(x, y)][1]);
		}

		// ���������
		else
		{
			int edge_len = imgPowderEdge[0][0].getwidth();		// ��Ե�ߴ�С
			int distance = (nObjSize - edge_len) / 2;			// ��϶��С
			
			// ���ķ�ĩ
			putimage(draw_x, draw_y, &imgPowderCenter[me.bPowered]);

			// ��Ե��ĩ
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

// ���Ƶ�����������굽��ǰ����
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

	// ͼ������

	// ��ͼ���ؿ��
	int nMapCanvasWidth = map->w * nObjSize;
	int nMapCanvasHeight = map->h * nObjSize;

	// �洢�ϴε�ͼ��
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

	//// X ���곬����Ļ�ж�
	//auto isOverscreen_X = [offset_x, zoom, nDrawArea_w](int x, int nObjSize) {
	//	return ((x + 1) * nObjSize + offset_x) * zoom < 0
	//		|| (x * nObjSize + offset_x) * zoom > nDrawArea_w;
	//};

	//// Y ���곬����Ļ�ж�
	//auto isOverscreen_Y = [offset_y, zoom, nDrawArea_h](int y, int nObjSize) {
	//	return ((y + 1) * nObjSize + offset_y) * zoom < 0
	//		|| (y * nObjSize + offset_y) * zoom > nDrawArea_h;
	//};

	// ����
	{
		// �ػ��ͼ
		if (redraw || resize)
		{
			SetWorkingImage(&imgMap);
			settextcolor(WHITE);
			settextstyle(10, 0, L"����");
			setbkmode(TRANSPARENT);

			// ����ʾ����ʱ������ɫ΢�������ڱ���
			if (!bShowXY || !bShowRuler)
			{
				setbkcolor(RGB(20, 20, 20));
				cleardevice();
			}

			// ȫ���ػ�ģʽ
			if (nChangeCount == map_size || resize)
			{
				// ���Ƶ�ͼ����
				for (int x = 0; x < map->w; x++)
				{
					for (int y = 0; y < map->h; y++)
					{
						DrawSingleObject(map, x, y);

						// ��������꡿ģʽ
						if (bShowXY && !bShowRuler)
						{
							DrawCoordinateOfSingleObject(map, x, y);
						}
					}
				}

				// ����
				if (bShowXY && bShowRuler)
				{
					// ��������
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

			// �����ػ�ģʽ
			else
			{
				for (int i = 0; i < nChangeCount; i++)
				{
					int x = pChange[i].x;
					int y = pChange[i].y;

					clearrectangle(x * nObjSize, y * nObjSize, (x + 1) * nObjSize, (y + 1) * nObjSize);

					DrawSingleObject(map, x, y);

					// ��������꡿ģʽ
					if (bShowXY && !bShowRuler)
					{
						DrawCoordinateOfSingleObject(map, x, y);
					}

					// ����
					if (bShowXY && bShowRuler)
					{
						// ��������
						setlinecolor(GRAY);
						setlinestyle(PS_DASH, 1);
						rectangle(x * nObjSize, y * nObjSize, (x + 1) * nObjSize, (y + 1) * nObjSize);
					}
				}
			}

			// ��С�ı�
			if (resize)
			{
				// �ػ���
				if (bShowRuler)
				{
					// x ����
					SetWorkingImage(&imgXRuler);
					settextstyle(12, 0, L"����");
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

					// y ����
					SetWorkingImage(&imgYRuler);
					settextstyle(10, 0, L"����");
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

		// �������
		SetWorkingImage(pOld);

		*pImg = imgMap;
		*pImgRulerX = imgXRuler;
		*pImgRulerY = imgYRuler;
	}
}

// ���ƹ�����
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

