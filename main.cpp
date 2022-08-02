///////////////////////////////////////////////
//
//	MCRedstoneSimulator
//	main.cpp
//	�ҵ������ʯģ����
//	by huidong <mailkey@yeah.net>
//
//	����޸ģ�2022.3.26
//

#include "resource.h"
#include "HiEasyX.h"
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <direct.h>

#define MAX_ZOOM 2.0
#define MIN_ZOOM 0.1

// �汾��Ϣ
const WCHAR strVersion[] = L"Version 1.4";

//
//    ___________________________________________________________________
//   / Warning													      |X|\
//   |___________________________________________________________________|
//   |                                                                   |
//   |       /\                                                          |
//   |      / !\    Warning                                              |
//   |     /____\                                                        |
//   |                �����Ŀ�и��£������ main.cpp ������ע���ϵ�ʱ�䣬   |
//   |                strVersion �ϵİ汾��./res/help/help.html �ϵ�    |
//   |                ������־���޸�ʱ�䡣                                 |
//   |___________________________________________________________________|
//
//


// ��ʯ��������
enum RedstoneObjectTypes
{
	RS_NULL,	// �������飨�޷��飩
	RS_POWDER,	// ��ʯ��
	RS_ROD,		// ����
	RS_BUTTON,	// ��ť
	RS_TORCHE,	// ��ʯ���
	RS_LIGHT,	// ��ʯ��
	RS_RELAY,	// �м���
	RS_CROSS	// �������߰壨������ MC ���飬����ƽ�潻���·�з������ã�
};

// ��ʯ���鳯��
enum RedstoneTowards
{
	RS_TO_UP,	// ������
	RS_TO_RIGHT,// ������
	RS_TO_DOWN,	// ������
	RS_TO_LEFT	// ������
};

// ��ʯ���鶨��
typedef struct RedstoneObject
{
	int nType = RS_NULL;			// ��������
	bool bPower = false;			// �Ƿ���ܣ����ڽ����ߣ����ⷽ����ܶ����ʶ��
	int nTowards = RS_TO_UP;		// ����

	bool bUprightPower = false;		// ��ֱ�����Ƿ���ܣ������ڽ����ߣ�
	bool bHorizonPower = false;		// ˮƽ�����Ƿ���ܣ������ڽ����ߣ�

	int nPowerCount = 0;			// ��������
	POINT* pPowerList = NULL;		// �����Դ����

}RsObj;

// ��ʯ��ͼ
typedef struct RedstoneMap
{
	int w, h;		// ��ͼ���
	RsObj** map;	// ��ͼ
}RsMap;

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

COLORREF colorPower = /*RGB(200, 0, 0)*/RGB(0, 240, 0);			// �е����ɫ
COLORREF colorNoPower = /*RGB(100, 0, 0)*/RGB(100, 100, 100);	// �޵����ɫ

// ��ʯ�ߴ�
int nPowderWidth = /*4*/3;

// �����С
int nObjSize;
int nHalfObjSize;

// ȫ�ְ�ťλ����Ϣ
RECT rctHelpBtn;
RECT rctSaveBtn;
RECT rctResizeBtn;

// ���ھ��
HWND hGraphicsWnd;	// ����ͼ����
HWND hToolBarWnd;	// ����������

// ��ʯ��ͼ�������
int nMapOutX = 0, nMapOutY = 20;

// ��߿��
int nRulerWidth = 20;
int nRulerHeight = 20;


// ��������

bool operator==(POINT a, POINT b)
{
	return a.x == b.x && a.y == b.y;
}

// ���ҹ�������Ƿ���ĳ����Դ
bool SearchPowerInList(POINT* pPowerList, int nCount, POINT pPower)
{
	for (int i = 0; i < nCount; i++)
	{
		if (pPowerList[i] == pPower)
		{
			return true;
		}
	}
	return false;
}

// ��Ӷ������Դ�������
// ������ӳɹ��ĸ���������ԭ���ظ�����ĸ�����
int AddToPowerList(POINT** pOldPowerList, int* p_nOldCount, POINT* pPowerList, int nCount)
{
	POINT* pNewList = new POINT[*p_nOldCount + nCount];
	int sum = 0;	// ȥ�غ������е�����
	if (!pNewList)
	{
		return 0;
	}
	if (*pOldPowerList != NULL)
	{
		memcpy(pNewList, *pOldPowerList, *p_nOldCount * sizeof(POINT));
		for (int i = 0; i < nCount; i++)
		{
			if (!SearchPowerInList(*pOldPowerList, *p_nOldCount, pPowerList[i]))
			{
				pNewList[*p_nOldCount + sum] = pPowerList[i];
				sum++;
			}
		}
		*p_nOldCount += sum;
		delete* pOldPowerList;
	}
	else
	{
		memcpy(pNewList, pPowerList, nCount * sizeof(POINT));
		*p_nOldCount = nCount;
		sum = nCount;
	}
	*pOldPowerList = pNewList;
	return sum;
}

// ��Ӷ������Դ�������
int AddToPowerList(RsObj* pObj, POINT* pPowerList, int nCount)
{
	return AddToPowerList(&pObj->pPowerList, &pObj->nPowerCount, pPowerList, nCount);
}

// �õ��շ���
RsObj GetNullObj()
{
	return RsObj{ RS_NULL ,false ,RS_TO_UP,NULL };
}

// ��ʼ����ʯ��ͼ
RsMap InitRsMap(int w, int h)
{
	RsMap map;
	map.w = w;
	map.h = h;

	map.map = new RsObj * [h];

	for (int i = 0; i < h; i++)
	{
		map.map[i] = new RsObj[w];

		for (int j = 0; j < w; j++)
		{
			map.map[i][j] = GetNullObj();
		}
	}

	return map;
}

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

// �ж�һ�����Ƿ�Ϊ�ź�Դ����
bool isPowerObj(RsObj* obj)
{
	return obj->nType == RS_ROD || obj->nType == RS_BUTTON || obj->nType == RS_TORCHE;
}

// �Ƿ�Ϊ��ͨ���飨�����Ƿ�ɵ��磬����ʯ��ѣ�����������Դ���飩
bool isNormalObj(RsObj* obj)
{
	return (obj->nType != RS_NULL && !isPowerObj(obj)) || obj->nType == RS_TORCHE;
}

// �Ƿ�Ϊ�ɵ��緽�飨ɸѡ��ʽ 1��
// ����һ��ʵ�巽�飨���˺�ʯ�ƣ�
bool isConductiveObj(RsObj* obj)
{
	return obj->nType != RS_NULL && obj->nType != RS_LIGHT;
}

// �Ƿ�Ϊ�ɵ��緽�飨ɸѡ��ʽ 2��
// ������ͨ���飨���˺�ʯ�ƣ����������ź�Դ����
bool isConductiveObj2(RsObj* obj)
{
	return isNormalObj(obj) && obj->nType != RS_LIGHT && !isPowerObj(obj);
}

// �ж���ĳ���������Ƿ��е�������
// x, y ��ǰ��������
// kx, ky ��������ֻ������һ��Ϊ 0��һ��Ϊ���� 1��
// pPowerList, p_nPowerCount ����е磬���صĹ����
// p_bPower ����ķ����Ƿ�Ϊ��Դ
//
// �� p_bPower ����Ϊ true������Ҫ���ⲿ�ͷ� pPowerList
bool isPowerTransfer(RsMap* pMap, int x, int y, int kx, int ky, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	bool r = false;					// ����Ƿ����������
	int nx = x + kx, ny = y + ky;	// ��������
	if (!(ny >= 0 && ny < pMap->h && nx >= 0 && nx < pMap->w))
	{
		return false;
	}

	RsObj obj = pMap->map[ny][nx];	// ��ǰ����

	// ���緽������ 1 �ർ�壬����Ҫ�й���Դ���߱����е�
	if (isConductiveObj(&obj) && (obj.nPowerCount || obj.bPower))
	{
		switch (obj.nType)
		{
			// �����м�������Ҫ���з����ж�
		case RS_RELAY:
			if (ky > 0 && obj.nTowards == RS_TO_UP)				r = true;
			else if (ky < 0 && obj.nTowards == RS_TO_DOWN)		r = true;
			else if (kx > 0 && obj.nTowards == RS_TO_LEFT)		r = true;
			else if (kx < 0 && obj.nTowards == RS_TO_RIGHT)		r = true;
			break;

			// CROSS ��Ҫ�����������
		case RS_CROSS:
			return isPowerTransfer(pMap, nx, ny, kx, ky, pPowerList, p_nPowerCount, p_bPower);
			break;

			// ���෽�����ֱ�ӱ��
		default:	r = true;	break;
		}
	}

	// �е�
	if (r)
	{
		// ��Դ���磬�򹩵����ֻ���ش˵�Դ
		if (obj.bPower)
		{
			POINT* p = new POINT;
			*p = { nx,ny };
			*pPowerList = p;
			*p_nPowerCount = 1;
			*p_bPower = true;
		}
		// ���幩�磬�򷵻������������
		else
		{
			*pPowerList = obj.pPowerList;
			*p_nPowerCount = obj.nPowerCount;
			*p_bPower = false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

// �����Ƿ�����
bool isUpPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, 0, -1, pPowerList, p_nPowerCount, p_bPower);
}

// �����Ƿ�����
bool isDownPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, 0, 1, pPowerList, p_nPowerCount, p_bPower);
}

// ����Ƿ�����
bool isLeftPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, -1, 0, pPowerList, p_nPowerCount, p_bPower);
}

