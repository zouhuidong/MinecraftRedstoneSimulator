///////////////////////////////////////////////
//
//	MCRedstoneSimulator
//	main.cpp
//	我的世界红石模拟器
//	by huidong <mailkey@yeah.net>
//
//	最后修改：2022.3.26
//

#include "resource.h"
#include "HiEasyX.h"
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <direct.h>

#define MAX_ZOOM 2.0
#define MIN_ZOOM 0.1

// 版本信息
const WCHAR strVersion[] = L"Version 1.4";

//
//    ___________________________________________________________________
//   / Warning													      |X|\
//   |___________________________________________________________________|
//   |                                                                   |
//   |       /\                                                          |
//   |      / !\    Warning                                              |
//   |     /____\                                                        |
//   |                如果项目有更新，请更新 main.cpp 顶部的注释上的时间，   |
//   |                strVersion 上的版本，./res/help/help.html 上的    |
//   |                更新日志，修改时间。                                 |
//   |___________________________________________________________________|
//
//


// 红石方块种类
enum RedstoneObjectTypes
{
	RS_NULL,	// 空气方块（无方块）
	RS_POWDER,	// 红石粉
	RS_ROD,		// 拉杆
	RS_BUTTON,	// 按钮
	RS_TORCHE,	// 红石火把
	RS_LIGHT,	// 红石灯
	RS_RELAY,	// 中继器
	RS_CROSS	// 交叉排线板（不属于 MC 方块，仅在平面交叉电路中发挥作用）
};

// 红石方块朝向
enum RedstoneTowards
{
	RS_TO_UP,	// 朝向上
	RS_TO_RIGHT,// 朝向右
	RS_TO_DOWN,	// 朝向下
	RS_TO_LEFT	// 朝向左
};

// 红石方块定义
typedef struct RedstoneObject
{
	int nType = RS_NULL;			// 方块种类
	bool bPower = false;			// 是否充能（对于交叉线，任意方向充能都会标识）
	int nTowards = RS_TO_UP;		// 朝向

	bool bUprightPower = false;		// 竖直方向是否充能（仅用于交叉线）
	bool bHorizonPower = false;		// 水平方向是否充能（仅用于交叉线）

	int nPowerCount = 0;			// 供电数量
	POINT* pPowerList = NULL;		// 供电电源坐标

}RsObj;

// 红石地图
typedef struct RedstoneMap
{
	int w, h;		// 地图宽高
	RsObj** map;	// 地图
}RsMap;

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

COLORREF colorPower = /*RGB(200, 0, 0)*/RGB(0, 240, 0);			// 有电的颜色
COLORREF colorNoPower = /*RGB(100, 0, 0)*/RGB(100, 100, 100);	// 无电的颜色

// 红石线粗
int nPowderWidth = /*4*/3;

// 物体大小
int nObjSize;
int nHalfObjSize;

// 全局按钮位置信息
RECT rctHelpBtn;
RECT rctSaveBtn;
RECT rctResizeBtn;

// 窗口句柄
HWND hGraphicsWnd;	// 主绘图窗口
HWND hToolBarWnd;	// 工具栏窗口

// 红石地图输出坐标
int nMapOutX = 0, nMapOutY = 20;

// 标尺宽高
int nRulerWidth = 20;
int nRulerHeight = 20;


// 函数定义

bool operator==(POINT a, POINT b)
{
	return a.x == b.x && a.y == b.y;
}

// 查找供电表中是否有某个电源
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

// 添加多个供电源到供电表
// 返回添加成功的个数（即与原表不重复的项的个数）
int AddToPowerList(POINT** pOldPowerList, int* p_nOldCount, POINT* pPowerList, int nCount)
{
	POINT* pNewList = new POINT[*p_nOldCount + nCount];
	int sum = 0;	// 去重后加入表中的数量
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

// 添加多个供电源到供电表
int AddToPowerList(RsObj* pObj, POINT* pPowerList, int nCount)
{
	return AddToPowerList(&pObj->pPowerList, &pObj->nPowerCount, pPowerList, nCount);
}

// 得到空方块
RsObj GetNullObj()
{
	return RsObj{ RS_NULL ,false ,RS_TO_UP,NULL };
}

// 初始化红石地图
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

// 判断一物体是否为信号源方块
bool isPowerObj(RsObj* obj)
{
	return obj->nType == RS_ROD || obj->nType == RS_BUTTON || obj->nType == RS_TORCHE;
}

// 是否为普通方块（无论是否可导电，含红石火把，不含其他电源方块）
bool isNormalObj(RsObj* obj)
{
	return (obj->nType != RS_NULL && !isPowerObj(obj)) || obj->nType == RS_TORCHE;
}

// 是否为可导电方块（筛选方式 1）
// 包含一切实体方块（除了红石灯）
bool isConductiveObj(RsObj* obj)
{
	return obj->nType != RS_NULL && obj->nType != RS_LIGHT;
}

// 是否为可导电方块（筛选方式 2）
// 包含普通方块（除了红石灯），不包含信号源方块
bool isConductiveObj2(RsObj* obj)
{
	return isNormalObj(obj) && obj->nType != RS_LIGHT && !isPowerObj(obj);
}

// 判断在某个方向上是否有电能流入
// x, y 当前方块坐标
// kx, ky 搜索方向（只能其中一个为 0，一个为正负 1）
// pPowerList, p_nPowerCount 如果有电，传回的供电表
// p_bPower 给电的方块是否为电源
//
// 若 p_bPower 返回为 true，则需要在外部释放 pPowerList
bool isPowerTransfer(RsMap* pMap, int x, int y, int kx, int ky, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	bool r = false;					// 标记是否检索到电能
	int nx = x + kx, ny = y + ky;	// 查找坐标
	if (!(ny >= 0 && ny < pMap->h && nx >= 0 && nx < pMap->w))
	{
		return false;
	}

	RsObj obj = pMap->map[ny][nx];	// 当前方块

	// 给电方块须是 1 类导体，且需要有供电源或者本身有电
	if (isConductiveObj(&obj) && (obj.nPowerCount || obj.bPower))
	{
		switch (obj.nType)
		{
			// 对于中继器，需要进行方向判断
		case RS_RELAY:
			if (ky > 0 && obj.nTowards == RS_TO_UP)				r = true;
			else if (ky < 0 && obj.nTowards == RS_TO_DOWN)		r = true;
			else if (kx > 0 && obj.nTowards == RS_TO_LEFT)		r = true;
			else if (kx < 0 && obj.nTowards == RS_TO_RIGHT)		r = true;
			break;

			// CROSS 需要继续深入查找
		case RS_CROSS:
			return isPowerTransfer(pMap, nx, ny, kx, ky, pPowerList, p_nPowerCount, p_bPower);
			break;

			// 其余方块可以直接标记
		default:	r = true;	break;
		}
	}

	// 有电
	if (r)
	{
		// 电源供电，则供电表中只返回此电源
		if (obj.bPower)
		{
			POINT* p = new POINT;
			*p = { nx,ny };
			*pPowerList = p;
			*p_nPowerCount = 1;
			*p_bPower = true;
		}
		// 导体供电，则返回其整个供电表
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

// 上面是否来电
bool isUpPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, 0, -1, pPowerList, p_nPowerCount, p_bPower);
}

// 下面是否来电
bool isDownPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, 0, 1, pPowerList, p_nPowerCount, p_bPower);
}

