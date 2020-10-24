///////////////////////////////////////////////
//
//	MCRedstoneSimulator
//	main.cpp
//	我的世界红石模拟器
//	by huidong <mailkey@yeah.net>
//
//	最后修改：2020.10.24
//


#include <easyx.h>
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <direct.h>

// 在debug模式下启动模块计时
#ifdef _DEBUG
double dfq;
LARGE_INTEGER fq, t_begin, t_end;
#define TIMEC_INIT    {QueryPerformanceFrequency(&fq);dfq=1.0/fq.QuadPart;}
#define TIMEC_BEGIN    QueryPerformanceCounter(&t_begin);
#define TIMEC_END    {QueryPerformanceCounter(&t_end);printf("%lf\n",(t_end.QuadPart-t_begin.QuadPart)*dfq);}
#endif

// 版本信息
char m_str_version[] = { "Version 1.3" };

//
//    ___________________________________________________________________
//   / Warning													      |X|\
//   |___________________________________________________________________|
//   |                                                                   |
//   |       /\                                                          |
//   |      / !\    Warning                                              |
//   |     /____\                                                        |
//   |                如果项目有更新，请更新 main.cpp 顶部的注释上的时间，   |
//   |                m_str_version 上的版本，./res/help/help.html 上的    |
//   |                更新日志，修改时间。                                 |
//   |___________________________________________________________________|
//
//

// 全局变量：方块图像（数组下标0表示未充能情况，数组下标为1表示充能情况）

IMAGE m_img_rod[2];		// 拉杆
IMAGE m_img_button[2];	// 按钮
IMAGE m_img_torche[2];	// 红石火把
IMAGE m_img_light[2];	// 红石灯
IMAGE m_img_relay[2];	// 红石中继器
IMAGE m_img_cross;		// 交叉排线板

//
// 交叉排线版他不属于MC方块，只是因为平面电路交叉的需要而设计
// 他自身不存在朝向，能量的概念，只是遇到他的时候要再次判断同个方向的下一个方块是否有电才能决定此方块是否有电
// 举个例子：
//     a
//     |
//  c——n——d
//     |
//     b
//
// n是交叉排线板，如果a有电只能决定b有电，而不影响c,d，以此类推。
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
	RS_CROSS	// 交叉排线板
};

// 红石方块朝向
enum RedstoneTowards
{
	RS_TO_UP,	// 朝向上
	RS_TO_DOWN,	// 朝向下
	RS_TO_LEFT,	// 朝向左
	RS_TO_RIGHT	// 朝向右
};

// 红石方块定义
typedef struct RedstoneObject
{
	int nObj;		// 方块种类
	bool bPower;	// 能量值
	int nTowards;	// 朝向（仅针对中继器）

}RsObj;

// 红石地图
typedef struct RedstoneMap
{
	// 地图宽高
	int w;
	int h;

	// 地图
	RsObj** map;
}RsMap;


// 得到空方块
RsObj GetNullObj()
{
	return RsObj{ RS_NULL ,false ,RS_TO_UP };
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
	loadimage(&m_img_rod[0], L"./res/objs/null/rod/rod.bmp");
	loadimage(&m_img_rod[1], L"./res/objs/power/rod/rod.bmp");

	loadimage(&m_img_button[0], L"./res/objs/null/button/button.bmp");
	loadimage(&m_img_button[1], L"./res/objs/power/button/button.bmp");

	loadimage(&m_img_torche[0], L"./res/objs/null/torche/torche.bmp");
	loadimage(&m_img_torche[1], L"./res/objs/power/torche/torche.bmp");

	loadimage(&m_img_light[0], L"./res/objs/null/light/light.bmp");
	loadimage(&m_img_light[1], L"./res/objs/power/light/light.bmp");

	loadimage(&m_img_relay[0], L"./res/objs/null/relay/relay.bmp");
	loadimage(&m_img_relay[1], L"./res/objs/power/relay/relay.bmp");

	loadimage(&m_img_cross, L"./res/objs/null/cross/cross.bmp");
}

// 判断一物体是否为信号源方块
bool isPowerObj(RsObj obj)
{
	return obj.nObj == RS_ROD || obj.nObj == RS_BUTTON || obj.nObj == RS_TORCHE;
}