// �ұ��Ƿ�����
bool isRightPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, 1, 0, pPowerList, p_nPowerCount, p_bPower);
}

// �ڽ��ܸ�������ʱ�����µĹ���Դ
bool JoinPowerList_AcceptPower(RsObj* pObj, POINT* pPowerList, int nCount, bool bPower)
{
	int r = AddToPowerList(pObj, pPowerList, nCount);
	if (bPower)
	{
		delete pPowerList;
	}
	return r;
}

// ���ܸ��������������Ƿ���ܵ�����
bool AcceptPowerNearby(RsMap* map, int x, int y)
{
	POINT* pPowerList = NULL;
	int nPowerCount = 0;
	bool bPower = false;

	RsObj* pObj = &map->map[y][x];	// ��ǰ����

	// �����ĸ�������ź�
	bool (*funcs[])(RsMap*, int, int, POINT**, int*, bool*) = { isLeftPower, isRightPower, isUpPower, isDownPower };
	bool result = false;
	for (int i = 0; i < 4; i++)
	{
		if (funcs[i](map, x, y, &pPowerList, &nPowerCount, &bPower))
		{
			// ����Ƿ�ɹ�����
			bool bSupply = true;
			switch (pObj->nType)
			{
				// ��ʯ���ֻ�����м�������
			case RS_TORCHE:
				switch (i)
				{
				case 0: if (map->map[y][x - 1].nType != RS_RELAY) bSupply = false; break;
				case 1: if (map->map[y][x + 1].nType != RS_RELAY) bSupply = false; break;
				case 2: if (map->map[y - 1][x].nType != RS_RELAY) bSupply = false; break;
				case 3: if (map->map[y + 1][x].nType != RS_RELAY) bSupply = false; break;
				}
				break;
				// �м���ֻ����ͬ����
			case RS_RELAY:
				switch (i)
				{
				case 0: if (pObj->nTowards != RS_TO_RIGHT)	bSupply = false; break;
				case 1: if (pObj->nTowards != RS_TO_LEFT)	bSupply = false; break;
				case 2: if (pObj->nTowards != RS_TO_DOWN)	bSupply = false; break;
				case 3: if (pObj->nTowards != RS_TO_UP)		bSupply = false; break;
				}
				break;
			}
			// ���ܳɹ��ż��빩�ܱ�
			if (bSupply)
			{
				if (JoinPowerList_AcceptPower(pObj, pPowerList, nPowerCount, bPower))
				{
					result = true;
				}
			}
			// �����ܲ��ɹ���ҲҪ�����ڴ�
			else if (bPower)
			{
				delete pPowerList;
			}
		}
	}
	return result;
}

void ConductPower(RsMap* pMap, int x, int y, POINT pPower);

// ִ�й�����ͨ����Ĵ�������ʯ��ѣ�
// pPower �����Դλ��
void RunObj(RsMap* pMap, int x, int y, POINT pPower)
{
	RsObj* pObj = &pMap->map[y][x];

	//// ��·û�бպϣ����������
	//if (!SearchPowerInList(pObj->pPowerList, pObj->nPowerCount, pPower))
	//{

	// ���ܵ���������
	if (AcceptPowerNearby(pMap, x, y))
	{
		// ����ǿ��Լ������������壬�����
		if (isConductiveObj2(pObj))
		{
			// ��������
			ConductPower(pMap, x, y, pPower);
		}
	}

	//}
}

// �����ܵ���
// pPower ԭ��Դλ��
void ConductPower(RsMap* pMap, int x, int y, POINT pPower)
{
	// ֻ����ͨ����ͺ�ʯ��ѽ��г�������
	if (x - 1 >= 0 && isNormalObj(&pMap->map[y][x - 1]))
	{
		RunObj(pMap, x - 1, y, pPower);
	}
	if (x + 1 < pMap->w && isNormalObj(&pMap->map[y][x + 1]))
	{
		RunObj(pMap, x + 1, y, pPower);
	}
	if (y - 1 >= 0 && isNormalObj(&pMap->map[y - 1][x]))
	{
		RunObj(pMap, x, y - 1, pPower);
	}
	if (y + 1 < pMap->h && isNormalObj(&pMap->map[y + 1][x]))
	{
		RunObj(pMap, x, y + 1, pPower);
	}
}

// ȷ�ϵ�Դ״̬
// pPower		��Դλ��
// flagFirst	�Ƿ�Ϊ��һ�ε���
void CheckPower(RsMap* pMap, POINT pPower, bool flagFirst = false)
{
	static POINT* pVisited = NULL;
	static int nCount = 0;

	RsObj* pObj = &pMap->map[pPower.y][pPower.x];

	// ֻ�л����Ҫȷ��״̬
	if (pObj->nType == RS_TORCHE)
	{
		// ��¼�㼣
		AddToPowerList(&pVisited, &nCount, &pPower, 1);

		// �ݹ�ȷ�����е�Դ��״̬
		for (int i = 0; i < pObj->nPowerCount; i++)
		{
			// �ظ���ټ���
			if (!SearchPowerInList(pVisited, nCount, pObj->pPowerList[i]))
			{
				CheckPower(pMap, pObj->pPowerList[i]);
			}
		}

		// ������ͨ���Դ����Ϩ��
		for (int i = 0; i < pObj->nPowerCount; i++)
		{
			POINT p = pObj->pPowerList[i];
			if (pMap->map[p.y][p.x].bPower)
			{
				pObj->bPower = false;
			}
		}
	}

	// λ�ڵݹ�ͷ�������ڴ�
	if (flagFirst && pVisited != NULL)
	{
		delete pVisited;
		pVisited = NULL;
		nCount = 0;
	}
}

// ���к�ʯ��ͼ
void RunRsMap(RsMap* pMap)
{
	// ���ó�ʼ����״̬
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			// ������Դ����״̬����ʯ�����Ϊ�е磬�����޵�
			if (isPowerObj(&pMap->map[j][i]))
			{
				if (pMap->map[j][i].nType == RS_TORCHE)
				{
					pMap->map[j][i].bPower = true;
				}
			}
			else
			{
				pMap->map[j][i].bPower = false;
			}

			// ������й����
			if (pMap->map[j][i].pPowerList != NULL)
			{
				delete pMap->map[j][i].pPowerList;
				pMap->map[j][i].pPowerList = NULL;
				pMap->map[j][i].nPowerCount = 0;
			}
		}
	}

	// ��Դ����
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (isPowerObj(&pMap->map[j][i]) && pMap->map[j][i].bPower)
			{
				// ����
				ConductPower(pMap, i, j, { i,j });
			}
		}
	}

	// ����Դ״̬����ʯ��ѿ������ã�
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (isPowerObj(&pMap->map[j][i]))
			{
				CheckPower(pMap, { i, j }, true);
			}
		}
	}

	// ������ͼ�����õ���ͨ��״̬
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			// ֻ����ͨ�����������
			if (!isPowerObj(&pMap->map[j][i]) && pMap->map[j][i].nType != RS_NULL)
			{
				// �������ܱ�����
				for (int k = 0; k < pMap->map[j][i].nPowerCount; k++)
				{
					POINT p = pMap->map[j][i].pPowerList[k];
					if (pMap->map[p.y][p.x].bPower)
					{
						pMap->map[j][i].bPower = true;
					}
				}
			}
		}
	}

	// ���е���ĳ���״̬������Ϻ��ٴζԽ����ߵ������ó���״̬
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (pMap->map[j][i].nType == RS_CROSS)
			{
				// Ԥ��Ϊ false
				pMap->map[j][i].bUprightPower = false;
				pMap->map[j][i].bHorizonPower = false;

				// �ж�һ�����Ƿ�Ϊ��Ч��Դ
				auto isActivePower = [pMap](int x, int y) {
					if (pMap->map[y][x].bPower)
						if (isConductiveObj(&pMap->map[y][x]) && pMap->map[y][x].nType != RS_CROSS)
							return true;
					return false;
				};

				// ���ڽ�������ˮƽ�ʹ�ֱ���򻥲����ţ�������ĳ��������ܣ��ش˷�������һ�����������ܷ���

				// ��ֱ��������
				// ���ѱ�Ǵ�ֱ������ܣ�������
				for (int k = -1; !pMap->map[j][i].bUprightPower && k <= 1; k += 2)
					for (int y = j + k;
						((y >= 0 && y < pMap->h)					// ����Խ��
							&& isConductiveObj(&pMap->map[y][i])	// �����ص��緽������
							&& pMap->map[y][i].bPower				// ����·��������ȫ����
							&& !pMap->map[j][i].bUprightPower);		// �ѱ�ǳ��ܺ�����
						y += k)
						if (isActivePower(i, y))
							pMap->map[j][i].bUprightPower = true;

				// ˮƽ��������
				// ���ѱ��ˮƽ������ܣ�������
				for (int k = -1; !pMap->map[j][i].bHorizonPower && k <= 1; k += 2)
					for (int x = i + k;
						((x >= 0 && x < pMap->w)					// ����Խ��
							&& isConductiveObj(&pMap->map[j][x])	// �����ص��緽������
							&& pMap->map[j][x].bPower				// ����·��������ȫ����
							&& !pMap->map[j][i].bHorizonPower);		// �ѱ�ǳ��ܺ�����
						x += k)
						if (isActivePower(x, j))
							pMap->map[j][i].bHorizonPower = true;
			}
		}
	}

}