// 左边是否来电
bool isLeftPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, -1, 0, pPowerList, p_nPowerCount, p_bPower);
}

// 右边是否来电
bool isRightPower(RsMap* pMap, int x, int y, POINT** pPowerList, int* p_nPowerCount, bool* p_bPower)
{
	return isPowerTransfer(pMap, x, y, 1, 0, pPowerList, p_nPowerCount, p_bPower);
}

// 在接受附近电流时加入新的供电源
bool JoinPowerList_AcceptPower(RsObj* pObj, POINT* pPowerList, int nCount, bool bPower)
{
	int r = AddToPowerList(pObj, pPowerList, nCount);
	if (bPower)
	{
		delete pPowerList;
	}
	return r;
}

// 接受附近电流，返回是否接受到电流
bool AcceptPowerNearby(RsMap* map, int x, int y)
{
	POINT* pPowerList = NULL;
	int nPowerCount = 0;
	bool bPower = false;

	RsObj* pObj = &map->map[y][x];	// 当前方块

	// 接收四个方向的信号
	bool (*funcs[])(RsMap*, int, int, POINT**, int*, bool*) = { isLeftPower, isRightPower, isUpPower, isDownPower };
	bool result = false;
	for (int i = 0; i < 4; i++)
	{
		if (funcs[i](map, x, y, &pPowerList, &nPowerCount, &bPower))
		{
			// 标记是否成功供能
			bool bSupply = true;
			switch (pObj->nType)
			{
				// 红石火把只接受中继器供能
			case RS_TORCHE:
				switch (i)
				{
				case 0: if (map->map[y][x - 1].nType != RS_RELAY) bSupply = false; break;
				case 1: if (map->map[y][x + 1].nType != RS_RELAY) bSupply = false; break;
				case 2: if (map->map[y - 1][x].nType != RS_RELAY) bSupply = false; break;
				case 3: if (map->map[y + 1][x].nType != RS_RELAY) bSupply = false; break;
				}
				break;
				// 中继器只接受同向供能
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
			// 供能成功才加入供能表
			if (bSupply)
			{
				if (JoinPowerList_AcceptPower(pObj, pPowerList, nPowerCount, bPower))
				{
					result = true;
				}
			}
			// 若供能不成功，也要回收内存
			else if (bPower)
			{
				delete pPowerList;
			}
		}
	}
	return result;
}

void ConductPower(RsMap* pMap, int x, int y, POINT pPower);

// 执行关于普通方块的处理（含红石火把）
// pPower 发起电源位置
void RunObj(RsMap* pMap, int x, int y, POINT pPower)
{
	RsObj* pObj = &pMap->map[y][x];

	//// 电路没有闭合，则继续延伸
	//if (!SearchPowerInList(pObj->pPowerList, pObj->nPowerCount, pPower))
	//{

	// 接受到附近电流
	if (AcceptPowerNearby(pMap, x, y))
	{
		// 如果是可以继续传导的物体，则继续
		if (isConductiveObj2(pObj))
		{
			// 继续导电
			ConductPower(pMap, x, y, pPower);
		}
	}

	//}
}

// 向四周导电
// pPower 原电源位置
void ConductPower(RsMap* pMap, int x, int y, POINT pPower)
{
	// 只对普通方块和红石火把进行初步导电
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

// 确认电源状态
// pPower		电源位置
// flagFirst	是否为第一次调用
void CheckPower(RsMap* pMap, POINT pPower, bool flagFirst = false)
{
	static POINT* pVisited = NULL;
	static int nCount = 0;

	RsObj* pObj = &pMap->map[pPower.y][pPower.x];

	// 只有火把需要确认状态
	if (pObj->nType == RS_TORCHE)
	{
		// 记录足迹
		AddToPowerList(&pVisited, &nCount, &pPower, 1);

		// 递归确认所有电源的状态
		for (int i = 0; i < pObj->nPowerCount; i++)
		{
			// 重复项不再检验
			if (!SearchPowerInList(pVisited, nCount, pObj->pPowerList[i]))
			{
				CheckPower(pMap, pObj->pPowerList[i]);
			}
		}

		// 若存在通电电源，则熄灭
		for (int i = 0; i < pObj->nPowerCount; i++)
		{
			POINT p = pObj->pPowerList[i];
			if (pMap->map[p.y][p.x].bPower)
			{
				pObj->bPower = false;
			}
		}
	}

	// 位于递归头，回收内存
	if (flagFirst && pVisited != NULL)
	{
		delete pVisited;
		pVisited = NULL;
		nCount = 0;
	}
}

// 运行红石地图
void RunRsMap(RsMap* pMap)
{
	// 重置初始供电状态
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			// 保留电源开关状态，红石火把设为有电，其余无电
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

			// 清空所有供电表
			if (pMap->map[j][i].pPowerList != NULL)
			{
				delete pMap->map[j][i].pPowerList;
				pMap->map[j][i].pPowerList = NULL;
				pMap->map[j][i].nPowerCount = 0;
			}
		}
	}

	// 电源开电
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (isPowerObj(&pMap->map[j][i]) && pMap->map[j][i].bPower)
			{
				// 导电
				ConductPower(pMap, i, j, { i,j });
			}
		}
	}

	// 审查电源状态（红石火把开关设置）
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

	// 遍历地图，设置导体通电状态
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			// 只对普通方块进行设置
			if (!isPowerObj(&pMap->map[j][i]) && pMap->map[j][i].nType != RS_NULL)
			{
				// 遍历供能表，给电
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

	// 所有导体的充能状态设置完毕后，再次对交叉线单独设置充能状态
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (pMap->map[j][i].nType == RS_CROSS)
			{
				// 预设为 false
				pMap->map[j][i].bUprightPower = false;
				pMap->map[j][i].bHorizonPower = false;

				// 判断一方块是否为有效电源
				auto isActivePower = [pMap](int x, int y) {
					if (pMap->map[y][x].bPower)
						if (isConductiveObj(&pMap->map[y][x]) && pMap->map[y][x].nType != RS_CROSS)
							return true;
					return false;
				};

				// 由于交叉线在水平和垂直方向互不干扰，所以若某个方向充能，沿此方向搜索一定能遇到充能方块

				// 垂直方向搜索
				// 若已标记垂直方向充能，则跳出
				for (int k = -1; !pMap->map[j][i].bUprightPower && k <= 1; k += 2)
					for (int y = j + k;
						((y >= 0 && y < pMap->h)					// 搜索越界
							&& isConductiveObj(&pMap->map[y][i])	// 必须沿导电方块搜索
							&& pMap->map[y][i].bPower				// 搜索路径必须完全充能
							&& !pMap->map[j][i].bUprightPower);		// 已标记充能后跳出
						y += k)
						if (isActivePower(i, y))
							pMap->map[j][i].bUprightPower = true;

				// 水平方向搜索
				// 若已标记水平方向充能，则跳出
				for (int k = -1; !pMap->map[j][i].bHorizonPower && k <= 1; k += 2)
					for (int x = i + k;
						((x >= 0 && x < pMap->w)					// 搜索越界
							&& isConductiveObj(&pMap->map[j][x])	// 必须沿导电方块搜索
							&& pMap->map[j][x].bPower				// 搜索路径必须完全充能
							&& !pMap->map[j][i].bHorizonPower);		// 已标记充能后跳出
						x += k)
						if (isActivePower(x, j))
							pMap->map[j][i].bHorizonPower = true;
			}
		}
	}

}