// 是否为普通方块（无论是否可导电）（含红石火把）
bool isNormalObj(RsObj obj)
{
	return (obj.nObj != RS_NULL && !isPowerObj(obj)) || obj.nObj == RS_TORCHE;
}

// 是否为可导电方块（包括信号源和普通方块）
bool isConductiveObj(RsObj obj)
{
	return obj.nObj != RS_NULL && obj.nObj != RS_LIGHT;
}

// 判断一个方块的左边是否有电来
bool isLeftPower(RsMap* map, int x, int y)
{
	if (x - 1 >= 0 && map->map[y][x - 1].bPower && isConductiveObj(map->map[y][x - 1]))
	{
		switch (map->map[y][x - 1].nObj)
		{
		case RS_RELAY:
			if (map->map[y][x - 1].nTowards == RS_TO_RIGHT)
			{
				return true;
			}
			break;

		case RS_CROSS:
			if (isLeftPower(map, x - 1, y))
			{
				return true;
			}
			break;

		default:
			return true;
			break;
		}
	}

	return false;
}

// 判断一个方块的右边是否有电来
bool isRightPower(RsMap* map, int x, int y)
{
	if (x + 1 < map->w && map->map[y][x + 1].bPower && isConductiveObj(map->map[y][x + 1]))
	{
		switch (map->map[y][x + 1].nObj)
		{
		case RS_RELAY:
			if (map->map[y][x + 1].nTowards == RS_TO_LEFT)
			{
				return true;
			}
			break;

		case RS_CROSS:
			if (isRightPower(map, x + 1, y))
			{
				return true;
			}
			break;

		default:
			return true;
			break;
		}
	}

	return false;
}

// 判断一个方块的上边是否有电来
bool isUpPower(RsMap* map, int x, int y)
{
	if (y - 1 >= 0 && map->map[y - 1][x].bPower && isConductiveObj(map->map[y - 1][x]))
	{
		switch (map->map[y - 1][x].nObj)
		{
		case RS_RELAY:
			if (map->map[y - 1][x].nTowards == RS_TO_DOWN)
			{
				return true;
			}
			break;

		case RS_CROSS:
			if (isUpPower(map, x, y - 1))
			{
				return true;
			}
			break;

		default:
			return true;
			break;
		}
	}

	return false;
}

// 判断一个方块的下边是否有电来
bool isDownPower(RsMap* map, int x, int y)
{
	if (y + 1 < map->h && map->map[y + 1][x].bPower && isConductiveObj(map->map[y + 1][x]))
	{
		switch (map->map[y + 1][x].nObj)
		{
		case RS_RELAY:
			if (map->map[y + 1][x].nTowards == RS_TO_UP)
			{
				return true;
			}
			break;

		case RS_CROSS:
			if (isDownPower(map, x, y + 1))
			{
				return true;
			}
			break;

		default:
			return true;
			break;
		}
	}

	return false;
}

// 根据周围方块的情况判断我是否应该有电
bool isMePower(RsMap* map, int x, int y)
{
	switch (map->map[y][x].nObj)
	{
	case RS_RELAY:

		switch (map->map[y][x].nTowards)
		{
		case RS_TO_RIGHT:
			if (isLeftPower(map, x, y))
			{
				return true;
			}
			break;

		case RS_TO_LEFT:
			if (isRightPower(map, x, y))
			{
				return true;
			}
			break;

		case RS_TO_UP:
			if (isDownPower(map, x, y))
			{
				return true;
			}
			break;

		case RS_TO_DOWN:
			if (isUpPower(map, x, y))
			{
				return true;
			}
			break;
		}

		break;

	default:
		if (
			isLeftPower(map, x, y) ||
			isRightPower(map, x, y) ||
			isUpPower(map, x, y) ||
			isDownPower(map, x, y)
			)
		{
			return true;
		}
		break;
	}

	return false;
}