// ������Ʒ����ͼ
void PutObjectToRsMap(RsMap* map, int x, int y, int object_id, int direction = RS_TO_UP)
{
	RsObj obj = GetNullObj();
	obj.nType = object_id;

	switch (object_id)
	{
	case RS_TORCHE:
		obj.bPower = true;
		break;
	case RS_RELAY:
		obj.nTowards = direction;
		break;
	}

	map->map[y][x] = obj;
}

// ѡ���ļ�
// isSave��־���Ƿ�Ϊ����ģʽ
const WCHAR* SelectFile(bool isSave = false)
{
	OPENFILENAME ofn;
	static WCHAR szFile[256];
	static WCHAR szFileTitle[256];
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetConsoleWindow();
	ofn.lpstrFilter = L"Redstone map File(*.rsp)\0*.rsp\0All File(*.*)\0*.*\0";
	ofn.lpstrDefExt = L"rsp";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = szFileTitle;
	ofn.nMaxFileTitle = sizeof(szFileTitle);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

	char oldpath[1024] = { 0 };
	_getcwd(oldpath, 1024);
	wchar_t woldpath[128] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, oldpath, -1, woldpath, 1024);

	if (isSave)
	{
		if (GetSaveFileName(&ofn))
		{
			SetCurrentDirectory(woldpath);
			return szFile;
		}
	}
	else
	{
		if (GetOpenFileName(&ofn))
		{
			SetCurrentDirectory(woldpath);
			return szFile;
		}
	}

	return _T("");
}

// �ж��ַ��Ƿ�Ϊ����
bool isNum(char ch)
{
	return (ch >= '0' && ch <= '9') || ch == '-';
}

// �ж�һ�ַ����Ƿ���ȫΪ����
bool isAllNum(const char* str)
{
	for (int i = 0; i < (int)strlen(str); i++)
		if (!isNum(str[i]))
			return false;
	return true;
}

// �ڵ�ǰ�������¶�ȡһ������
bool ReadNum(const char* str, int& index, int& num)
{
	char* chNum = new char[strlen(str) + 1];
	memset(chNum, 0, strlen(str) + 1);
	int num_index = 0;

	for (; index < (int)strlen(str); index++, num_index++)
	{
		if (isNum(str[index]))
		{
			chNum[num_index] = str[index];
		}
		else
		{
			break;
		}
	}

	if (num_index > 0)
	{
		num = atoi(chNum);

		delete[] chNum;
		return true;
	}

	delete[] chNum;
	return false;
}

// ɾ����ͼ
void DeleteRsMap(RsMap* map)
{
	if (map->map != NULL)
	{
		for (int i = 0; i < map->h; i++)
		{
			delete[] map->map[i];
		}

		delete[] map->map;
		map->map = NULL;
	}
}

// �����ͼ��С
void ResizeRsMap(RsMap* map, int w, int h)
{
	RsMap newmap = InitRsMap(w, h);

	int old_w = map->w;
	int old_h = map->h;

	if (old_w > w)
	{
		old_w = w;
	}
	if (old_h > h)
	{
		old_h = h;
	}

	for (int i = 0; i < old_w; i++)
	{
		for (int j = 0; j < old_h; j++)
		{
			newmap.map[j][i] = map->map[j][i];
		}
	}

	DeleteRsMap(map);
	*map = newmap;
}

// ������Ŀ
bool SaveProject(RsMap map, const WCHAR* strFileName)
{
	if (!lstrlen(strFileName))
	{
		return false;
	}

	FILE* fp;
	int nSize = WideCharToMultiByte(CP_ACP, 0, strFileName, -1, NULL, 0, NULL, NULL);
	char* chFileName = new char[nSize];
	memset(chFileName, 0, nSize);
	WideCharToMultiByte(CP_ACP, 0, strFileName, -1, chFileName, nSize, NULL, NULL);

	if (fopen_s(&fp, chFileName, "wt+") != 0)
	{
		return false;
	}

	char chW[12] = { 0 };
	char chH[12] = { 0 };
	_itoa_s(map.w, chW, 10);
	_itoa_s(map.h, chH, 10);

	fputs(chW, fp);
	fputs(" ", fp);
	fputs(chH, fp);
	fputs("\n", fp);

	for (int i = 0; i < map.h; i++)
	{
		for (int j = 0; j < map.w; j++)
		{
			char chObj[6] = { 0 };
			char chTowards[6] = { 0 };
			char chPower = '0';
			_itoa_s(map.map[i][j].nType, chObj, 10);
			_itoa_s(map.map[i][j].nTowards, chTowards, 10);
			if (map.map[i][j].bPower && map.map[i][j].nType != RS_BUTTON)
				chPower = '1';

			fputs(chObj, fp);
			fputc(',', fp);
			fputs(chTowards, fp);
			fputc(',', fp);
			fputc(chPower, fp);

			if (j + 1 != map.w)
			{
				fputc(' ', fp);
			}
		}

		fputc('\n', fp);
	}

	fclose(fp);
	return true;
}

// ����Ŀ
RsMap OpenProject(const WCHAR* strFileName)
{
	if (!lstrlen(strFileName))
	{
		return RsMap{};
	}

	FILE* fp;
	int nSize = WideCharToMultiByte(CP_ACP, 0, strFileName, -1, NULL, 0, NULL, NULL);
	char* chFileName = new char[nSize];
	memset(chFileName, 0, nSize);
	WideCharToMultiByte(CP_ACP, 0, strFileName, -1, chFileName, nSize, NULL, NULL);

	fopen_s(&fp, chFileName, "r+");
	char* chProject = new char[_filelength(_fileno(fp)) + 1];
	memset(chProject, 0, _filelength(_fileno(fp)) + 1);
	fread_s(chProject, _filelength(_fileno(fp)) + 1, _filelength(_fileno(fp)), 1, fp);

	fclose(fp);

	// �����ļ�

	// �ļ���ʽ��
	// ��ͼ�� ��ͼ��
	// ����id,����,���� ����id,����,���� ......

	// ��ȡ��ͼ���
	int nMap_w, nMap_h;
	int index = 0;

	if (!ReadNum(chProject, index, nMap_w))
		return RsMap{};
	index++;
	if (!ReadNum(chProject, index, nMap_h))
		return RsMap{};
	index++;

	RsMap map = InitRsMap(nMap_w, nMap_h);

	for (int i = 0; i < nMap_h; i++)
	{
		for (int j = 0; j < nMap_w; j++)
		{
			int nType = RS_NULL;
			int nTowards = RS_TO_UP;
			int nPower = false;

			// ����Ʒid
			if (!ReadNum(chProject, index, nType))
				return RsMap{};
			index++;

			// ������
			if (!ReadNum(chProject, index, nTowards))
				return RsMap{};
			index++;

			// ������
			if (!ReadNum(chProject, index, nPower))
				return RsMap{};
			index++;

			RsObj rsobj = GetNullObj();
			rsobj.nType = nType;
			rsobj.nTowards = nTowards;
			rsobj.bPower = (bool)nPower;

			map.map[i][j] = rsobj;
		}
	}

	return map;
}

// ������Ŀ
// out ����Ŀ
// in ��������Ŀ
// l, l ��������Ŀλ������Ŀ�����Ͻ�����
void ImportProject(RsMap* out, RsMap in, int x, int y)
{
	for (int jo = y, ji = 0; jo < out->h && ji < in.h; jo++, ji++)
	{
		for (int io = x, ii = 0; io < out->w && ii < in.w; io++, ii++)
		{
			if (jo >= 0 && io >= 0)
			{
				out->map[jo][io] = in.map[ji][ii];
			}
		}
	}
}

// ͼƬ����
// width, height ������ͼƬ��С
// img ԭͼ��
void ImageToSize(int width, int height, IMAGE* img)
{
	IMAGE* pOldImage = GetWorkingImage();
	SetWorkingImage(img);

	IMAGE temp_image(width, height);

	StretchBlt(
		GetImageHDC(&temp_image), 0, 0, width, height,
		GetImageHDC(img), 0, 0,
		getwidth(), getheight(),
		SRCCOPY
	);

	Resize(img, width, height);
	putimage(0, 0, &temp_image);

	SetWorkingImage(pOldImage);
}