// 放置物品到地图
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

// 选择文件
// isSave标志着是否为保存模式
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

// 判断字符是否为数字
bool isNum(char ch)
{
	return (ch >= '0' && ch <= '9') || ch == '-';
}

// 判读一字符串是否完全为数字
bool isAllNum(const char* str)
{
	for (int i = 0; i < (int)strlen(str); i++)
		if (!isNum(str[i]))
			return false;
	return true;
}

// 在当前进度往下读取一串数字
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

// 删除地图
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

// 重设地图大小
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

// 保存项目
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

// 打开项目
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

	// 分析文件

	// 文件格式：
	// 地图宽 地图高
	// 物体id,朝向,能量 物体id,朝向,能量 ......

	// 读取地图宽高
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

			// 读物品id
			if (!ReadNum(chProject, index, nType))
				return RsMap{};
			index++;

			// 读朝向
			if (!ReadNum(chProject, index, nTowards))
				return RsMap{};
			index++;

			// 读能量
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

// 导入项目
// out 主项目
// in 被导入项目
// l, l 被导入项目位于主项目的左上角坐标
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

// 图片拉伸
// width, height 拉伸后的图片大小
// img 原图像
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
 *    参考自http://tieba.baidu.com/pPower/5218523817
 *    函数名:zoomImage(IMAGE* pImg,int width，int height)
 *    参数说明:pImg是原图指针，width1和height1是目标图片的尺寸。
 *    函数功能:将图片进行缩放，返回目标图片 可以自定义长与宽，也可以只给长自动计算宽
 *    返回目标图片
