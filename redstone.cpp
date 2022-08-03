#include "redstone.h"

#include "HiEasyX/HiFunc.h"

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
	}

	return map;
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


bool operator==(POINT a, POINT b)
{
	return a.x == b.x && a.y == b.y;
}

// 查找供电表中是否有某个电源
bool SearchPowerInList(Power* pPowerList, int nCount, Power pPower)
{
	for (int i = 0; i < nCount; i++)
	{
		bool xy_equal = pPowerList[i].x == pPower.x && pPowerList[i].y == pPower.y;

		if (xy_equal)
		{
			if (pPowerList[i].isPowerOfCross)
			{
				if (pPowerList[i].horizon == pPower.horizon
					&& pPowerList[i].upright == pPower.upright)
					return true;
			}
			else
				return true;
		}
	}
	return false;
}

// 添加多个供电源到供电表
// 返回添加成功的个数（即与原表不重复的项的个数）
int AddToPowerList(RsObj* pObj, Power* pPowerList, int nCount)
{
	Power* pNewList = new Power[pObj->nPowerCount + nCount];
	int sum = 0;	// 去重后加入表中的数量
	if (!pNewList)
	{
		return 0;
	}
	if (pObj->pPowerList != NULL)
	{
		memcpy(pNewList, pObj->pPowerList, pObj->nPowerCount * sizeof(Power));
		for (int i = 0; i < nCount; i++)
		{
			if (!SearchPowerInList(pNewList, pObj->nPowerCount + sum, pPowerList[i]))
			{
				pNewList[pObj->nPowerCount + sum] = pPowerList[i];
				sum++;
			}
		}
		pObj->nPowerCount += sum;
		delete[] pObj->pPowerList;
	}
	else
	{
		memcpy(pNewList, pPowerList, nCount * sizeof(Power));
		pObj->nPowerCount = nCount;
		sum = nCount;
	}

	pObj->pPowerList = pNewList;
	return sum;

}

// 判断一物体是否为信号源方块
bool isPowerObj(RsObj* obj)
{
	return obj->nType == RS_ROD || obj->nType == RS_BUTTON || obj->nType == RS_TORCHE;
}

// 是否为普通方块（含一切可被充能的实体方块，包括红石火把，不包括其余电源方块）
bool isNormalObj(RsObj* obj)
{
	return (obj->nType != RS_NULL && !isPowerObj(obj)) || obj->nType == RS_TORCHE;
}

// 是否为可导电方块（筛选方式 1）
// 包含一切实体方块（除了红石灯，包括所有电源）
// 即自身可以发电，或可以继续导电的方块
bool isConductiveObj(RsObj* obj)
{
	return obj->nType != RS_NULL && obj->nType != RS_LIGHT;
}

// 是否为可导电方块（筛选方式 2）
// 包含除了灯、电源以外的实体方块
// 即可以继续导电的方块
// 和筛选方式 1 的区别在于，电源不算
bool isConductiveObj2(RsObj* obj)
{
	return obj->nType != RS_NULL && obj->nType != RS_LIGHT && !isPowerObj(obj);
}

void SetPowerRelation(RsMap*, int, int, Power);