/*
 *    �ο���http://tieba.baidu.com/pPower/5218523817
 *    ������:zoomImage(IMAGE* pImg,int width��int height)
 *    ����˵��:pImg��ԭͼָ�룬width1��height1��Ŀ��ͼƬ�ĳߴ硣
 *    ��������:��ͼƬ�������ţ�����Ŀ��ͼƬ �����Զ��峤���Ҳ����ֻ�����Զ������
 *    ����Ŀ��ͼƬ
*/
IMAGE zoomImage(IMAGE* pImg, int newWidth, int newHeight = 0)
{
	// ��ֹԽ��
	if (newWidth < 0 || newHeight < 0) {
		newWidth = pImg->getwidth();
		newHeight = pImg->getheight();
	}

	// ������ֻ��һ��ʱ�������Զ�����
	if (newHeight == 0) {
		// �˴���Ҫע����*��/����Ȼ��Ŀ��ͼƬС��ԭͼʱ�����
		newHeight = newWidth * pImg->getheight() / pImg->getwidth();
	}

	// ��ȡ��Ҫ�������ŵ�ͼƬ
	IMAGE newImg(newWidth, newHeight);

	// �ֱ��ԭͼ���Ŀ��ͼ���ȡָ��
	DWORD* oldDr = GetImageBuffer(pImg);
	DWORD* newDr = GetImageBuffer(&newImg);

	// ��ֵ ʹ��˫���Բ�ֵ�㷨
	for (int i = 0; i < newHeight - 1; i++) {
		for (int j = 0; j < newWidth - 1; j++) {
			int t = i * newWidth + j;
			int xt = j * pImg->getwidth() / newWidth;
			int yt = i * pImg->getheight() / newHeight;
			newDr[i * newWidth + j] = oldDr[xt + yt * pImg->getwidth()];
			// ʵ�����м���ͼƬ
			/*byte return_value = (GetRValue(oldDr[xt + yt * pImg->getwidth()]) +
				GetRValue(oldDr[xt + yt * pImg->getwidth() + 1]) +
				GetRValue(oldDr[xt + (yt + 1) * pImg->getwidth()]) +
				GetRValue(oldDr[xt + (yt + 1) * pImg->getwidth() + 1])) / 4;
			byte g = (GetGValue(oldDr[xt + yt * pImg->getwidth()]) +
				GetGValue(oldDr[xt + yt * pImg->getwidth()] + 1) +
				GetGValue(oldDr[xt + (yt + 1) * pImg->getwidth()]) +
				GetGValue(oldDr[xt + (yt + 1) * pImg->getwidth()]) + 1) / 4;
			byte b = (GetBValue(oldDr[xt + yt * pImg->getwidth()]) +
				GetBValue(oldDr[xt + yt * pImg->getwidth()] + 1) +
				GetBValue(oldDr[xt + (yt + 1) * pImg->getwidth()]) +
				GetBValue(oldDr[xt + (yt + 1) * pImg->getwidth() + 1])) / 4;*/
			byte r = GetRValue(oldDr[xt + yt * pImg->getwidth()]);
			byte g = GetGValue(oldDr[xt + yt * pImg->getwidth()]);
			byte b = GetBValue(oldDr[xt + yt * pImg->getwidth()]);
			newDr[i * newWidth + j] = RGB(r, g, b);
		}
	}

	return newImg;
}