*/
IMAGE zoomImage(IMAGE* pImg, int newWidth, int newHeight = 0)
{
	// 防止越界
	if (newWidth < 0 || newHeight < 0) {
		newWidth = pImg->getwidth();
		newHeight = pImg->getheight();
	}

	// 当参数只有一个时按比例自动缩放
	if (newHeight == 0) {
		// 此处需要注意先*再/。不然当目标图片小于原图时会出错
		newHeight = newWidth * pImg->getheight() / pImg->getwidth();
	}

	// 获取需要进行缩放的图片
	IMAGE newImg(newWidth, newHeight);

	// 分别对原图像和目标图像获取指针
	DWORD* oldDr = GetImageBuffer(pImg);
	DWORD* newDr = GetImageBuffer(&newImg);

	// 赋值 使用双线性插值算法
	for (int i = 0; i < newHeight - 1; i++) {
		for (int j = 0; j < newWidth - 1; j++) {
			int t = i * newWidth + j;
			int xt = j * pImg->getwidth() / newWidth;
			int yt = i * pImg->getheight() / newHeight;
			newDr[i * newWidth + j] = oldDr[xt + yt * pImg->getwidth()];
			// 实现逐行加载图片
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

// 得到地图画面
// map						地图
// redraw					标识是否重新绘制地图
// resize					标识地图尺寸是否更新
// bShowXY					指定是否显示坐标
// bShowRuler				指定显示坐标时的方式，为 true 表示显示标尺，为 false 表示直接在方块上显示坐标
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
		if (redraw)
		{
			SetWorkingImage(&imgMap);
			settextcolor(WHITE);
			settextstyle(10, 0, L"黑体");
			setbkmode(TRANSPARENT);

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

						// 实时绘制红石粉
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

						line(draw_x + nHalfObjSize, draw_y, draw_x + nHalfObjSize, draw_y + nObjSize);	// 竖向电路

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

// 从命令中分离参数
// cmd 原命令
// chCmdsArray_out 输出参数列表
// nArgsNum_out 输出参数数量
void GetArguments(const char* cmd, char*** chCmdsArray_out, int* nArgsNum_out)
{
	// 参数数量
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

	// 分离每个参数
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

// 帮助菜单
void HelpMenu()
{
	system("start ./res/help/help.html");
	printf("已打开帮助文档，请稍后页面打开。\n");
}

// 从字符串转换成 ID 号
// str 表示原字符串
// type_out 表示 ID 类型，为 0 表示物品，为 1 表示朝向
// id_out 表示 ID
// 返回输入的字符串是否合法
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

	// 错误的输入
	else
	{
		return false;
	}

	return true;
}

// 检验坐标合法性（是否超出地图）
bool PointIsInMap(RsMap* map, int x, int y)
{
	return x >= 0 && x < map->w&& y >= 0 && y < map->h;
}

// 处理图像
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
		// 缩放图像
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

	// 输出
	BEGIN_TASK_WND(hGraphicsWnd);
	{
		cleardevice();

		// 输出图像
		// 存在标尺时，需要将图像偏移加上标尺的大小
		// 干脆任何时候都加上吧，这样计算鼠标坐标的时候不需要分情况
		int render_offset_x = nMapOutX + offset_x + (int)(nRulerWidth * zoom);
		int render_offset_y = nMapOutY + offset_y + (int)(nRulerHeight * zoom);
		putimage(render_offset_x, render_offset_y, &imgMap_zoomed);

		// 复制绘制好的标尺
		if (bShowXY && bShowRuler)
		{
			putimage(nMapOutX + offset_x + (int)(nRulerWidth * zoom), nMapOutY, &imgRulerX_zoomed);
			putimage(nMapOutX, nMapOutY + offset_y + (int)(nRulerHeight * zoom), &imgRulerY_zoomed);
		}

		// 绘制顶栏
		int w = getwidth();
		setfillcolor(RGB(50, 50, 50));
		solidrectangle(0, 0, w, nMapOutY);
		line(0, nMapOutY, w, nMapOutY);
		WCHAR strName[] = L"Minecraft Redstone Simulator";
		outtextxy((w - textwidth(strName)) / 2, 3, strName);

		// 按钮绘制
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

// 点击一个按钮（阻塞）
void ClickButton(RsMap* map, int x, int y, int offset_x, int offset_y, double zoom, bool bShowXY, bool bShowRuler)
{
	int delay = 1000;

	if (map->map[y][x].nType == RS_BUTTON)
	{
		map->map[y][x].bPower = true;
		RunRsMap(map);

		Render(map, true, false, false, offset_x, offset_y, zoom, bShowXY, bShowRuler);	// 手动重绘

		Sleep(delay);
		map->map[y][x].bPower = false;
		RunRsMap(map);

		Render(map, true, false, false, offset_x, offset_y, zoom, bShowXY, bShowRuler);
	}
}

// 得到排序好且合法的坐标（针对两个坐标使用）
void GetSortingPoint(RsMap* map, int* x1, int* y1, int* x2, int* y2)
{
	// 确保x1 <= x2, y1 <= y2
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

	// 确保坐标合法
	if (*x1 < 0)
		*x1 = 0;
	if (*x2 >= map->w)
		*x2 = map->w - 1;
	if (*y1 < 0)
		*y1 = 0;
	if (*y2 >= map->h)
		*y2 = map->h - 1;
}

// 在红石地图中绘制红石直线（不得弯曲），可指定用以填充红石直线的物品
// 返回是否绘制成功，绘制不成功的原因都是直线有弯曲
bool LineRsMap(RsMap* map, int x1, int y1, int x2, int y2, int object)
{
	// 整理坐标
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

// 清除红石地图中的一块区域
void ClearRsMap(RsMap* map, int x1, int y1, int x2, int y2)
{
	// 整理坐标
	GetSortingPoint(map, &x1, &y1, &x2, &y2);

	for (int i = y1; i <= y2; i++)
	{
		for (int j = x1; j <= x2; j++)
		{
			PutObjectToRsMap(map, j, i, RS_NULL);
		}
	}
}

// 处理用户命令输入（阻塞）
void ProcessCommand(RsMap* map, int* offset_x, int* offset_y, double* zoom, bool* p_bShowXY, bool* p_bShowRuler)
{
	// 地图移动的单位大小（像素）
	const int offset_unit_size = 10;

	// 命令最长长度
	int nMaxInputdSize = 1024;
	char* chCmd = new char[nMaxInputdSize];
	memset(chCmd, 0, nMaxInputdSize);

	gets_s(chCmd, nMaxInputdSize);

	// 参数数量
	static int nArgsNum = 0;
	static char** chCmdsArray = NULL;

	// 清理上一次的内存
	if (chCmdsArray != NULL)
	{
		for (int i = 0; i < nArgsNum; i++)
			delete[] chCmdsArray[i];
		delete[] chCmdsArray;
	}

	// 解析参数
	GetArguments(chCmd, &chCmdsArray, &nArgsNum);

	delete[] chCmd;

	// 设置是否开启显示坐标
	if (nArgsNum == 1 && strcmp(chCmdsArray[0], "xy") == 0)
	{
		*p_bShowXY = !(*p_bShowXY);
	}

	// 切换坐标显示模式
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "xy_mode") == 0)
	{
		*p_bShowRuler = !(*p_bShowRuler);
	}

	// 帮助
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "help") == 0)
	{
		HelpMenu();
		return;
	}

	// 清屏
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "cls") == 0)
	{
		system("cls");
		return;
	}

	// 直接输入坐标表示在某位置放置红石粉，如果那个位置已有物品，则清除那个物品
	else if (nArgsNum == 2 && isAllNum(chCmdsArray[0]) && isAllNum(chCmdsArray[1]))
	{
		int x = atoi(chCmdsArray[0]);
		int y = atoi(chCmdsArray[1]);

		if (!PointIsInMap(map, x, y))
		{
			printf("坐标越界。\n");
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

	// 坐标 坐标 物品 （朝向）
	// 两种情况：
	// 在某位置放置某物品，如果那个位置已有物品，则覆盖那个物品。红石中继器有朝向设定，如果留空则表示向上
	// 设置某位置的物品的朝向（针对红石中继器）
	else if ((nArgsNum == 3 || nArgsNum == 4) && isAllNum(chCmdsArray[0]) && isAllNum(chCmdsArray[1]) && !isAllNum(chCmdsArray[2]))
	{
		int type;
		int id;
		if (!GetIdFromString(chCmdsArray[2], &type, &id))
		{
			printf("方块名称错误。\n");
			return;
		}

		int x = atoi(chCmdsArray[0]);
		int y = atoi(chCmdsArray[1]);

		if (!PointIsInMap(map, x, y))
		{
			printf("坐标越界。\n");
			return;
		}

		if (type == 0)
		{
			PutObjectToRsMap(map, x, y, id);

			// 如有四个参数，说明还有一个朝向
			if (nArgsNum == 4)
			{
				int type2;
				int direction;
				if (!GetIdFromString(chCmdsArray[3], &type2, &direction))
				{
					printf("命令执行不完全后因语法错误而终止了。错误原因：第四个参数未能指定方块朝向。\n");
					return;
				}

				if (type2 == 1)
				{
					map->map[y][x].nTowards = direction;
				}
				else
				{
					printf("命令执行不完全后因语法错误而终止了。错误原因：第四个参数未能指定方块朝向。\n");
					return;
				}
			}
		}
		else if (type == 1)
		{
			if (map->map[y][x].nType == RS_NULL)
			{
				printf("该坐标并无物品方块。\n");
				return;
			}
			else
			{
				map->map[y][x].nTowards = id;
			}
		}
	}

	// . 坐标x 坐标y
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], ".") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int x = atoi(chCmdsArray[1]);
		int y = atoi(chCmdsArray[2]);

		if (!PointIsInMap(map, x, y))
		{
			printf("坐标越界。\n");
			return;
		}

		switch (map->map[y][x].nType)
		{
		case RS_NULL:
			printf("该坐标并无物品方块。\n");
			return;
			break;

		case RS_ROD:
			map->map[y][x].bPower = !map->map[y][x].bPower;
			break;

		case RS_BUTTON:
			ClickButton(map, x, y, *offset_x, *offset_y, *zoom, *p_bShowXY, *p_bShowRuler);
			break;

		default:
			printf("该物品不支持点击操作。\n");
			return;
			break;
		}
	}

	// resize 宽 高
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "resize") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int w = atoi(chCmdsArray[1]);
		int h = atoi(chCmdsArray[2]);

		if (w <= 0 || h <= 0)
		{
			printf("地图大小不得小于等于0\n");
			return;
		}

		ResizeRsMap(map, w, h);
	}

	// save 保存项目
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "save") == 0)
	{
		if (SaveProject(*map, SelectFile(true)))
		{
			printf("保存成功。\n");
			return;
		}
		else
		{
			printf("保存失败。\n");
			return;
		}
	}

	// 将地图画面的xy偏移都回到0
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "reset_map_offset") == 0)
	{
		*offset_x = 0;
		*offset_y = 0;
	}

	// 移动地图
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "up") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_y -= atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// 移动地图
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "down") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_y += atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// 移动地图
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "left") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_x -= atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// 移动地图
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "right") == 0 && isAllNum(chCmdsArray[1]))
	{
		*offset_x += atoi(chCmdsArray[1]) * offset_unit_size;
	}

	// 缩放地图
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "zoom") == 0 && isAllNum(chCmdsArray[1]))
	{
		*zoom = atoi(chCmdsArray[1]) / 100.0;
	}

	// 缩放地图
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "zoom+") == 0 && isAllNum(chCmdsArray[1]))
	{
		*zoom += atoi(chCmdsArray[1]) / 100.0;
	}

	// 缩放地图
	else if (nArgsNum == 2 && strcmp(chCmdsArray[0], "zoom-") == 0 && isAllNum(chCmdsArray[1]))
	{
		*zoom -= atoi(chCmdsArray[1]) / 100.0;
	}

	// 得到地图偏移量
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "get_map_offset") == 0)
	{
		printf("x轴偏移量：%d个单位（即%d像素）\ny轴偏移量：%d个单位（即%d像素）\n",
			*offset_x / offset_unit_size, *offset_x, *offset_y / offset_unit_size, *offset_y);
	}

	// 得到地图缩放量
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "get_map_zoom") == 0)
	{
		printf("缩放量：%d%%\n", (int)(*zoom * 100));
	}

	// import l l
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "import") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int x = atoi(chCmdsArray[1]);
		int y = atoi(chCmdsArray[2]);

		/*if (!PointIsInMap(map, l, l))
		{
			printf("坐标越界。\n");
			return;
		}*/

		printf("请选择要导入的项目。\n");

		const WCHAR* map_file = SelectFile();
		RsMap import_map = OpenProject(map_file);

		char warning[512] = { 0 };
		sprintf_s(warning, 512,
			"你正在导入一个项目。以下是导入项目的信息：\n"
			"导入子项目到主项目的左上角位置：( %d, %d )。\n"
			"子项目地图大小：宽 %d 格，高 %d 格。\n"
			"子项目将覆盖主项目的以下范围：( %d, %d ) 到 ( %d, %d )，如果超出主项目的地图大小则自动将超出的部分裁剪。\n"
			"\n\n"
			"是否继续导入此项目？点击【确定】来确定导入此项目，否则点击【取消】终止此操作。如果你担心造成不可逆的后果，可以先保存当前项目，再进行导入。\n"
			, x, y, import_map.w, import_map.h, x, y, x + import_map.w, y + import_map.h);

		wchar_t wstr[512] = { 0 };
		MultiByteToWideChar(CP_ACP, 0, warning, (int)strlen(warning), wstr, (int)strlen(warning));

		if (MessageBox(GetConsoleWindow(), wstr, L"确定？", MB_OKCANCEL) == IDOK)
		{
			ImportProject(map, import_map, x, y);
		}

		DeleteRsMap(&import_map);
		printf("导入完毕。\n");
	}

	// line 坐标x1 坐标y1 坐标x2 坐标y2 (物品) : 画红石直线（不能弯曲），如果在末尾加上物品名表示以某方块填充直线。
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
			printf("坐标越界。\n");
			return;
		}

		if (nArgsNum == start + 5)
		{
			int type;
			if (!GetIdFromString(chCmdsArray[start + 4], &type, &id) || type != 0)
			{
				printf("最后一个参数应该是物品id。\n");
				return;
			}
		}

		if (!LineRsMap(map, x1, y1, x2, y2, id))
		{
			printf("(%d, %d) 到 (%d, %d) 有弯曲，无法绘制直线。\n", x1, y1, x2, y2);
			return;
		}
	}

	// clear 坐标x1 坐标y1 坐标x2 坐标y2 : 以空气方块覆盖 (x1,y1) 到 (x2,y2) 的所有方块。
	else if (nArgsNum == 5 && strcmp(chCmdsArray[0], "clear") == 0 &&
		isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]) && isAllNum(chCmdsArray[3]) && isAllNum(chCmdsArray[4]))
	{
		int x1 = atoi(chCmdsArray[1]);
		int y1 = atoi(chCmdsArray[2]);
		int x2 = atoi(chCmdsArray[3]);
		int y2 = atoi(chCmdsArray[4]);

		if (!PointIsInMap(map, x1, y1) || !PointIsInMap(map, x2, y2))
		{
			printf("坐标越界。\n");
			return;
		}

		ClearRsMap(map, x1, y1, x2, y2);
	}

	// exit 退出程序
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "exit") == 0)
	{
		if (MessageBox(GetConsoleWindow(), L"确定要退出程序吗？", L"退出程序", MB_OKCANCEL) == IDOK)
		{
			exit(0);
		}
	}

	// cmd_window_top 使cmd顶置
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "cmd_window_top") == 0)
	{
		SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// cmd_window_no_top 使cmd不顶置
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "cmd_window_no_top") == 0)
	{
		SetWindowPos(GetConsoleWindow(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// map_window_top 使绘图窗口顶置
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "map_window_top") == 0)
	{
		SetWindowPos(GetHWnd(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// map_window_no_top 使绘图窗口不顶置
	else if (nArgsNum == 1 && strcmp(chCmdsArray[0], "map_window_no_top") == 0)
	{
		SetWindowPos(GetHWnd(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	// resize_cmd_window 设置cmd窗口大小
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "resize_cmd_window") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int w = atoi(chCmdsArray[1]);
		int h = atoi(chCmdsArray[2]);
		char cmd[64] = { 0 };
		sprintf_s(cmd, 64, "mode con cols=%d lines=%d", w, h);
		system(cmd);
	}

	// resize_map_window 设置绘图窗口大小
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "resize_map_window") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int w = atoi(chCmdsArray[1]);
		int h = atoi(chCmdsArray[2]);
		Resize(NULL, w, h);
	}

	else
	{
		printf("未知命令。\n");
		return;
	}

	printf("执行完毕。\n");
}

// 阻塞处理 CMD 消息
void CommandMessageLoop(RsMap* map, int* offset_x, int* offset_y, double* zoom, bool* p_bShowXY, bool* p_bShowRuler)
{
	while (true)
	{
		ProcessCommand(map, offset_x, offset_y, zoom, p_bShowXY, p_bShowRuler);
	}
}

// 显示帮助窗口
void HelpBox()
{
	int w = 820, h = 420;
	HWND hHelpWnd = HiEasyX::initgraph_win32(w, h, EW_NORMAL, L"帮助", NULL, hGraphicsWnd);
	DisableResizing(true);

	RECT rctDoneBtn = { w / 2 - 25,h - 35,w / 2 + 25,h - 8 };

	BEGIN_TASK();

	settextstyle(46, 0, L"宋体");
	outtextxy(30, 20, L"MCRedStoneSimulator 使用说明");
	settextstyle(22, 0, L"微软雅黑");
	outtextxy(30, 80, L"本程序是 huidong <mailhuid@163.com> 制作的红石（电路）模拟软件");
	outtextxy(30, 110, L"您可以在 ToolBar 中选择工具或元件，并在显示有电路图的窗口中使用它们来编辑电路图。");
	outtextxy(30, 140, L"在程序的命令行窗口中您可以键入指令对电路项目进行操作，输入“help”指令可以查看指令表。");
	outtextxy(30, 200, L"左键单击：操作元件    Ctrl + 左键：清除元件           在 ToolBar 中：");
	outtextxy(30, 230, L"左键拖动：移动地图    Ctrl + 右键：旋转元件           选择到鼠标时，右键单击可以操作元件");
	outtextxy(30, 260, L"鼠标滚轮：缩放地图");
	outtextxy(410, 260, L"选择到元件时，右键按下可以放置元件");
	outtextxy(30, 310, L"作者博客：http://huidong.xyz   EasyX 主站：https://easyx.cn");
	outtextxy(30, 340, L"Github 项目地址：https://github.com/zouhuidong/MinecraftRedstoneSimulator");

	setfillcolor(BLUE);
	setbkmode(TRANSPARENT);
	fillrectangle_RECT(rctDoneBtn);
	outtextxy(rctDoneBtn.left + 4, rctDoneBtn.top + 3, L"Done");
	END_TASK();
	FLUSH_DRAW();

	// 消息响应
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

WCHAR strMapSize[2][12];	// 在输入框中存储地图大小数据
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
		outtextxy(30, 30, L"宽：");
		outtextxy(30, 55, L"高：");
		settextcolor(GRAY);
		settextstyle(14, 0, L"宋体");
		//outtextxy(30, 80, L"地图大小超出 50x50 或导致卡顿");
		END_TASK();
		break;
	}

	return true;
}

// 显示设置地图大小窗口
// 返回是否成功设置了一个新的大小
bool ResizeBox(RsMap* pMap, HWND hParent)
{
	_itow_s(pMap->w, strMapSize[0], 10);
	_itow_s(pMap->h, strMapSize[1], 10);

	HWND hResizeWnd = HiEasyX::initgraph_win32(320, 120, EW_NORMAL, L"设置地图大小", ResizeBoxWndProc, hParent);
	DisableResizing(true);

	while (HiEasyX::isAliveWindow(hResizeWnd))
	{
		Sleep(10);
	}

	int w = _wtoi(strMapSize[0]);
	int h = _wtoi(strMapSize[1]);

	if (w * h == 0)
	{
		MessageBox(hParent, L"无效值", L"Error", MB_OK | MB_ICONERROR);
		return false;
	}
	else if (w != pMap->w || h != pMap->h)
	{
		ResizeRsMap(pMap, w, h);
		return true;
	}
	return false;
}

// 重绘消息
enum DrawMsg
{
	DM_NULL = 0,		// 无重绘消息
	DM_TOOLBAR = 1,		// 重绘工具栏
	DM_DRAWMAP = 2,		// 绘制地图
	DM_REDRAWMAP = 4,	// 重新绘制地图
	DM_RESIZEMAP = 8,	// 地图大小改变
	DM_ZOOM = 16,		// 缩放
};

// 处理鼠标消息
// 返回重绘消息（见 DrawMsg）
int ProcessMouseMsg(RsMap* map, int* offset_x, int* offset_y, double* zoom, bool* p_bShowXY, bool* p_bShowRuler, int* pSelect)
{
	// 绘图窗口消息
	ExMessage msgGraWnd;
	static ExMessage msgGraWndLast = {};
	static bool bGraWndLBtn = false;
	static bool bMoved = false;

	// 工具栏窗口消息
	ExMessage msgToolWnd;

	// 返回值
	int return_value = DM_NULL;

	// 滚轮缩放大小
	double dZoom;

	// 鼠标点击在地图上的坐标
	int nClickMapX, nClickMapY;

	// 绘图窗口
	if (HiEasyX::SetWorkingWindow(hGraphicsWnd))
	{
		HiEasyX::BeginTask();
		while (MouseHit())
		{
			msgGraWnd = getmessage(EM_MOUSE);

			dZoom = *zoom + (msgGraWnd.wheel / 120) * 0.1;

			nClickMapX = (int)((msgGraWnd.x - nMapOutX - nRulerWidth * *zoom - *offset_x) / (nObjSize * *zoom));
			nClickMapY = (int)((msgGraWnd.y - nMapOutY - nRulerHeight * *zoom - *offset_y) / (nObjSize * *zoom));

			// 缩放
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

			// 左键弹起：按钮消息处理
			if (msgGraWnd.message == WM_LBUTTONUP)
			{
				// 帮助按钮
				if (isInRect(msgGraWnd.x, msgGraWnd.y, rctHelpBtn))
				{
					// 中途切换窗口，所以先终止任务
					HiEasyX::EndTask();

					HelpBox();

					// 重新启动任务
					if (!HiEasyX::SetWorkingWindow(hGraphicsWnd))
					{
						goto ToolBar_Begin;
					}
					HiEasyX::BeginTask();
				}
				// 保存
				else if (isInRect(msgGraWnd.x, msgGraWnd.y, rctSaveBtn))
				{
					SaveProject(*map, SelectFile(true));
					flushmessage();
				}
				// 重设地图大小
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

			// 右键按下
			if (msgGraWnd.rbutton)
			{
				if (PointIsInMap(map, nClickMapX, nClickMapY))
				{
					if (msgGraWnd.ctrl)	// Ctrl：旋转元件
					{
						if (++map->map[nClickMapY][nClickMapX].nTowards > 3)
						{
							map->map[nClickMapY][nClickMapX].nTowards = 0;
						}
					}
					else if (*pSelect != RS_NULL)	// 选择元件，放置
					{
						map->map[nClickMapY][nClickMapX].nType = *pSelect;
					}
					return_value |= DM_REDRAWMAP;
				}
			}

			// 右键弹起 或 左键单击
			if (msgGraWnd.message == WM_RBUTTONUP || (msgGraWnd.message == WM_LBUTTONUP && !bMoved))
			{
				if (*pSelect == RS_NULL || msgGraWnd.message == WM_LBUTTONUP)	// 操作元件
				{
					if (PointIsInMap(map, nClickMapX, nClickMapY))
					{
						switch (map->map[nClickMapY][nClickMapX].nType)
						{
						case RS_ROD:
							map->map[nClickMapY][nClickMapX].bPower = !map->map[nClickMapY][nClickMapX].bPower;
							break;

						case RS_BUTTON:
							// 中途启动其他任务，故先终止任务
							HiEasyX::EndTask();

							ClickButton(map, nClickMapX, nClickMapY, *offset_x, *offset_y, *zoom, *p_bShowXY, *p_bShowRuler);

							// 重新启动任务
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

			// 左键拖动：平移
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

			// 左键按下且未按下 Ctrl 才进行平移
			if (msgGraWnd.lbutton && !msgGraWnd.ctrl)
			{
				bGraWndLBtn = true;
				msgGraWndLast = msgGraWnd;
			}
			else
			{
				// 清除
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

// 绘制工具栏
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

// 窗口大小改变的消息处理
void WindowSized()
{
	BEGIN_TASK_WND(hGraphicsWnd);
	int w = getwidth(), h = getheight();
	rctSaveBtn = { 1,0,45,20 };
	rctResizeBtn = { 47,0,107,20 };
	rctHelpBtn = { w - 45 ,0,w - 1,20 };
	END_TASK();
}

// 绘制渐变色矩形
// c 原色
// kh, ks ,kl 色彩的 HSL 单位像素变化率
// 注：H 和 S 的渐变不能实现异色过渡
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

// 绘制渐变色矩形
// c1, c2 初始颜色、终止颜色
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

// 绘制渐变色矩形
// c 初始颜色（只用于 H 和 S 的采样）
// l1, l2 初始亮度、终止亮度（暗 0 ~ 1 亮）
void GradientRectangle(RECT rct, COLORREF c, double l1, double l2)
{
	float h, s, l;
	RGBtoHSL(c, &h, &s, &l);
	GradientRectangle(rct, HSLtoRGB(h, s, (float)l1), 0, 0, (float)(l2 - l1) / (rct.right - rct.left));
}

// 开始界面
void StartMenu(RsMap* pMap)
{
	HWND hStartMenuWnd = HiEasyX::initgraph_win32(640, 480, EW_NORMAL, L"开始");
	DisableResizing(true);

	RECT rctBtn[2] = { {30, 140, 550, 240}, { 30,260,550,360 } };
	BEGIN_TASK_WND(hStartMenuWnd);

	setbkcolor(RGB(40, 50, 60));
	cleardevice();

	settextstyle(38, 0, L"黑体");
	settextcolor(RGB(215, 70, 130));
	WCHAR strTitle[] = L"Minecraft Redstone Simulator";
	int nTitleWidth = textwidth(strTitle);
	outtextxy(30, 40, strTitle);
	settextstyle(20, 0, L"宋体");
	settextcolor(CLASSICGRAY);
	outtextxy(30 + nTitleWidth - textwidth(strVersion), 80, strVersion);
	settextstyle(16, 0, L"宋体");
	settextcolor(GRAY);
	WCHAR strAuthor[] = L"by huidong <mailhuid@163.com>";
	outtextxy(getwidth() - textwidth(strAuthor), getheight() - textheight(strAuthor), strAuthor);

	setfillcolor(RGB(0, 0, 120));
	settextstyle(34, 0, L"宋体");
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
	SetConsoleTitle(L"Minecraft Redstone Simulator 终端");

	loadimages();

	RsMap map = { 0,0,NULL };

	// 如果有文件、参数传入
	if (argc > 1)
	{
		printf("请稍后，打开地图中……");

		TCHAR* strFileName = new TCHAR[strlen(argv[1]) + 1];
		memset(strFileName, 0, strlen(argv[1]) + 1);

		MultiByteToWideChar(CP_ACP, 0, argv[1], -1, strFileName, 1024);

		map = OpenProject(strFileName);
	}
	else
	{
		// 正常进入开始菜单
		StartMenu(&map);
	}

	system("cls");
	printf("地图加载成功。输入指令以操作地图，输入 help 查看帮助。\n");

	// 创建两个窗口
	hGraphicsWnd = HiEasyX::initgraph_win32(1024, 768, EW_SHOWCONSOLE, L"Minecraft Redstone Simulator");
	hToolBarWnd = HiEasyX::initgraph_win32(nObjSize * 2, nObjSize * 4 + 100, EW_NORMAL, L"ToolBar");

	// Tool Bar 的窗口样式
	EnableToolWindowStyle(true);
	DisableSystemMenu(true);
	DisableResizing(true);
	SetWindowPos(hToolBarWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	BEGIN_TASK_WND(hGraphicsWnd);
	setfillcolor(BLUE);
	setbkmode(TRANSPARENT);
	END_TASK();

	bool bShowXY = true;		// 是否显示方块坐标
	bool bShowRuler = true;		// 坐标显示方式
	int offset_x = 0;			// 地图偏移显示
	int offset_y = 0;
	double zoom = 1;			// 地图缩放
	int nSelect = RS_NULL;		// 工具栏选择
	bool bFirst = true;			// 是否为第一次运行

	// 处理用户 CMD 输入
	std::thread(CommandMessageLoop, &map, &offset_x, &offset_y, &zoom, &bShowXY, &bShowRuler).detach();

	// >>>
	// >>> 消息处理
	// >>>
	while (HiEasyX::isAliveWindow(hGraphicsWnd))
	{
		int r = ProcessMouseMsg(&map, &offset_x, &offset_y, &zoom, &bShowXY, &bShowRuler, &nSelect);

		if ((r & DM_REDRAWMAP) || (r & DM_RESIZEMAP) || bFirst)
		{
			RunRsMap(&map);
		}

		// 窗口拉伸消息处理
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