// 执行关于普通方块的处理（含红石火把）
void RunObj(RsMap* map, bool**& pPass, int x, int y)
{
	RsObj* me = &map->map[y][x];

	// 已经访问过，不再重复访问，防止造成递归死循环（CROSS除外，即使被访问过也可以继续访问）
	if (pPass[y][x] && me->nObj != RS_CROSS)
		return;

	// 为什么CROSS即使被访问过也可以继续访问？
	// 访问记录的目的是防止环形电路导致的递归死循环，但是cross方块不能异向导电，所以不能构成环形电路，故不需要停止RunObj函数。
	// 
	// 这里有更详细的解释：
	/*
	举个例子，如果cross方块被访问过也一样要return的话：

				 L1
				 |
				 |
		 ------CROSS-----L2
		 |       |
		 |       |
		rod1	rod2

	在上面的电路情况中，如果rod1和rod2都开启，那么RunRsMap函数中，会遍历地图，先执行rod1的处理。
	rod1的处理结果是，rod1到L2的线路被充能，L2亮起。

	然后等到执行到rod2的处理时，会和rod1一样，调用runobj递归处理这条线路，但是运行到cross处时，
	再次向不同方向执行runobj函数递归前，cross的访问记录已经被标记为true了，所以接下来的递归都以失败告终，
	所以最终会导致这样一个结果：

	rod2到L1的线路中，rod2到cross段是充能的，但是cross到L1段是不充能的，就会导致L1不亮，L2在rod1的影响下还是亮的。

	*/

	// 如果有能量传给我
	if (isMePower(map, x, y))
	{
		// 如果是红石火把被传了信号，那么直接熄灭。
		if (me->nObj == RS_TORCHE)
		{
			me->bPower = false;
			return;
		}

		// 其它方块的话就被充能
		else
		{
			me->bPower = true;
		}
	}

	// 如果没有能量传给我
	else
	{
		// 如果是红石火把没被传信号，那么可以亮起来
		if (me->nObj == RS_TORCHE)
		{
			me->bPower = true;
		}

		// 其他方块就暗掉
		else
		{
			me->bPower = false;
		}

		return;
	}

	// 登记方块的访问记录
	pPass[y][x] = true;

	// 判断四周是否可以继续传输信号，并往下传输。
	if (x - 1 >= 0 && isNormalObj(map->map[y][x - 1]))
	{
		RunObj(map, pPass, x - 1, y);
	}

	if (x + 1 < map->w && isNormalObj(map->map[y][x + 1]))
	{
		RunObj(map, pPass, x + 1, y);
	}

	if (y - 1 >= 0 && isNormalObj(map->map[y - 1][x]))
	{
		RunObj(map, pPass, x, y - 1);
	}

	if (y + 1 < map->h && isNormalObj(map->map[y + 1][x]))
	{
		RunObj(map, pPass, x, y + 1);
	}
}

// 执行关于能量源的处理
void RunPower(RsMap* map, bool**& pPass, int x, int y)
{
	// 每次处理信号源前都将访问记录清空
	for (int i = 0; i < map->h; i++)
		for (int j = 0; j < map->w; j++)
			pPass[i][j] = false;

	pPass[y][x] = true;

	// 信号源中，只有红石火把会接收外部信号输入，如果有外部信号输入，则火把熄灭
	if (map->map[y][x].nObj == RS_TORCHE)
	{
		if (isMePower(map, x, y))
		{
			map->map[y][x].bPower = false;
			return;
		}
	}

	if (x - 1 >= 0 && isNormalObj(map->map[y][x - 1]))
	{
		RunObj(map, pPass, x - 1, y);
	}

	if (x + 1 < map->w && isNormalObj(map->map[y][x + 1]))
	{
		RunObj(map, pPass, x + 1, y);
	}

	if (y - 1 >= 0 && isNormalObj(map->map[y - 1][x]))
	{
		RunObj(map, pPass, x, y - 1);
	}

	if (y + 1 < map->h && isNormalObj(map->map[y + 1][x]))
	{
		RunObj(map, pPass, x, y + 1);
	}
}