// �õ���ͼ����
// map						��ͼ
// redraw					��ʶ�Ƿ����»��Ƶ�ͼ
// resize					��ʶ��ͼ�ߴ��Ƿ����
// bShowXY					ָ���Ƿ���ʾ����
// bShowRuler				ָ����ʾ����ʱ�ķ�ʽ��Ϊ true ��ʾ��ʾ��ߣ�Ϊ false ��ʾֱ���ڷ�������ʾ����
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

	if (redraw)
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
		if (redraw)
		{
			SetWorkingImage(&imgMap);
			settextcolor(WHITE);
			settextstyle(10, 0, L"����");
			setbkmode(TRANSPARENT);

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

						if (me.bPower)
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

						// ʵʱ���ƺ�ʯ��
						if (y - 1 >= 0 && up.nType != RS_NULL)			// line to up
						{
							line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x + nHalfObjSize, draw_y);
							bConnect = true;
						}
						if (y + 1 < map->h && down.nType != RS_NULL)	// line to down
						{
							line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x + nHalfObjSize, draw_y + nObjSize);
							bConnect = true;
						}
						if (x - 1 >= 0 && left.nType != RS_NULL)		// line to left
						{
							line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x, draw_y + nHalfObjSize);
							bConnect = true;
						}
						if (x + 1 < map->w && right.nType != RS_NULL)	// line to right
						{
							line(draw_x + nHalfObjSize, draw_y + nHalfObjSize, draw_x + nObjSize, draw_y + nHalfObjSize);
							bConnect = true;
						}

						if (!bConnect)
						{
							fillcircle(draw_x + nHalfObjSize, draw_y + nHalfObjSize, nPowderWidth);
						}

						//putimage(l * nObjSize, l * nObjSize, &powder);
					}
					break;

					case RS_ROD:		putimage(draw_x, draw_y, &imgRod[me.bPower]);		break;
					case RS_BUTTON:		putimage(draw_x, draw_y, &imgButton[me.bPower]);	break;
					case RS_TORCHE:		putimage(draw_x, draw_y, &imgTorche[me.bPower]);	break;
					case RS_LIGHT:		putimage(draw_x, draw_y, &imgLight[me.bPower]);		break;

					case RS_RELAY:
					{
						IMAGE* p = NULL;
						switch (me.nTowards)
						{
						case RS_TO_UP:		p = &imgRelay[me.bPower];				break;
						case RS_TO_LEFT:	p = &imgRelayRotated[me.bPower][0];		break;
						case RS_TO_DOWN:	p = &imgRelayRotated[me.bPower][1];		break;
						case RS_TO_RIGHT:	p = &imgRelayRotated[me.bPower][2];		break;
						}
						putimage(draw_x, draw_y, p);
					}
					break;

					case RS_CROSS:
					{
						setlinestyle(PS_SOLID, nPowderWidth);

						if (me.bUprightPower)
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

						if (me.bHorizonPower)
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

// �������з������
// cmd ԭ����
// chCmdsArray_out ��������б�
// nArgsNum_out �����������
void GetArguments(const char* cmd, char*** chCmdsArray_out, int* nArgsNum_out)
{
	// ��������
	int nArgsNum = 1;
	for (int i = 0; i < (int)strlen(cmd); i++)
		if (cmd[i] == ' ')
			nArgsNum++;

	int nMaxCmdSize = (int)strlen(cmd) + 1;
	char** chCmdsArray = new char* [nArgsNum];
	for (int i = 0; i < nArgsNum; i++)
	{
		chCmdsArray[i] = new char[nMaxCmdSize];
		memset(chCmdsArray[i], 0, nMaxCmdSize);
	}

	// ����ÿ������
	for (int nIndex = 0, nMainIndex = 0, nArgNum = 0; nIndex < (int)strlen(cmd); nIndex++, nMainIndex++)
	{
		if (cmd[nMainIndex] == ' ')
		{
			nIndex = -1;
			nArgNum++;
			continue;
		}

		chCmdsArray[nArgNum][nIndex] = cmd[nMainIndex];
	}

	*chCmdsArray_out = chCmdsArray;
	*nArgsNum_out = nArgsNum;
}

// �����˵�
void HelpMenu()
{
	system("start ./res/help/help.html");
	printf("�Ѵ򿪰����ĵ������Ժ�ҳ��򿪡�\n");
}

// ���ַ���ת���� ID ��
// str ��ʾԭ�ַ���
// type_out ��ʾ ID ���ͣ�Ϊ 0 ��ʾ��Ʒ��Ϊ 1 ��ʾ����
// id_out ��ʾ ID
// ����������ַ����Ƿ�Ϸ�
bool GetIdFromString(const char* str, int* type_out, int* id_out)
{
	const int OBJECT = 0;
	const int DIRECTION = 1;

	// object
	if (strcmp(str, "null") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_NULL;
	}
	else if (strcmp(str, "powder") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_POWDER;
	}
	else if (strcmp(str, "rod") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_ROD;
	}
	else if (strcmp(str, "button") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_BUTTON;
	}
	else if (strcmp(str, "torche") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_TORCHE;
	}
	else if (strcmp(str, "light") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_LIGHT;
	}
	else if (strcmp(str, "relay") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_RELAY;
	}

	else if (strcmp(str, "cross") == 0)
	{
		*type_out = OBJECT;
		*id_out = RS_CROSS;
	}

	// direction
	else if (strcmp(str, "up") == 0)
	{
		*type_out = DIRECTION;
		*id_out = RS_TO_UP;
	}
	else if (strcmp(str, "down") == 0)
	{
		*type_out = DIRECTION;
		*id_out = RS_TO_DOWN;
	}
	else if (strcmp(str, "left") == 0)
	{
		*type_out = DIRECTION;
		*id_out = RS_TO_LEFT;
	}
	else if (strcmp(str, "right") == 0)
	{
		*type_out = DIRECTION;
		*id_out = RS_TO_RIGHT;
	}

	// ���������
	else
	{
		return false;
	}

	return true;
}

// ��������Ϸ��ԣ��Ƿ񳬳���ͼ��
bool PointIsInMap(RsMap* map, int x, int y)
{
	return x >= 0 && x < map->w&& y >= 0 && y < map->h;
}

// ����ͼ��
void Render(
	RsMap* map,
	bool redraw,
	bool resize,
	bool zoom_changed,
	int offset_x,
	int offset_y,
	double zoom,
	bool bShowXY,
	bool bShowRuler
)
{
	static IMAGE imgMap, imgRulerX, imgRulerY;
	static IMAGE imgMap_zoomed, imgRulerX_zoomed, imgRulerY_zoomed;

	bool draw = redraw || resize;
	if (draw)
		GetRsMapImage(&imgMap, &imgRulerX, &imgRulerY, map, redraw, resize, bShowXY, bShowRuler);
	if (draw || zoom_changed)
	{
		// ����ͼ��
		imgMap_zoomed = imgMap;
		imgRulerX_zoomed = imgRulerX;
		imgRulerY_zoomed = imgRulerY;
		IMAGE* p[3] = { &imgMap_zoomed,&imgRulerX_zoomed,&imgRulerY_zoomed };
		for (int i = 0; i < 3; i++)
			*p[i] = zoomImage(
				p[i],
				(int)(p[i]->getwidth() * zoom),
				(int)(p[i]->getheight() * zoom)
			);
	}

	// ���
	BEGIN_TASK_WND(hGraphicsWnd);
	{
		cleardevice();

		// ���ͼ��
		// ���ڱ��ʱ����Ҫ��ͼ��ƫ�Ƽ��ϱ�ߵĴ�С
		// �ɴ��κ�ʱ�򶼼��ϰɣ�����������������ʱ����Ҫ�����
		int render_offset_x = nMapOutX + offset_x + (int)(nRulerWidth * zoom);
		int render_offset_y = nMapOutY + offset_y + (int)(nRulerHeight * zoom);
		putimage(render_offset_x, render_offset_y, &imgMap_zoomed);

		// ���ƻ��ƺõı��
		if (bShowXY && bShowRuler)
		{
			putimage(nMapOutX + offset_x + (int)(nRulerWidth * zoom), nMapOutY, &imgRulerX_zoomed);
			putimage(nMapOutX, nMapOutY + offset_y + (int)(nRulerHeight * zoom), &imgRulerY_zoomed);
		}

		// ���ƶ���
		int w = getwidth();
		setfillcolor(RGB(50, 50, 50));
		solidrectangle(0, 0, w, nMapOutY);
		line(0, nMapOutY, w, nMapOutY);
		WCHAR strName[] = L"Minecraft Redstone Simulator";
		outtextxy((w - textwidth(strName)) / 2, 3, strName);

		// ��ť����
		setfillcolor(BLUE);
		RECT pBtns[3] = { rctSaveBtn,rctResizeBtn,rctHelpBtn };
		WCHAR pStrs[3][12] = { L"SAVE",L"RESIZE",L"HELP" };
		for (int i = 0; i < 3; i++)
		{
			fillrectangle_RECT(pBtns[i]);
			outtextxy(pBtns[i].left + 4, pBtns[i].top + 3, pStrs[i]);
		}
	}
	END_TASK();
	FLUSH_DRAW();
}

// ���һ����ť��������
void ClickButton(RsMap* map, int x, int y, int offset_x, int offset_y, double zoom, bool bShowXY, bool bShowRuler)
{
	int delay = 1000;

	if (map->map[y][x].nType == RS_BUTTON)
	{
		map->map[y][x].bPower = true;
		RunRsMap(map);

		Render(map, true, false, false, offset_x, offset_y, zoom, bShowXY, bShowRuler);	// �ֶ��ػ�

		Sleep(delay);
		map->map[y][x].bPower = false;
		RunRsMap(map);

		Render(map, true, false, false, offset_x, offset_y, zoom, bShowXY, bShowRuler);
	}
}

// �õ�������ҺϷ������꣨�����������ʹ�ã�
void GetSortingPoint(RsMap* map, int* x1, int* y1, int* x2, int* y2)
{
	// ȷ��x1 <= x2, y1 <= y2
	if (*x1 > *x2)
	{
		int temp = *x1;
		*x1 = *x2;
		*x2 = temp;
	}
	if (*y1 > *y2)
	{
		int temp = *y1;
		*y1 = *y2;
		*y2 = temp;
	}

	// ȷ������Ϸ�
	if (*x1 < 0)
		*x1 = 0;
	if (*x2 >= map->w)
		*x2 = map->w - 1;
	if (*y1 < 0)
		*y1 = 0;
	if (*y2 >= map->h)
		*y2 = map->h - 1;
}

// �ں�ʯ��ͼ�л��ƺ�ʯֱ�ߣ���������������ָ����������ʯֱ�ߵ���Ʒ
// �����Ƿ���Ƴɹ������Ʋ��ɹ���ԭ����ֱ��������
bool LineRsMap(RsMap* map, int x1, int y1, int x2, int y2, int object)
{
	// ��������
	GetSortingPoint(map, &x1, &y1, &x2, &y2);

	if (x1 == x2)
	{
		for (int i = y1; i <= y2; i++)
		{
			PutObjectToRsMap(map, x1, i, object);
		}
	}
	else if (y1 == y2)
	{
		for (int i = x1; i <= x2; i++)
		{
			PutObjectToRsMap(map, i, y1, object);
		}
	}
	else
	{
		return false;
	}

	return true;
}

// �����ʯ��ͼ�е�һ������
void ClearRsMap(RsMap* map, int x1, int y1, int x2, int y2)
{
	// ��������
	GetSortingPoint(map, &x1, &y1, &x2, &y2);

	for (int i = y1; i <= y2; i++)
	{
		for (int j = x1; j <= x2; j++)
		{
			PutObjectToRsMap(map, j, i, RS_NULL);
		}
	}
}

// �����û��������루������
void ProcessCommand(RsMap* map, int* offset_x, int* offset_y, double* zoom, bool* p_bShowXY, bool* p_bShowRuler)
{
	// ��ͼ�ƶ��ĵ�λ��С�����أ�
	const int offset_unit_size = 10;

	// ���������
	int nMaxInputdSize = 1024;
	char* chCmd = new char[nMaxInputdSize];
	memset(chCmd, 0, nMaxInputdSize);

	gets_s(chCmd, nMaxInputdSize);

	// ��������
	static int nArgsNum = 0;
	static char** chCmdsArray = NULL;

	// ������һ�ε��ڴ�
	if (chCmdsArray != NULL)
	{
		for (int i = 0; i < nArgsNum; i++)
			delete[] chCmdsArray[i];
		delete[] chCmdsArray;
	}

	// ��������
	GetArguments(chCmd, &chCmdsArray, &nArgsNum);

	delete[] chCmd;

	// �����Ƿ�����ʾ����
	if (nArgsNum == 1 && strcmp(chCmdsArray[0], "xy") == 0)
	{
		*p_bShowXY = !(*p_bShowXY);
	}

	// �л�������ʾģʽ
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "xy_mode") == 0)
	{
		*p_bShowRuler = !(*p_bShowRuler);
	}

	// ����
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "help") == 0)
	{
		HelpMenu();
		return;
	}

	// ����
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "cls") == 0)
	{
		system("cls");
		return;
	}

	// ֱ�����������ʾ��ĳλ�÷��ú�ʯ�ۣ�����Ǹ�λ��������Ʒ��������Ǹ���Ʒ
	else if (nArgsNum == 2 && isAllNum(chCmdsArray[0]) && isAllNum(chCmdsArray[1]))
	{
		int x = atoi(chCmdsArray[0]);
		int y = atoi(chCmdsArray[1]);

		if (!PointIsInMap(map, x, y))
		{
			printf("����Խ�硣\n");
			return;
		}

		if (map->map[y][x].nType != RS_NULL)
		{
			PutObjectToRsMap(map, x, y, RS_NULL);
		}
		else
		{
			PutObjectToRsMap(map, x, y, RS_POWDER);
		}
	}

	// ���� ���� ��Ʒ ������
	// ���������
	// ��ĳλ�÷���ĳ��Ʒ������Ǹ�λ��������Ʒ���򸲸��Ǹ���Ʒ����ʯ�м����г����趨������������ʾ����
	// ����ĳλ�õ���Ʒ�ĳ�����Ժ�ʯ�м�����
	else if ((nArgsNum == 3 || nArgsNum == 4) && isAllNum(chCmdsArray[0]) && isAllNum(chCmdsArray[1]) && !isAllNum(chCmdsArray[2]))
	{
		int type;
		int id;
		if (!GetIdFromString(chCmdsArray[2], &type, &id))
		{
			printf("�������ƴ���\n");
			return;
		}

		int x = atoi(chCmdsArray[0]);
		int y = atoi(chCmdsArray[1]);

		if (!PointIsInMap(map, x, y))
		{
			printf("����Խ�硣\n");
			return;
		}

		if (type == 0)
		{
			PutObjectToRsMap(map, x, y, id);

			// �����ĸ�������˵������һ������
			if (nArgsNum == 4)
			{
				int type2;
				int direction;
				if (!GetIdFromString(chCmdsArray[3], &type2, &direction))
				{
					printf("����ִ�в���ȫ�����﷨�������ֹ�ˡ�����ԭ�򣺵��ĸ�����δ��ָ�����鳯��\n");
					return;
				}

				if (type2 == 1)
				{
					map->map[y][x].nTowards = direction;
				}
				else
				{
					printf("����ִ�в���ȫ�����﷨�������ֹ�ˡ�����ԭ�򣺵��ĸ�����δ��ָ�����鳯��\n");
					return;
				}
			}
		}
		else if (type == 1)
		{
			if (map->map[y][x].nType == RS_NULL)
			{
				printf("�����겢����Ʒ���顣\n");
				return;
			}
			else
			{
				map->map[y][x].nTowards = id;
			}
		}
	}

	// . ����x ����y
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], ".") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int x = atoi(chCmdsArray[1]);
		int y = atoi(chCmdsArray[2]);

		if (!PointIsInMap(map, x, y))
		{
			printf("����Խ�硣\n");
			return;
		}

		switch (map->map[y][x].nType)
		{
		case RS_NULL:
			printf("�����겢����Ʒ���顣\n");
			return;
			break;

		case RS_ROD:
			map->map[y][x].bPower = !map->map[y][x].bPower;
			break;

		case RS_BUTTON:
			ClickButton(map, x, y, *offset_x, *offset_y, *zoom, *p_bShowXY, *p_bShowRuler);
			break;

		default:
			printf("����Ʒ��֧�ֵ��������\n");
			return;
			break;
		}
	}

	// resize �� ��
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "resize") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int w = atoi(chCmdsArray[1]);
		int h = atoi(chCmdsArray[2]);

		if (w <= 0 || h <= 0)
		{
			printf("��ͼ��С����С�ڵ���0\n");
			return;
		}

		ResizeRsMap(map, w, h);
	}

	// save ������Ŀ
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "save") == 0)
	{
		if (SaveProject(*map, SelectFile(true)))
		{
			printf("����ɹ���\n");
			return;
		}
		else
		{
			printf("����ʧ�ܡ�\n");
			return;
		}
	}

	// ����ͼ�����xyƫ�ƶ��ص�0
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "reset_map_offset") == 0)
	{
		*offset_x = 0;
		*offset_y = 0;
	}

	// �ƶ���ͼ
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "up") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_y -= atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// �ƶ���ͼ
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "down") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_y += atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// �ƶ���ͼ
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "left") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_x -= atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// �ƶ���ͼ
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "right") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_x += atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// ���ŵ�ͼ
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "zoom") == 0 && isAllNum(chCmdsArray[1]))
	{
		*zoom = atoi(chCmdsArray[1]) / 100.0;
	}

	// ���ŵ�ͼ
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "zoom+") == 0 && isAllNum(chCmdsArray[1]))
	{
		*zoom += atoi(chCmdsArray[1]) / 100.0;
	}

	// ���ŵ�ͼ
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "zoom-") == 0 && isAllNum(chCmdsArray[1]))
	{
		*zoom -= atoi(chCmdsArray[1]) / 100.0;
	}

	// �õ���ͼƫ����
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "get_map_offset") == 0)
	{
		printf("x��ƫ������%d����λ����%d���أ�\ny��ƫ������%d����λ����%d���أ�\n",
			*offset_x / offset_unit_size, *offset_x, *offset_y / offset_unit_size, *offset_y);
	}

	// �õ���ͼ������
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "get_map_zoom") == 0)
	{
		printf("��������%d%%\n", (int)(*zoom * 100));
	}

	// import l l
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "import") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int x = atoi(chCmdsArray[1]);
		int y = atoi(chCmdsArray[2]);

		/*if (!PointIsInMap(map, l, l))
		{
			printf("����Խ�硣\n");
			return;
		}*/

		printf("��ѡ��Ҫ�������Ŀ��\n");

		const WCHAR* map_file = SelectFile();
		RsMap import_map = OpenProject(map_file);

		char warning[512] = { 0 };
		sprintf_s(warning, 512,
			"�����ڵ���һ����Ŀ�������ǵ�����Ŀ����Ϣ��\n"
			"��������Ŀ������Ŀ�����Ͻ�λ�ã�( %d, %d )��\n"
			"����Ŀ��ͼ��С���� %d �񣬸� %d ��\n"
			"����Ŀ����������Ŀ�����·�Χ��( %d, %d ) �� ( %d, %d )�������������Ŀ�ĵ�ͼ��С���Զ��������Ĳ��ֲü���\n"
			"\n\n"
			"�Ƿ�����������Ŀ�������ȷ������ȷ���������Ŀ����������ȡ������ֹ�˲���������㵣����ɲ�����ĺ���������ȱ��浱ǰ��Ŀ���ٽ��е��롣\n"
			, x, y, import_map.w, import_map.h, x, y, x + import_map.w, y + import_map.h);

		wchar_t wstr[512] = { 0 };
		MultiByteToWideChar(CP_ACP, 0, warning, (int)strlen(warning), wstr, (int)strlen(warning));

		if (MessageBox(GetConsoleWindow(), wstr, L"ȷ����", MB_OKCANCEL) == IDOK)
		{
			ImportProject(map, import_map, x, y);
		}

		DeleteRsMap(&import_map);
		printf("������ϡ�\n");
	}

	// line ����x1 ����y1 ����x2 ����y2 (��Ʒ) : ����ʯֱ�ߣ������������������ĩβ������Ʒ����ʾ��ĳ�������ֱ�ߡ�
	else if (
		(nArgsNum == 5 || nArgsNum == 6) && strcmp(chCmdsArray[0], "line") == 0 &&
		isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]) && isAllNum(chCmdsArray[3]) && isAllNum(chCmdsArray[4]) ||

		((nArgsNum == 4 || nArgsNum == 5) && isAllNum(chCmdsArray[0]) && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]) && isAllNum(chCmdsArray[3]))
		)
	{
		int start = 1;
		if (nArgsNum == 4 || (nArgsNum == 5 && !isAllNum(chCmdsArray[4])))
		{
			start = 0;
		}

		int x1 = atoi(chCmdsArray[start]);
		int y1 = atoi(chCmdsArray[start + 1]);
		int x2 = atoi(chCmdsArray[start + 2]);
		int y2 = atoi(chCmdsArray[start + 3]);
		int id = RS_POWDER;

		if (!PointIsInMap(map, x1, y1) || !PointIsInMap(map, x2, y2))
		{
			printf("����Խ�硣\n");
			return;
		}

		if (nArgsNum == start + 5)
		{
			int type;
			if (!GetIdFromString(chCmdsArray[start + 4], &type, &id) || type != 0)
			{
				printf("���һ������Ӧ������Ʒid��\n");
				return;
			}
		}

		if (!LineRsMap(map, x1, y1, x2, y2, id))
		{
			printf("(%d, %d) �� (%d, %d) ���������޷�����ֱ�ߡ�\n", x1, y1, x2, y2);
			return;
		}
	}

	// clear ����x1 ����y1 ����x2 ����y2 : �Կ������鸲�� (x1,y1) �� (x2,y2) �����з��顣
	else if (nArgsNum == 5 && strcmp(chCmdsArray[0], "clear") == 0 &&
		isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]) && isAllNum(chCmdsArray[3]) && isAllNum(chCmdsArray[4]))
	{
		int x1 = atoi(chCmdsArray[1]);
		int y1 = atoi(chCmdsArray[2]);
		int x2 = atoi(chCmdsArray[3]);
		int y2 = atoi(chCmdsArray[4]);

		if (!PointIsInMap(map, x1, y1) || !PointIsInMap(map, x2, y2))
		{
			printf("����Խ�硣\n");
			return;
		}

		ClearRsMap(map, x1, y1, x2, y2);
	}

	// exit �˳�����
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "exit") == 0)
	{
		if (MessageBox(GetConsoleWindow(), L"ȷ��Ҫ�˳�������", L"�˳�����", MB_OKCANCEL) == IDOK)
		{
			exit(0);
		}
	}

	// cmd_window_top ʹcmd����
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "cmd_window_top") == 0)
	{
		SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// cmd_window_no_top ʹcmd������
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "cmd_window_no_top") == 0)
	{
		SetWindowPos(GetConsoleWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// map_window_top ʹ��ͼ���ڶ���
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "map_window_top") == 0)
	{
		SetWindowPos(GetHWnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// map_window_no_top ʹ��ͼ���ڲ�����
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "map_window_no_top") == 0)
	{
		SetWindowPos(GetHWnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// resize_cmd_window ����cmd���ڴ�С
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "resize_cmd_window") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int w = atoi(chCmdsArray[1]);
		int h = atoi(chCmdsArray[2]);
		char cmd[64] = { 0 };
		sprintf_s(cmd, 64, "mode con cols=%d lines=%d", w, h);
		system(cmd);
	}

	// resize_map_window ���û�ͼ���ڴ�С
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "resize_map_window") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int w = atoi(chCmdsArray[1]);
		int h = atoi(chCmdsArray[2]);
		Resize(NULL, w, h);
	}

	else
	{
		printf("δ֪���\n");
		return;
	}

	printf("ִ����ϡ�\n");
}

