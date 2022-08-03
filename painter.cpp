#include "painter.h"

// ȫ�ֱ���������ͼ�������±�0��ʾδ��������������±�Ϊ1��ʾ���������

IMAGE imgRod[2];							// ����
IMAGE imgButton[2];							// ��ť
IMAGE imgTorche[2];							// ��ʯ���
IMAGE imgLight[2];							// ��ʯ��
IMAGE imgRelay[2];							// ��ʯ�м���
IMAGE imgRelayRotated[2][3];				// ��ת��ĺ�ʯ�м���
IMAGE imgCursor;							// ��꣨����ʾ�ڹ�������
IMAGE imgPowder;							// ��ʯ�ۣ�����ʾ�ڹ�������
IMAGE imgCross;								// �����ߣ�����ʾ�ڹ�������

bool bGreen = true;
COLORREF colorPower = bGreen ? RGB(0, 240, 0) : RGB(200, 0, 0);			// �е����ɫ
COLORREF colorNoPower = bGreen ? RGB(100, 100, 100) : RGB(100, 0, 0);	// �޵����ɫ

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

	loadimage(&imgRelay[0], L"./res/objs/null/relay/relay.bmp");
	loadimage(&imgRelay[1], L"./res/objs/power/relay/relay.bmp");

	// ��ת�м���
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++)
			rotateimage(&imgRelayRotated[i][j], &imgRelay[i], PI / 2 * (j + 1));

	// ȷ��ͼ���С
	nObjSize = imgLight[0].getwidth();
	nHalfObjSize = nObjSize / 2;

	// �������
	SetWorkingImage(&imgCursor);
	imgCursor.Resize(nObjSize, nObjSize);
	POINT pCursor[9] = { {8,24},{8,3},{21,16},{21,18},{16,18},{19,25},{18,26},{16,26},{13,20} };
	polygon(pCursor, 9);

	// ��ĩ
	SetWorkingImage(&imgPowder);
	imgPowder.Resize(nObjSize, nObjSize);
	POINT pPowder[4] = { {10,27},{10,12},{19,12},{19,2} };
	setlinestyle(PS_SOLID, nPowderWidth);
	setlinecolor(colorPower);
	polyline(pPowder, 4);

	// ������
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
)
{
	// ͼ������

	// ��ͼ���ؿ��
	int nMapCanvasWidth = map->w * nObjSize;
	int nMapCanvasHeight = map->h * nObjSize;

	// �洢�ϴε�ͼ��
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

			// ���Ƶ�ͼ����
			for (int x = 0; x < map->w; x++)
			{
				//if (isOverscreen_X(x, nObjSize))		continue;

				for (int y = 0; y < map->h; y++)
				{
					//if (isOverscreen_Y(y, nObjSize))	continue;

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

					switch (me.nType)
					{
					case RS_NULL:	break;
					case RS_POWDER:
					{
						// �ú�ʯ���Ƿ���������Χ����
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

						// ʵʱ���ƺ�ʯ��
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

						line(draw_x + nHalfObjSize, draw_y, draw_x + nHalfObjSize, draw_y + nObjSize);	// �����·

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

						// �����߻��Ƶ�λ
						POINT pCrossHLine[3] = {
							{ draw_x,draw_y + nHalfObjSize },
							{ draw_x + nHalfObjSize,draw_y + nHalfObjSize / 2 },
							{ draw_x + nObjSize,draw_y + nHalfObjSize }
						};

						// �����·��������
						polyline(pCrossHLine, 3);
					}
					break;

					}

					// �������ģʽ
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

			// ����
			if (bShowXY && bShowRuler)
			{
				// ��������
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

// ���ƹ�����
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

