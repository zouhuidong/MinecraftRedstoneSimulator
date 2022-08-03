#pragma once

#include <Windows.h>

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

struct Power
{
	bool isPowerOfCross = false;	// 是否为交叉线的电源

	// 规定：如果同一个电源供给交叉线的两个方向，则应当存储两个电源，分别标识为水平电源和竖直电源
	bool horizon = false;			// 是交叉线的水平方向电源（仅用于交叉线）
	bool upright = false;			// 是交叉线的垂直方向电源（仅用于交叉线）

	int x = -1, y = -1;

	Power()
	{
	}

	Power(POINT pt)
	{
		x = pt.x;
		y = pt.y;
	}
};

// 红石方块定义
typedef struct RedstoneObject
{
	int nType = RS_NULL;				// 方块种类
	int nTowards = RS_TO_UP;			// 朝向

	bool bPowered = false;				// 是否充能（对于交叉线，任意方向充能都会标识）

	bool bUprightPowered = false;		// 竖直方向是否充能（仅用于交叉线）
	bool bHorizonPowered = false;		// 水平方向是否充能（仅用于交叉线）

	bool bHaveUprightPower = false;		// 竖直方向是否存在供电源（仅用于交叉线）
	bool bHaveHorizonPower = false;		// 水平方向是否存在供电源（仅用于交叉线）

	int nPowerCount = 0;				// 供电数量
	Power* pPowerList = NULL;			// 供电电源坐标

}RsObj;

// 红石地图
typedef struct RedstoneMap
{
	int w, h;		// 地图宽高
	RsObj** map;	// 地图
}RsMap;


// 初始化红石地图
RsMap InitRsMap(int w, int h);

// 删除地图
void DeleteRsMap(RsMap* map);

// 重设地图大小
void ResizeRsMap(RsMap* map, int w, int h);

// 运行红石地图
void RunRsMap(RsMap* pMap);