// �������� CMD ��Ϣ
void CommandMessageLoop(RsMap* map, int* offset_x, int* offset_y, double* zoom, bool* p_bShowXY, bool* p_bShowRuler)
{
	while (true)
	{
		ProcessCommand(map, offset_x, offset_y, zoom, p_bShowXY, p_bShowRuler);
	}
}

// ��ʾ��������
void HelpBox()
{
	int w = 820, h = 420;
	HWND hHelpWnd = HiEasyX::initgraph_win32(w, h, EW_NORMAL, L"����", NULL, hGraphicsWnd);
	DisableResizing(true);

	RECT rctDoneBtn = { w / 2 - 25,h - 35,w / 2 + 25,h - 8 };

	BEGIN_TASK();

	settextstyle(46, 0, L"����");
	outtextxy(30, 20, L"MCRedStoneSimulator ʹ��˵��");
	settextstyle(22, 0, L"΢���ź�");
	outtextxy(30, 80, L"�������� huidong <mailhuid@163.com> �����ĺ�ʯ����·��ģ�����");
	outtextxy(30, 110, L"�������� ToolBar ��ѡ�񹤾߻�Ԫ����������ʾ�е�·ͼ�Ĵ�����ʹ���������༭��·ͼ��");
	outtextxy(30, 140, L"�ڳ���������д����������Լ���ָ��Ե�·��Ŀ���в��������롰help��ָ����Բ鿴ָ���");
	outtextxy(30, 200, L"�������������Ԫ��    Ctrl + ��������Ԫ��           �� ToolBar �У�");
	outtextxy(30, 230, L"����϶����ƶ���ͼ    Ctrl + �Ҽ�����תԪ��           ѡ�����ʱ���Ҽ��������Բ���Ԫ��");
	outtextxy(30, 260, L"�����֣����ŵ�ͼ");
	outtextxy(410, 260, L"ѡ��Ԫ��ʱ���Ҽ����¿��Է���Ԫ��");
	outtextxy(30, 310, L"���߲��ͣ�http://huidong.xyz   EasyX ��վ��https://easyx.cn");
	outtextxy(30, 340, L"Github ��Ŀ��ַ��https://github.com/zouhuidong/MinecraftRedstoneSimulator");

	setfillcolor(BLUE);
	setbkmode(TRANSPARENT);
	fillrectangle_RECT(rctDoneBtn);
	outtextxy(rctDoneBtn.left + 4, rctDoneBtn.top + 3, L"Done");
	END_TASK();
	FLUSH_DRAW();

	// ��Ϣ��Ӧ
	while (HiEasyX::isAliveWindow(hHelpWnd))
	{
		bool end = false;
		BEGIN_TASK();
		while (MouseHit())
		{
			ExMessage msg = getmessage(EM_MOUSE);
			if (msg.message == WM_LBUTTONUP)
			{
				if (isInRect(msg.x, msg.y, rctDoneBtn))
				{
					end = true;
					break;
				}
			}
		}
		END_TASK();
		if (end)
		{
			HiEasyX::closegraph_win32(hHelpWnd);
			break;
		}
		Sleep(10);
	}
}