// 运行红石地图
// repeat 表示地图运行循环次数，可不填。循环可避免一些bug的出现，最好是3次。
void RunRsMap(RsMap* map, int repeat = 3)
{
	// debug模式下启动模块计时
#ifdef _DEBUG
	TIMEC_BEGIN;
#endif

	// 存储地图中每个点的访问记录
	bool** pPass = new bool* [map->h];
	for (int i = 0; i < map->h; i++)
	{
		pPass[i] = new bool[map->w];
		for (int j = 0; j < map->w; j++)
			pPass[i][j] = false;
	}

	for (int n = 0; n < repeat; n++)
	{
		// 首先使所有不是电源的方块都没电
		for (int i = 0; i < map->w; i++)
		{
			for (int j = 0; j < map->h; j++)
			{
				if (!isPowerObj(map->map[j][i]))
				{
					map->map[j][i].bPower = false;
				}
			}
		}

		for (int i = 0; i < map->w; i++)
		{
			for (int j = 0; j < map->h; j++)
			{
				if (isPowerObj(map->map[j][i]))
				{
					RunPower(map, pPass, i, j);
				}
			}
		}

		// 遍历地图，做后续工作
		for (int i = 0; i < map->w; i++)
		{
			for (int j = 0; j < map->h; j++)
			{
				// 如果我周围无能量
				if (!isMePower(map, i, j) && isNormalObj(map->map[j][i]))
				{
					// 有个特殊情况是红石火把，如果他没被充能，就把他的信号恢复成true
					if (map->map[j][i].nObj == RS_TORCHE)
					{
						map->map[j][i].bPower = true;
					}

					// 普通方块则变成无信号的
					else
					{
						map->map[j][i].bPower = false;
					}
				}
			}
		}
	}

	// 释放所有内存
	for (int i = 0; i < map->h; i++)
		delete[] pPass[i];
	delete[] pPass;
	pPass = NULL;

	// debug模式下启动模块计时
#ifdef _DEBUG
	printf("执行地图用时：");
	TIMEC_END;
#endif
}

