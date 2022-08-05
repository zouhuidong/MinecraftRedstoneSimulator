///////////////////////////////////////////////
//
//	MCRedstoneSimulator
//	我的世界红石模拟器
// 
//	by huidong <mailkey@yeah.net>
//
//	最后修改：2022.08.05
//

#include "resource.h"

#include "HiEasyX.h"
#include "map_operation.h"
#include "painter.h"

#include <stdio.h>
#include <conio.h>
#include <direct.h>


// 版本信息
const WCHAR strVersion[] = L"Version 2.0";

// 缩放限制
#define MAX_ZOOM 2.0
#define MIN_ZOOM 0.1

// 全局按钮位置信息
RECT rctHelpBtn;
RECT rctSaveBtn;
RECT rctResizeBtn;

// 窗口句柄
HWND hGraphicsWnd;	// 主绘图窗口
HWND hToolBarWnd;	// 工具栏窗口


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
 *    参考自http://tieba.baidu.com/pos/5218523817
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
			byte r = GetRValue(oldDr[xt + yt * pImg->getwidth()]);
			byte g = GetGValue(oldDr[xt + yt * pImg->getwidth()]);
			byte b = GetBValue(oldDr[xt + yt * pImg->getwidth()]);
			newDr[i * newWidth + j] = RGB(r, g, b);
		}
	}

	return newImg;
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
	POINT* pChange,			// 地图变更点
	int nChangeCount,		// 变更数量
	bool redraw,			// 是否重绘（依据变更点重绘）
	bool resize,			// 地图尺寸是否更新
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
		GetRsMapImage(&imgMap, &imgRulerX, &imgRulerY, map, pChange, nChangeCount, redraw, resize, bShowXY, bShowRuler);
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
		// 模拟按下再弹起
		for (int i = 0; i <= 1; i++)
		{
			RsMap old;
			POINT* pChange = nullptr;
			int nChangeCount = 0;
			CopyRsMap(&old, map);

			map->map[y][x].bPowered = !i;
			RunRsMap(map);

			CmpRsMapForRedraw(&old, map, &pChange, &nChangeCount);
			Render(map, pChange, nChangeCount, true, false, false, offset_x, offset_y, zoom, bShowXY, bShowRuler);	// 手动重绘

			delete[] pChange;
			pChange = nullptr;

			if (!i)
			{
				Sleep(delay);
			}
		}
	}
}

// 判读一字符串是否完全为数字
bool isAllNum(const char* str)
{
	for (int i = 0; i < (int)strlen(str); i++)
		if (!isNum(str[i]))
			return false;
	return true;
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
	static char** chCmdsArray = nullptr;

	// 清理上一次的内存
	if (chCmdsArray != nullptr)
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
			map->map[y][x].bPowered = !map->map[y][x].bPowered;
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
		Resize(nullptr, w, h);
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
	HWND hHelpWnd = HiEasyX::initgraph_win32(w, h, EW_NORMAL, L"帮助", nullptr, hGraphicsWnd);
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
			hWnd, (HMENU)IDC_EDIT1, hInstance, 0);
		hEdit[1] = CreateWindow(L"edit", strMapSize[1],
			WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER,
			60, 55, 40, 20,
			hWnd, (HMENU)IDC_EDIT2, hInstance, 0);
		hBtn = CreateWindow(L"button", L"Done",
			WS_CHILD | WS_VISIBLE | ES_LEFT | WS_BORDER,
			160, 40, 60, 30,
			hWnd, (HMENU)IDC_BUTTON, hInstance, 0);
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
	DM_DRAWMAP = 2,		// 绘制地图（通常为平移消息）
	DM_REDRAWMAP = 4,	// 重新绘制地图（此消息通常在操作方块时发出）
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
							map->map[nClickMapY][nClickMapX].bPowered = !map->map[nClickMapY][nClickMapX].bPowered;
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

						// 操作非空方块，重绘
						if (map->map[nClickMapY][nClickMapX].nType != RS_NULL)
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
						if (map->map[nClickMapY][nClickMapX].nType != RS_NULL)
						{
							map->map[nClickMapY][nClickMapX].nType = RS_NULL;
							map->map[nClickMapY][nClickMapX].bPowered = false;
							return_value |= DM_REDRAWMAP;
						}
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
void DrawToolBar(int indexSelected)
{
	BEGIN_TASK_WND(hToolBarWnd);

	DrawToolBar_painter(indexSelected);

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

	RsMap map;

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

	ShowWindow(GetConsoleWindow(), SW_SHOW);

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
	// 最好不要再用 CMD，很多功能不再维护
	std::thread(CommandMessageLoop, &map, &offset_x, &offset_y, &zoom, &bShowXY, &bShowRuler).detach();

	// >>>
	// >>> 消息处理
	// >>>

	RsMap mapOld;					// 保存上一次的地图数据
	POINT* pChange = nullptr;		// 变更点
	int nChangeCount = 0;			// 变更数量统计
	while (HiEasyX::isAliveWindow(hGraphicsWnd))
	{
		int r = ProcessMouseMsg(&map, &offset_x, &offset_y, &zoom, &bShowXY, &bShowRuler, &nSelect);
		bool wnd_resize = false;

		// 需要重新运行地图的情况
		if ((r & DM_REDRAWMAP) || (r & DM_RESIZEMAP) || bFirst)
		{
			RunRsMap(&map);

			int size = map.w * map.h;		// 地图大小

			// 全部重绘的情况
			if (r & DM_RESIZEMAP || bFirst)
			{
				nChangeCount = size;		// 标记数量为地图面积时，即标识全部重绘，无需设置数组内容
			}

			// 其余情况部分重绘。需要检测变更点
			else
			{
				CmpRsMapForRedraw(&mapOld, &map, &pChange, &nChangeCount);

			}

#ifdef _DEBUG
			printf("需要重绘方块数：%d\n", nChangeCount);
#endif
			CopyRsMap(&mapOld, &map);
		}

		// 窗口拉伸消息处理
		if (HiEasyX::isWindowSizeChanged(hGraphicsWnd) || bFirst)
		{
			WindowSized();
			wnd_resize = true;
		}

		if ((r & DM_TOOLBAR) || bFirst)
		{
			DrawToolBar(nSelect);
		}

		if (wnd_resize
			|| (r & DM_DRAWMAP)
			|| (r & DM_REDRAWMAP)
			|| (r & DM_RESIZEMAP)
			|| (r & DM_ZOOM)
			|| bFirst)
		{
			Render(
				&map,
				pChange,
				nChangeCount,
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

		if (pChange)
		{
			delete[] pChange;
			pChange = nullptr;
		}

		Sleep(50);
	}

	closegraph();
	return 0;
}