WCHAR strMapSize[2][12];	// ��������д洢��ͼ��С����
#define IDC_EDIT1	100
#define IDC_EDIT2	101
#define IDC_BUTTON	102
bool ResizeBoxWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance)
{
	static HWND hEdit[2];
	static HWND hBtn;

	switch (msg)
	{
	case WM_CREATE:
		hEdit[0] = CreateWindow(L"edit", strMapSize[0],
			WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER,
			60, 30, 40, 20,
			hWnd, (HMENU)IDC_EDIT1, hInstance, NULL);
		hEdit[1] = CreateWindow(L"edit", strMapSize[1],
			WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER,
			60, 55, 40, 20,
			hWnd, (HMENU)IDC_EDIT2, hInstance, NULL);
		hBtn = CreateWindow(L"button", L"Done",
			WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER,
			160, 40, 60, 30,
			hWnd, (HMENU)IDC_BUTTON, hInstance, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON:
		{
			GetWindowText(hEdit[0], strMapSize[0], 12);
			GetWindowText(hEdit[1], strMapSize[1], 12);
			SendMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;
		}
		break;
	case WM_PAINT:
		BEGIN_TASK();
		setbkcolor(CLASSICGRAY);
		settextcolor(BLACK);
		cleardevice();
		outtextxy(30, 30, L"��");
		outtextxy(30, 55, L"�ߣ�");
		settextcolor(GRAY);
		settextstyle(14, 0, L"����");
		//outtextxy(30, 80, L"��ͼ��С���� 50x50 ���¿���");
		END_TASK();
		break;
	}

	return true;
}

// ��ʾ���õ�ͼ��С����
// �����Ƿ�ɹ�������һ���µĴ�С
bool ResizeBox(RsMap* pMap, HWND hParent)
{
	_itow_s(pMap->w, strMapSize[0], 10);
	_itow_s(pMap->h, strMapSize[1], 10);

	HWND hResizeWnd = HiEasyX::initgraph_win32(320, 120, EW_NORMAL, L"���õ�ͼ��С", ResizeBoxWndProc, hParent);
	DisableResizing(true);

	while (HiEasyX::isAliveWindow(hResizeWnd))
	{
		Sleep(10);
	}

	int w = _wtoi(strMapSize[0]);
	int h = _wtoi(strMapSize[1]);

	if (w * h == 0)
	{
		MessageBox(hParent, L"��Чֵ", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	else if (w != pMap->w || h != pMap->h)
	{
		ResizeRsMap(pMap, w, h);
		return true;
	}
	return false;
}

// �ػ���Ϣ
enum DrawMsg
{
	DM_NULL = 0,		// ���ػ���Ϣ
	DM_TOOLBAR = 1,		// �ػ湤����
	DM_DRAWMAP = 2,		// ���Ƶ�ͼ
	DM_REDRAWMAP = 4,	// ���»��Ƶ�ͼ
	DM_RESIZEMAP = 8,	// ��ͼ��С�ı�
	DM_ZOOM = 16,		// ����
};

// ���������Ϣ
// �����ػ���Ϣ���� DrawMsg��
int ProcessMouseMsg(RsMap* map, int* offset_x, int* offset_y, double* zoom, bool* p_bShowXY, bool* p_bShowRuler, int* pSelect)
{
	// ��ͼ������Ϣ
	ExMessage msgGraWnd;
	static ExMessage msgGraWndLast = {};
	static bool bGraWndLBtn = false;
	static bool bMoved = false;

	// ������������Ϣ
	ExMessage msgToolWnd;

	// ����ֵ
	int return_value = DM_NULL;

	// �������Ŵ�С
	double dZoom;

	// ������ڵ�ͼ�ϵ�����
	int nClickMapX, nClickMapY;

	// ��ͼ����
	if (HiEasyX::SetWorkingWindow(hGraphicsWnd))
	{
		HiEasyX::BeginTask();
		while (MouseHit())
		{
			msgGraWnd = getmessage(EM_MOUSE);

			dZoom = *zoom + (msgGraWnd.wheel / 120) * 0.1;

			nClickMapX = (int)((msgGraWnd.x - nMapOutX - nRulerWidth * *zoom - *offset_x) / (nObjSize * *zoom));
			nClickMapY = (int)((msgGraWnd.y - nMapOutY - nRulerHeight * *zoom - *offset_y) / (nObjSize * *zoom));

			// ����
			if (msgGraWnd.wheel < 0)
			{
				if (dZoom >= MIN_ZOOM)
				{
					*zoom = dZoom;
					return_value |= DM_ZOOM;
				}
			}
			else if (msgGraWnd.wheel > 0)
			{
				if (dZoom <= MAX_ZOOM)
				{
					*zoom = dZoom;
					return_value |= DM_ZOOM;
				}
			}

			// ������𣺰�ť��Ϣ����
			if (msgGraWnd.message == WM_LBUTTONUP)
			{
				// ������ť
				if (isInRect(msgGraWnd.x, msgGraWnd.y, rctHelpBtn))
				{
					// ��;�л����ڣ���������ֹ����
					HiEasyX::EndTask();

					HelpBox();

					// ������������
					if (!HiEasyX::SetWorkingWindow(hGraphicsWnd))
					{
						goto ToolBar_Begin;
					}
					HiEasyX::BeginTask();
				}
				// ����
				else if (isInRect(msgGraWnd.x, msgGraWnd.y, rctSaveBtn))
				{
					SaveProject(*map, SelectFile(true));
					flushmessage();
				}
				// �����ͼ��С
				else if (isInRect(msgGraWnd.x, msgGraWnd.y, rctResizeBtn))
				{
					HiEasyX::EndTask();

					ResizeBox(map, hGraphicsWnd);
					return_value |= DM_RESIZEMAP;

					if (!HiEasyX::SetWorkingWindow(hGraphicsWnd))
					{
						goto ToolBar_Begin;
					}
					HiEasyX::BeginTask();
				}
			}

			// �Ҽ�����
			if (msgGraWnd.rbutton)
			{
				if (PointIsInMap(map, nClickMapX, nClickMapY))
				{
					if (msgGraWnd.ctrl)	// Ctrl����תԪ��
					{
						if (++map->map[nClickMapY][nClickMapX].nTowards > 3)
						{
							map->map[nClickMapY][nClickMapX].nTowards = 0;
						}
					}
					else if (*pSelect != RS_NULL)	// ѡ��Ԫ��������
					{
						map->map[nClickMapY][nClickMapX].nType = *pSelect;
					}
					return_value |= DM_REDRAWMAP;
				}
			}

			// �Ҽ����� �� �������
			if (msgGraWnd.message == WM_RBUTTONUP || (msgGraWnd.message == WM_LBUTTONUP && !bMoved))
			{
				if (*pSelect == RS_NULL || msgGraWnd.message == WM_LBUTTONUP)	// ����Ԫ��
				{
					if (PointIsInMap(map, nClickMapX, nClickMapY))
					{
						switch (map->map[nClickMapY][nClickMapX].nType)
						{
						case RS_ROD:
							map->map[nClickMapY][nClickMapX].bPower = !map->map[nClickMapY][nClickMapX].bPower;
							break;

						case RS_BUTTON:
							// ��;�����������񣬹�����ֹ����
							HiEasyX::EndTask();

							ClickButton(map, nClickMapX, nClickMapY, *offset_x, *offset_y, *zoom, *p_bShowXY, *p_bShowRuler);

							// ������������
							if (!HiEasyX::SetWorkingWindow(hGraphicsWnd))
							{
								goto ToolBar_Begin;
							}
							HiEasyX::BeginTask();
							break;
						}
						return_value |= DM_REDRAWMAP;
					}
				}
			}

			// ����϶���ƽ��
			if (bGraWndLBtn)
			{
				if (msgGraWnd.x != msgGraWndLast.x || msgGraWnd.y != msgGraWndLast.y)
				{
					*offset_x += (int)((msgGraWnd.x - msgGraWndLast.x));
					*offset_y += (int)((msgGraWnd.y - msgGraWndLast.y));
					return_value |= DM_DRAWMAP;
					bMoved = true;
				}
			}

			// ���������δ���� Ctrl �Ž���ƽ��
			if (msgGraWnd.lbutton && !msgGraWnd.ctrl)
			{
				bGraWndLBtn = true;
				msgGraWndLast = msgGraWnd;
			}
			else
			{
				// ���
				if (msgGraWnd.lbutton && msgGraWnd.ctrl)
				{
					if (PointIsInMap(map, nClickMapX, nClickMapY))
					{
						map->map[nClickMapY][nClickMapX].nType = RS_NULL;
						map->map[nClickMapY][nClickMapX].bPower = false;
						return_value |= DM_REDRAWMAP;
					}
				}

				bGraWndLBtn = false;
				bMoved = false;
			}
		}

		HiEasyX::EndTask();
	}

ToolBar_Begin:

	// Tool Bar
	BEGIN_TASK_WND(hToolBarWnd);

	while (MouseHit())
	{
		msgToolWnd = getmessage(EM_MOUSE);

		if (msgToolWnd.message == WM_LBUTTONUP)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					int id = i * 2 + j;
					int x = j * nObjSize;
					int y = i * nObjSize;
					if (isInRect(msgToolWnd.x, msgToolWnd.y, { x,y,x + nObjSize, y + nObjSize }))
					{
						*pSelect = id;
					}
				}
			}
			return_value |= DM_TOOLBAR;
		}
	}

	END_TASK();

	return return_value;
}