// 可充能方块连接到电源
// x, y		当前方块位置
// cx, cy	给电体位置（按理必须紧挨当前方块）
// power	发起电源位置
// 返回是否连接成功。若之前连接过此电源，则返回 false
bool AssociateWithPower(RsMap* pMap, int x, int y, int cx, int cy, Power power)
{
	RsObj& obj = pMap->map[y][x];		// 当前方块
	RsObj cobj = pMap->map[cy][cx];		// 给电方块
	int dx = cx - x, dy = cy - y;		// 给电方块相对当前方块的位置

	bool associated = false;			// 标识是否成功连接到电源

	// 根据当前方块类型，约束给电类型
	switch (obj.nType)
	{
		// 红石火把只接受中继器供能
	case RS_TORCHE:
	{
		if (cobj.nType != RS_RELAY)		// 中继器方向不会出错，因为传导时就已经限制了传导方向
		{
			return false;
		}
	}
	break;

	// 中继器只接受同向供能
	case RS_RELAY:
	{
		switch (obj.nTowards)			// 若给电方向不对，则连接失败
		{
		case RS_TO_UP:		if (!(dx == 0 && dy == 1))	return false;	break;
		case RS_TO_DOWN:	if (!(dx == 0 && dy == -1))	return false;	break;
		case RS_TO_LEFT:	if (!(dx == 1 && dy == 0))	return false;	break;
		case RS_TO_RIGHT:	if (!(dx == -1 && dy == 0))	return false;	break;
		}
	}
	break;
	}

	// 根据给电体类型，做出不同的判断
	switch (cobj.nType)
	{
		// 对于 CROSS 需要判断来电方向
	case RS_CROSS:
		if (dx)			associated = cobj.bHaveHorizonPower;
		else if (dy)	associated = cobj.bHaveUprightPower;
		break;

		// 其余方块可以直接标记
	default:
		associated = true;
		break;
	}

	// 连接电源
	if (associated)
	{
		Power mypower = power;

		if (obj.nType == RS_CROSS)	// 对于交叉线，还必须记录更详细的电源信息
		{
			bool horizon = dx;		// 记录电源方向
			bool upright = dy;

			obj.bHaveHorizonPower = horizon;	// 在交叉线中记录：具有某个方向的电源
			obj.bHaveUprightPower = upright;

			mypower.isPowerOfCross = true;		// 在电源中记录：这是交叉线的某个方向的电源
			mypower.horizon = horizon;
			mypower.upright = upright;
		}

		associated = AddToPowerList(&obj, &mypower, 1);
	}

	return associated;
}

// 把电源和可充能方块联系起来（含红石火把）
// x, y		当前方块位置
// cx, cy	给电体位置（按理必须紧挨当前方块）
// power	发起电源位置
void AssociatePowerToObj(RsMap* pMap, int x, int y, int cx, int cy, Power power)
{
	RsObj* pObj = &pMap->map[y][x];

	// 此方块必须可被充能
	if (!isNormalObj(pObj))
		return;

	// 将传入的电源与此导体联系
	if (AssociateWithPower(pMap, x, y, cx, cy, power))
	{
		// 如果是可以继续传导的物体，则继续
		if (isConductiveObj2(pObj))
		{
			// 继续传导电源关系
			SetPowerRelation(pMap, x, y, power);
		}
	}
}

// 设置电源和导体的连接关系
// power	电源位置
void SetPowerRelation(RsMap* pMap, int x, int y, Power power)
{
	// 向四周的传导方向
	POINT pConduct[4] = { {-1,0},{1,0},{0,-1},{0,1} };

	// 索引起始和结束点
	int begin = 0, end = 0;

	switch (pMap->map[y][x].nType)
	{
		// 中继器只向一个方向导电
	case RS_RELAY:
	{
		switch (pMap->map[y][x].nTowards)
		{
		case RS_TO_UP:		begin = 2;	break;
		case RS_TO_DOWN:	begin = 3;	break;
		case RS_TO_LEFT:	begin = 0;	break;
		case RS_TO_RIGHT:	begin = 1;	break;
		}
		end = begin;
	}
	break;

	// 其余方块全向导电
	// 此处未单独处理交叉线问题，两个方向导电的问题留给连接电源的时候再去处理。
	default:
		begin = 0;
		end = 3;
		break;
	}

	for (int i = begin; i <= end; i++)
	{
		int nx = x + pConduct[i].x;
		int ny = y + pConduct[i].y;
		if (isInRect(nx, ny, { 0,0,pMap->w - 1,pMap->h - 1 }))
		{
			AssociatePowerToObj(pMap, nx, ny, x, y, power);
		}
	}
}