// 放置物品到地图
void PutObjectToRsMap(RsMap* map, int x, int y, int object_id, int direction = RS_TO_UP)
{
	RsObj obj = GetNullObj();
	obj.nObj = object_id;

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
	ofn.lpstrDefExt = L"rsp";	// 指向包含默认扩展名的缓冲。如果用户忘记输入扩展名，GetOpenFileName和GetSaveFileName附加这个扩展名到文件名中。
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
	return ch >= '0' && ch <= '9';
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
void ReSizeRsMap(RsMap* map, int w, int h)
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
			_itoa_s(map.map[i][j].nObj, chObj, 10);
			_itoa_s(map.map[i][j].nTowards, chTowards, 10);
			if (map.map[i][j].bPower && map.map[i][j].nObj != RS_BUTTON)
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
	/*
	地图宽 地图高
	物体id,朝向,能量 物体id,朝向,能量 ......
	物体id,朝向,能量 物体id,朝向,能量 ......
	......
	物体id,朝向,能量 物体id,朝向,能量 ......
	*/

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
			int nObj = RS_NULL;
			int nTowards = RS_TO_UP;
			int nPower = false;

			// 读物品id
			if (!ReadNum(chProject, index, nObj))
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
			rsobj.nObj = nObj;
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
// x,y 被导入项目位于主项目的左上角坐标
void ImportProject(RsMap* out, RsMap in, int x, int y)
{
	for (int jo = y, ji = 0; jo < out->h && ji < in.h; jo++, ji++)
	{
		for (int io = x, ii = 0; io < out->w && ii < in.w; io++, ii++)
		{
			out->map[jo][io] = in.map[ji][ii];
		}
	}
}

// 开始界面
RsMap StartMenu()
{
	printf("MCRedstoneSimulator 作者：huidong <mailkey@yeah.net>\n");
	printf("%s\n", m_str_version);
	printf("打开项目（按 O） 或 新建项目（按 N）？");

	while (true)
	{
		if (GetKeyState('O') & 0x80)
		{
			printf("\n加载地图中……请稍候。\n");
			return OpenProject(SelectFile());
		}

		else if (GetKeyState('N') & 0x80)
		{
			int w = 0, h = 0;
		again:
			printf("\n地图大小（输入宽后回车，再输入高再回车）：\n");
			scanf_s("%d", &w);
			scanf_s("%d", &h);

			if (w <= 0 || h <= 0)
			{
				printf("地图大小不得小于等于0，请重输。");
				goto again;
			}

			return InitRsMap(w, h);
		}

		Sleep(10);

		// 清空键盘缓冲区
		FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	}

	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
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

// 得到地图画面
// offset_x,y 指定地图画面偏移
// zoom 指定缩放大小
// bShowXY 指定是否显示坐标
// bShowRuler 指定显示坐标时的方式，为true表示显示标尺，为false表示直接在方块上显示坐标
IMAGE* GetRsMapImage(RsMap* map, int offset_x, int offset_y, double zoom, bool bShowXY, bool bShowRuler)
{
	// debug模式下启动模块计时
#ifdef _DEBUG
	TIMEC_BEGIN;
#endif

	const double PI = 3.141592653589793846;
	const double DEGREE = PI / 180;

	int nObjSize = m_img_light[0].getwidth();

	IMAGE* imgMap = new IMAGE;
	imgMap->Resize(map->w * nObjSize, map->h * nObjSize);

	IMAGE* pOld = GetWorkingImage();
	int nScreen_w = (int)(getwidth() / zoom);
	int nScreen_h = (int)(getheight() / zoom);

	SetWorkingImage(imgMap);
	settextcolor(WHITE);
	settextstyle(10, 0, L"黑体");
	setbkmode(TRANSPARENT);

	// 判断xy坐标是否超出屏幕
#define x_overscreen(x) ((x + 1) * nObjSize + offset_x) * zoom < 0 || (x * nObjSize + offset_x) * zoom > nScreen_w
#define y_overscreen(y) ((y + 1) * nObjSize + offset_y) * zoom < 0 || (y * nObjSize + offset_y) * zoom > nScreen_h

	for (int x = 0; x < map->w; x++)
	{
		// 若x坐标在屏幕范围外，不绘制此点
		if (x_overscreen(x))
			continue;

		for (int y = 0; y < map->h; y++)
		{
			// 若y坐标在屏幕范围外，不绘制此点
			if (y_overscreen(y))
				continue;

			IMAGE powder(nObjSize, nObjSize);	// 实时绘制红石粉的画布
			IMAGE relay(nObjSize, nObjSize);	// 实时绘制红石中继器的画布
			RsObj me = map->map[y][x];
			RsObj up, down, left, right;

			if (y - 1 >= 0)
				up = map->map[y - 1][x];
			if (y + 1 < map->h)
				down = map->map[y + 1][x];
			if (x - 1 >= 0)
				left = map->map[y][x - 1];
			if (x + 1 < map->w)
				right = map->map[y][x + 1];

			switch (me.nObj)
			{
			case RS_NULL:
				break;

			case RS_POWDER:

				SetWorkingImage(&powder);

				if (me.bPower)
				{
					setfillcolor(RGB(255, 0, 0));
					setlinecolor(RGB(255, 0, 0));
				}

				else
				{
					setfillcolor(RGB(100, 0, 0));
					setlinecolor(RGB(100, 0, 0));
				}

				fillcircle(nObjSize / 2, nObjSize / 2, 5);
				setlinestyle(PS_SOLID, 12);

				// 实时绘制红石粉
				if (y - 1 >= 0 && up.nObj != RS_NULL)
					line(nObjSize / 2, nObjSize / 2, nObjSize / 2, 0);	// line to up

				if (y + 1 < map->h && down.nObj != RS_NULL)
					line(nObjSize / 2, nObjSize / 2, nObjSize / 2, nObjSize);	// line to down

				if (x - 1 >= 0 && left.nObj != RS_NULL)
					line(nObjSize / 2, nObjSize / 2, 0, nObjSize / 2);	// line to left

				if (x + 1 < map->w && right.nObj != RS_NULL)
					line(nObjSize / 2, nObjSize / 2, nObjSize, nObjSize / 2);	// line to right

				SetWorkingImage(imgMap);

				putimage(x * nObjSize, y * nObjSize, &powder);

				break;

			case RS_ROD:
				putimage(x * nObjSize, y * nObjSize, &m_img_rod[me.bPower]);
				break;

			case RS_BUTTON:
				putimage(x * nObjSize, y * nObjSize, &m_img_button[me.bPower]);
				break;

			case RS_TORCHE:
				putimage(x * nObjSize, y * nObjSize, &m_img_torche[me.bPower]);
				break;

			case RS_LIGHT:
				putimage(x * nObjSize, y * nObjSize, &m_img_light[me.bPower]);
				break;

			case RS_RELAY:

				// 旋转中继器
				switch (me.nTowards)
				{
				case RS_TO_UP:
					relay = m_img_relay[me.bPower];
					break;
				case RS_TO_LEFT:
					rotateimage(&relay, &m_img_relay[me.bPower], DEGREE * 90);
					break;
				case RS_TO_DOWN:
					rotateimage(&relay, &m_img_relay[me.bPower], DEGREE * 180);
					break;
				case RS_TO_RIGHT:
					rotateimage(&relay, &m_img_relay[me.bPower], DEGREE * 270);
					break;
				}

				putimage(x * nObjSize, y * nObjSize, &relay);

				break;

			case RS_CROSS:
				putimage(x * nObjSize, y * nObjSize, &m_img_cross);
				break;
			}

			// 输出坐标
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

	// -----绘制标尺，网格起始点

	// 标尺宽高
	const int nRulerWidth = 20;
	const int nRulerHeight = 20;

	// xy轴标尺
	IMAGE* imgXRuler = new IMAGE(getwidth(), nRulerHeight);
	IMAGE* imgYRuler = new IMAGE(nRulerWidth, getheight());

	// 绘制网格，标尺
	if (bShowXY && bShowRuler)
	{
		// 网格线形
		setlinecolor(WHITE);
		setlinestyle(PS_DASH, 1);

		// 网格
		for (int x = 0; x < map->w; x++)
		{
			// 若x坐标在屏幕范围外，不绘制此点
			if (x_overscreen(x))
				continue;

			line(x * nObjSize, 0, x * nObjSize, getheight());
		}
		for (int y = 0; y < map->h; y++)
		{
			// 若y坐标在屏幕范围外，不绘制此点
			if (y_overscreen(y))
				continue;

			line(0, y * nObjSize, getwidth(), y * nObjSize);
		}

		// x轴标尺
		SetWorkingImage(imgXRuler);
		setlinecolor(WHITE);
		settextcolor(WHITE);
		settextstyle(12, 0, L"黑体");
		setbkmode(TRANSPARENT);
		setbkcolor(BLUE);
		cleardevice();
		rectangle(0, 0, getwidth(), getheight());

		for (int x = 0; x < map->w; x++)
		{
			// 若x坐标在屏幕范围外，不绘制此点
			if (x_overscreen(x))
				continue;

			line(x * nObjSize, 0, x * nObjSize, getheight());
			TCHAR str[6] = { 0 };
			wsprintf(str, L"%d", x);
			outtextxy(x * nObjSize + 5, 5, str);
		}

		// y轴标尺
		SetWorkingImage(imgYRuler);
		setlinecolor(WHITE);
		settextcolor(WHITE);
		settextstyle(10, 0, L"黑体");
		setbkmode(TRANSPARENT);
		setbkcolor(BLUE);
		cleardevice();
		rectangle(0, 0, getwidth(), getheight());

		for (int y = 0; y < map->h; y++)
		{
			// 若y坐标在屏幕范围外，不绘制此点
			if (y_overscreen(y))
				continue;

			line(0, y * nObjSize, getwidth(), y * nObjSize);
			TCHAR str[6] = { 0 };
			wsprintf(str, L"%d", y);
			outtextxy(5, y * nObjSize + 5, str);
		}
	}

	// 存储移动、缩放过后的图像
	IMAGE* imgMap_offset = new IMAGE(nScreen_w, nScreen_h);
	SetWorkingImage(imgMap_offset);

	// 由于标尺的出现，需要将图像偏移加上标尺的大小
	offset_x += imgYRuler->getwidth();
	offset_y += imgXRuler->getheight();
	putimage(offset_x, offset_y, imgMap);

	// 将绘制好的标尺putimage到画布
	if (bShowXY && bShowRuler)
	{
		putimage(offset_x, 0, imgXRuler);
		putimage(0, offset_y, imgYRuler);
	}

	// -----绘制标尺，网格结束点

	// 缩放
	ImageToSize((int)(getwidth() * zoom), (int)(getheight() * zoom), imgMap_offset);
	delete imgMap;

	SetWorkingImage(pOld);

	// debug模式下启动模块计时
#ifdef _DEBUG
	printf("生成图像用时：");
	TIMEC_END;
#endif

	return imgMap_offset;
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

	int nMaxCmdSize = strlen(cmd) + 1;
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

// 从字符串转换成ID号
// str 表示原字符串
// type_out 表示ID类型，为0表示物品，为1表示朝向
// id_out 表示ID
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
void ProcessingImage(RsMap* map, int offset_x, int offset_y, double zoom, bool bShowXY, bool bShowRuler)
{
	IMAGE* img = GetRsMapImage(map, offset_x, offset_y, zoom, bShowXY, bShowRuler);
	cleardevice();
	putimage(0, 0, img);
	delete img;
}

// 点击一个按钮（阻塞）
void ClickButton(RsMap* map, int x, int y, int offset_x, int offset_y, double zoom, bool bShowXY, bool bShowRuler)
{
	int delay = 2000;

	if (map->map[y][x].nObj == RS_BUTTON)
	{
		map->map[y][x].bPower = true;
		RunRsMap(map);

		ProcessingImage(map, offset_x, offset_y, zoom, bShowXY, bShowRuler);	// 手动重绘

		Sleep(delay);
		map->map[y][x].bPower = false;
		RunRsMap(map);

		ProcessingImage(map, offset_x, offset_y, zoom, bShowXY, bShowRuler);
	}
}

// 得到排序好且合法的坐标（针对两个坐标使用）
void GetSortingPoint(RsMap* map, int* x1, int* y1, int* x2, int* y2)
{
	// 确保x1 <= x2, y1 <= y2
	if (*x1 > * x2)
	{
		int temp = *x1;
		*x1 = *x2;
		*x2 = temp;
	}
	if (*y1 > * y2)
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
void ProcessingCommand(RsMap* map, int* offset_x, int* offset_y, double* zoom, bool* p_bShowXY, bool* p_bShowRuler)
{
	// 地图移动的单位大小（像素）
	int offset_unit_size = 10;

	// 命令最长长度
	int nMaxInputdSize = 1024;
	char* chCmd = new char[nMaxInputdSize];
	memset(chCmd, 0, nMaxInputdSize);

	gets_s(chCmd, nMaxInputdSize);

	// 参数数量
	int nArgsNum;
	char** chCmdsArray;

	// 解析参数
	GetArguments(chCmd, &chCmdsArray, &nArgsNum);

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

		if (map->map[y][x].nObj != RS_NULL)
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
			if (map->map[y][x].nObj == RS_NULL)
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

		switch (map->map[y][x].nObj)
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

		ReSizeRsMap(map, w, h);
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

	// import x y
	else if (nArgsNum == 3 && strcmp(chCmdsArray[0], "import") == 0 && isAllNum(chCmdsArray[1]) && isAllNum(chCmdsArray[2]))
	{
		int x = atoi(chCmdsArray[1]);
		int y = atoi(chCmdsArray[2]);

		if (!PointIsInMap(map, x, y))
		{
			printf("坐标越界。\n");
			return;
		}

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
		MultiByteToWideChar(CP_ACP, 0, warning, strlen(warning), wstr, strlen(warning));

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
		char cmd[64] = {0};
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

int main(int argc, char* argv[])
{
	// debug模式下启动模块计时
#ifdef _DEBUG
	TIMEC_INIT;
#endif

	SetConsoleTitle(L"MCRedstoneSimulator 终端");

	// 加载图像
	loadimages();

	RsMap map;

	// 如果有文件/参数传入
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
		map = StartMenu();
	}

	system("cls");
	system("mode con cols=120 lines=40");
	printf("地图加载成功。输入指令以操作地图，输入help查看帮助。\n");

	initgraph(1024, 768, EW_SHOWCONSOLE);
	SetWindowPos(GetConsoleWindow(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	SetWindowText(GetHWnd(), L"MCRedstoneSimulator 模拟器");

	// 是否显示方块坐标
	bool bShowXY = true;

	// 坐标显示方式
	bool bShowRuler = true;

	// 地图偏移显示
	int offset_x = 0;
	int offset_y = 0;

	// 地图缩放
	double zoom = 1;

	// 处理用户输入
	while (true)
	{
		RunRsMap(&map);
		ProcessingImage(&map, offset_x, offset_y, zoom, bShowXY, bShowRuler);
		ProcessingCommand(&map, &offset_x, &offset_y, &zoom, &bShowXY, &bShowRuler);
	}

	closegraph();
	return 0;
}