// ���ƹ�����
void DrawToolBar(int* pSelect)
{
	BEGIN_TASK_WND(hToolBarWnd);

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
			if (*pSelect == id)
			{
				int x = j * nObjSize;
				int y = i * nObjSize;
				rectangle(x, y, x + nObjSize, y + nObjSize);
			}
		}
	}

	END_TASK();
	FLUSH_DRAW();
}

// ���ڴ�С�ı����Ϣ����
void WindowSized()
{
	BEGIN_TASK_WND(hGraphicsWnd);
	int w = getwidth(), h = getheight();
	rctSaveBtn = { 1,0,45,20 };
	rctResizeBtn = { 47,0,107,20 };
	rctHelpBtn = { w - 45 ,0,w - 1,20 };
	END_TASK();
}

// ���ƽ���ɫ����
// c ԭɫ
// kh, ks ,kl ɫ�ʵ� HSL ��λ���ر仯��
// ע��H �� S �Ľ��䲻��ʵ����ɫ����
void GradientRectangle(RECT rct, COLORREF c, float kh, float ks, float kl)
{
	float h, s, l;
	RGBtoHSL(c, &h, &s, &l);
	int nOldLineColor = getlinecolor();
	for (int i = 0; i < rct.right - rct.left; i++)
	{
		setlinecolor(HSLtoRGB(h + i * kh, s + i * ks, l + i * kl));
		line(i + rct.left, rct.top, i + rct.left, rct.bottom);
	}
	setlinecolor(nOldLineColor);
}

// ���ƽ���ɫ����
// c1, c2 ��ʼ��ɫ����ֹ��ɫ
void GradientRectangle(RECT rct, COLORREF c1, COLORREF c2)
{
	float h[2], s[2], l[2];
	COLORREF c[2] = { c1,c2 };
	for (int i = 0; i < 2; i++)
		RGBtoHSL(c[i], &h[i], &s[i], &l[i]);
	int len = rct.right - rct.left;
	float kh = (h[1] - h[0]) / len;
	float ks = (s[1] - s[0]) / len;
	float kl = (l[1] - l[0]) / len;
	GradientRectangle(rct, c[0], kh, ks, kl);
}

// ���ƽ���ɫ����
// c ��ʼ��ɫ��ֻ���� H �� S �Ĳ�����
// l1, l2 ��ʼ���ȡ���ֹ���ȣ��� 0 ~ 1 ����
void GradientRectangle(RECT rct, COLORREF c, double l1, double l2)
{
	float h, s, l;
	RGBtoHSL(c, &h, &s, &l);
	GradientRectangle(rct, HSLtoRGB(h, s, (float)l1), 0, 0, (float)(l2 - l1) / (rct.right - rct.left));
}

// ��ʼ����
void StartMenu(RsMap* pMap)
{
	HWND hStartMenuWnd = HiEasyX::initgraph_win32(640, 480, EW_NORMAL, L"��ʼ");
	DisableResizing(true);

	RECT rctBtn[2] = { {30, 140, 550, 240}, { 30,260,550,360 } };
	BEGIN_TASK_WND(hStartMenuWnd);

	setbkcolor(RGB(40, 50, 60));
	cleardevice();

	settextstyle(38, 0, L"����");
	settextcolor(RGB(215, 70, 130));
	WCHAR strTitle[] = L"Minecraft Redstone Simulator";
	int nTitleWidth = textwidth(strTitle);
	outtextxy(30, 40, strTitle);
	settextstyle(20, 0, L"����");
	settextcolor(CLASSICGRAY);
	outtextxy(30 + nTitleWidth - textwidth(strVersion), 80, strVersion);
	settextstyle(16, 0, L"����");
	settextcolor(GRAY);
	WCHAR strAuthor[] = L"by huidong <mailhuid@163.com>";
	outtextxy(getwidth() - textwidth(strAuthor), getheight() - textheight(strAuthor), strAuthor);

	setfillcolor(RGB(0, 0, 120));
	settextstyle(34, 0, L"����");
	setbkmode(TRANSPARENT);
	settextcolor(WHITE);
	GradientRectangle(rctBtn[0], RGB(160, 80, 200), 0.6, 0.3);
	GradientRectangle(rctBtn[1], RGB(160, 80, 200), 0.6, 0.3);
	outtextxy(60, 170, L"Open a project");
	outtextxy(60, 290, L"Create a project");

	END_TASK();
	FLUSH_DRAW();

	ExMessage msg;
	while (true)
	{
		if (HiEasyX::BeginTask())
		{
			while (MouseHit())
			{
				msg = getmessage(EM_MOUSE);
				if (msg.message == WM_LBUTTONUP)
				{
					if (isInRect(msg.x, msg.y, rctBtn[0]))
					{
						HiEasyX::EndTask();
						const WCHAR* str = SelectFile();
						if (lstrlen(str) != 0)
						{
							*pMap = OpenProject(str);
							goto end;
						}
						if (!HiEasyX::BeginTask())
						{
							exit(-1);
						}
					}
					else if (isInRect(msg.x, msg.y, rctBtn[1]))
					{
						HiEasyX::EndTask();
						if (ResizeBox(pMap, hStartMenuWnd))
						{
							goto end;
						}
						else
						{
							if (!HiEasyX::SetWorkingWindow(hStartMenuWnd) || !HiEasyX::BeginTask())
							{
								exit(-1);
							}
						}
					}
				}
			}
			HiEasyX::EndTask();
		}

		if (!HiEasyX::isAliveWindow(hStartMenuWnd))
		{
			exit(0);
		}

		Sleep(10);
	}
end:
	HiEasyX::closegraph_win32();
}

int main(int argc, char* argv[])
{
	HiEasyX::SetCustomIcon(IDI_ICON1, IDI_ICON1);
	SetConsoleTitle(L"Minecraft Redstone Simulator �ն�");

	loadimages();

	RsMap map = { 0,0,NULL };

	// ������ļ�����������
	if (argc > 1)
	{
		printf("���Ժ󣬴򿪵�ͼ�С���");

		TCHAR* strFileName = new TCHAR[strlen(argv[1]) + 1];
		memset(strFileName, 0, strlen(argv[1]) + 1);

		MultiByteToWideChar(CP_ACP, 0, argv[1], -1, strFileName, 1024);

		map = OpenProject(strFileName);
	}
	else
	{
		// �������뿪ʼ�˵�
		StartMenu(&map);
	}

	system("cls");
	printf("��ͼ���سɹ�������ָ���Բ�����ͼ������ help �鿴������\n");

	// ������������
	hGraphicsWnd = HiEasyX::initgraph_win32(1024, 768, EW_SHOWCONSOLE, L"Minecraft Redstone Simulator");
	hToolBarWnd = HiEasyX::initgraph_win32(nObjSize * 2, nObjSize * 4 + 100, EW_NORMAL, L"ToolBar");

	// Tool Bar �Ĵ�����ʽ
	EnableToolWindowStyle(true);
	DisableSystemMenu(true);
	DisableResizing(true);
	SetWindowPos(hToolBarWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	BEGIN_TASK_WND(hGraphicsWnd);
	setfillcolor(BLUE);
	setbkmode(TRANSPARENT);
	END_TASK();

	bool bShowXY = true;		// �Ƿ���ʾ��������
	bool bShowRuler = true;		// ������ʾ��ʽ
	int offset_x = 0;			// ��ͼƫ����ʾ
	int offset_y = 0;
	double zoom = 1;			// ��ͼ����
	int nSelect = RS_NULL;		// ������ѡ��
	bool bFirst = true;			// �Ƿ�Ϊ��һ������

	// �����û� CMD ����
	std::thread(CommandMessageLoop, &map, &offset_x, &offset_y, &zoom, &bShowXY, &bShowRuler).detach();

	// >>>
	// >>> ��Ϣ����
	// >>>
	while (HiEasyX::isAliveWindow(hGraphicsWnd))
	{
		int r = ProcessMouseMsg(&map, &offset_x, &offset_y, &zoom, &bShowXY, &bShowRuler, &nSelect);

		if ((r & DM_REDRAWMAP) || (r & DM_RESIZEMAP) || bFirst)
		{
			RunRsMap(&map);
		}

		// ����������Ϣ����
		if (HiEasyX::isWindowSizeChanged(hGraphicsWnd) || bFirst)
		{
			WindowSized();
		}

		if ((r & DM_TOOLBAR) || bFirst)
		{
			DrawToolBar(&nSelect);
		}

		if ((r & DM_DRAWMAP)
			|| (r & DM_REDRAWMAP)
			|| (r & DM_RESIZEMAP)
			|| (r & DM_ZOOM)
			|| bFirst)
		{
			Render(
				&map,
				(r & DM_REDRAWMAP) || bFirst,
				(r & DM_RESIZEMAP) || bFirst,
				(r & DM_ZOOM) || bFirst,
				offset_x, offset_y, zoom, bShowXY, bShowRuler
			);
		}

		if (bFirst)
		{
			bFirst = false;
		}

		Sleep(50);
	}

	closegraph();
	return 0;
}