// 确认电源状态
// power		电源位置
// flagFirst	是否为第一次调用
void CheckPower(RsMap* pMap, Power pPower, bool flagFirst = false)
{
	static RsObj pVisited;	// 利用物体的 power 表存储已遍历点
	static int nCount = 0;

	RsObj* pObj = &pMap->map[pPower.y][pPower.x];

	// 只有火把需要确认状态
	if (pObj->nType == RS_TORCHE)
	{
		// 记录足迹
		AddToPowerList(&pVisited, &pPower, 1);
		nCount++;

		// 递归确认所有电源的状态
		for (int i = 0; i < pObj->nPowerCount; i++)
		{
			// 重复项不再检验，否则会递归爆栈
			if (!SearchPowerInList(pVisited.pPowerList, nCount, pObj->pPowerList[i]))
			{
				CheckPower(pMap, pObj->pPowerList[i]);
			}
		}

		//
		// 现在有两种方案。
		// 1.	在 CheckPower 完全执行完毕后，再完全遍历每个电源的供电表，如果供电都无效，而自己野无效，则需要设置自己有效。
		// 2.	直接在下方判断：如果全是无效电源，而自己也无效，则需要将自己设置为有效
		//

		// 若存在通电电源，则熄灭
		bool any_efficient_power = false;
		for (int i = 0; i < pObj->nPowerCount; i++)
		{
			Power p = pObj->pPowerList[i];
			if (pMap->map[p.y][p.x].bPowered)
			{
				any_efficient_power = true;
				break;
			}
		}
		if (any_efficient_power)
		{
			pObj->bPowered = false;
		}
		else
		{
			pObj->bPowered = true;
		}
	}

	// 位于递归头，回收内存
	if (flagFirst && pVisited.pPowerList != NULL)
	{
		delete[] pVisited.pPowerList;
		pVisited.pPowerList = NULL;
		nCount = 0;
	}
}

// 重置地图状态
void ResetRsMap(RsMap* pMap)
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
					pMap->map[j][i].bPowered = true;
				}
			}
			else
			{
				pMap->map[j][i].bPowered = false;
			}

			// 清空交叉线充能状态
			pMap->map[j][i].bUprightPowered = false;
			pMap->map[j][i].bHorizonPowered = false;

			// 清空交叉线电源标识
			pMap->map[j][i].bHaveUprightPower = false;
			pMap->map[j][i].bHaveHorizonPower = false;

			// 清空所有供电表
			if (pMap->map[j][i].pPowerList != NULL)
			{
				delete[] pMap->map[j][i].pPowerList;
				pMap->map[j][i].pPowerList = NULL;
				pMap->map[j][i].nPowerCount = 0;
			}
		}
	}
}

// 设置红石地图电源关系
void SetRsMapPowerRelation(RsMap* pMap)
{
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (isPowerObj(&pMap->map[j][i]) && pMap->map[j][i].bPowered)
			{
				// 从单个电源出发
				SetPowerRelation(pMap, i, j, Power({ i, j }));
			}
		}
	}
}

// 检查地图中的电源状态
void CheckRsMapPower(RsMap* pMap)
{
	// 审查电源状态（红石火把开关设置）
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (isPowerObj(&pMap->map[j][i]))
			{
				CheckPower(pMap, Power({ i, j }), true);
			}
		}
	}
}

// 根据已有的电源关系导电
void ConductPower(RsMap* pMap)
{
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
					Power p = pMap->map[j][i].pPowerList[k];
					if (pMap->map[p.y][p.x].bPowered)
					{
						pMap->map[j][i].bPowered = true;

						// 对于交叉线，还需要补充是哪个方向通电 
						if (pMap->map[j][i].nType == RS_CROSS)
						{
							pMap->map[j][i].bHorizonPowered |= p.horizon;
							pMap->map[j][i].bUprightPowered |= p.upright;
						}
					}
				}
			}
		}
	}
}

// 运行红石地图
void RunRsMap(RsMap* pMap)
{
	// 重置电源状态
	ResetRsMap(pMap);

	// 联络导体和电源
	SetRsMapPowerRelation(pMap);

	// 电源重定状态
	CheckRsMapPower(pMap);

	// 导电
	ConductPower(pMap);
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


