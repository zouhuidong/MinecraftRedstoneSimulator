#include "map_operation.h"

#include "HiEasyX/HiFunc.h"

#include <stdio.h>
#include <io.h>

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
	if (map->map != nullptr)
	{
		for (int i = 0; i < map->h; i++)
		{
			for (int j = 0; j < map->w; j++)
			{
				if (map->map[i][j].pPowerList)
				{
					delete[] map->map[i][j].pPowerList;
					map->map[i][j].pPowerList = nullptr;
				}
			}

			delete[] map->map[i];
		}

		delete[] map->map;
		map->map = nullptr;
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

			// 此处创建了每个物体的副本，最后再将原地图删除
			// 删除地图后，物体的供电表会变成野指针，所以此处直接置空
			newmap.map[j][i].pPowerList = nullptr;
			newmap.map[j][i].nPowerCount = 0;
		}
	}

	DeleteRsMap(map);
	*map = newmap;
}

// 放置物品到地图
void PutObjectToRsMap(RsMap* map, int x, int y, int object_id, int direction)
{
	RsObj obj;
	obj.nType = object_id;

	switch (object_id)
	{
	case RS_TORCHE:
		obj.bPowered = true;
		break;
	case RS_RELAY:
		obj.nTowards = direction;
		break;
	}

	map->map[y][x] = obj;
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

void CopyRsMap(RsMap* pDst, RsMap* pSrc)
{
	ResizeRsMap(pDst, pSrc->w, pSrc->h);

	for (int i = 0; i < pSrc->w; i++)
	{
		for (int j = 0; j < pSrc->h; j++)
		{
			pDst->map[j][i] = pSrc->map[j][i];
			pDst->map[j][i].pPowerList = new Power[pSrc->map[j][i].nPowerCount];
			memcpy(pDst->map[j][i].pPowerList, pSrc->map[j][i].pPowerList, pSrc->map[j][i].nPowerCount * sizeof(Power));
		}
	}
}

// 粗略判断两个物体在显示效果上是否相等（趋于不相等）
bool isEqualEffect(RsObj obj1, RsObj obj2)
{
	return (obj1.nType == obj2.nType
		&& obj1.nTowards == obj2.nTowards
		&& obj1.bPowered == obj2.bPowered
		&& obj1.bUprightPowered == obj2.bUprightPowered
		&& obj1.bHorizonPowered == obj2.bHorizonPowered);
}

void CmpRsMapForRedraw(RsMap* pOld, RsMap* pNew, POINT* pChange, int* pCount)
{
	int count = 0;
	POINT neighborhood[4] = { {-1,0},{1,0},{0,-1},{0,1} };
	for (int i = 0; i < pNew->w; i++)
	{
		for (int j = 0; j < pNew->h; j++)
		{
			if (!isEqualEffect(pOld->map[j][i], pNew->map[j][i]))
			{
				// 防重
				bool repeat = false;
				for (int n = 0; n < count; n++)
					if (pChange[n].x == i && pChange[n].y == j)
					{
						repeat = true;
						break;
					}
				if (repeat)
					continue;

				pChange[count] = { i,j };
				count++;

				// 异常情况
				if (count > pNew->w * pNew->h)
				{
					DebugBreak();
					return;
				}

				// 由于一个物体的改变可能影响到四周的红石，所以连带邻近的红石重绘
				for (int k = 0; k < 4; k++)
				{
					int x = i + neighborhood[k].x;
					int y = j + neighborhood[k].y;
					if (isInRect(x, y, { 0,0,pNew->w - 1,pNew->h - 1 }))
					{
						if (pNew->map[y][x].nType == RS_POWDER)
						{
							// 若已存在此点则不再重复
							bool repeat = false;
							for (int n = 0; n < count; n++)
								if (pChange[n].x == x && pChange[n].y == y)
								{
									repeat = true;
									break;
								}
							if (!repeat)
							{
								pChange[count] = { x,y };
								count++;
							}
						}
					}
				}
			}
		}
	}
	*pCount = count;
}

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

// 判断字符是否为数字
bool isNum(char ch)
{
	return (ch >= '0' && ch <= '9') || ch == '-';
}

// 判断是否为旧版地图
// 依据：开头是数字字符，紧接着一个空格（宽 高的结构）
bool isOldMap(void* pBuf, int len)
{
	char* buf = (char*)pBuf;
	int i = 0;
	for (; i < len && isNum(buf[i]); i++);
	if (i > 0 && buf[i] == ' ')
		return true;
	else
		return false;
}

// 打开项目
RsMap OpenProject(const WCHAR* strFileName)
{
	if (!lstrlen(strFileName))
	{
		return RsMap{};
	}

	FILE* fp = nullptr;
	errno_t err = _wfopen_s(&fp, strFileName, L"r+");
	if (err)
	{
		MessageBox(nullptr, L"打开文件失败", L"ERROR", MB_OK);
		return RsMap{};
	}

	int len = _filelength(_fileno(fp));
	int* pBuf = new int[len];
	ZeroMemory(pBuf, len * sizeof(int));
	fread(pBuf, len * sizeof(int), 1, fp);
	fclose(fp);

	if (isOldMap(pBuf, len))
	{
		MessageBox(nullptr, L"此地图为旧版地图，需要转换为新版地图才能正常打开。", L"ERROR", MB_OK);
		return RsMap{};
	}

	// 地图宽高
	int w = pBuf[0];
	int h = pBuf[1];
	RsMap map = InitRsMap(w, h);

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			int index = i + j * w;		// 元素索引
			int pos = 2 + index * 3;	// 实际读取位置

			RsObj rsobj;
			rsobj.nType = pBuf[pos];
			rsobj.nTowards = pBuf[pos + 1];
			rsobj.bPowered = pBuf[pos + 2];

			map.map[j][i] = rsobj;
		}
	}

	return map;
}

// 保存项目
bool SaveProject(RsMap map, const WCHAR* strFileName)
{
	if (!lstrlen(strFileName))
	{
		return false;
	}

	int len = 2 + (map.w * map.h) * 3;
	int* pBuf = new int[len];
	ZeroMemory(pBuf, len * sizeof(int));

	// 地图宽高
	pBuf[0] = map.w;
	pBuf[1] = map.h;

	for (int i = 0; i < map.w; i++)
	{
		for (int j = 0; j < map.h; j++)
		{
			int index = i + j * map.w;	// 元素索引
			int pos = 2 + index * 3;	// 实际读取位置

			RsObj rsobj = map.map[j][i];
			pBuf[pos] = rsobj.nType;
			pBuf[pos + 1] = rsobj.nTowards;
			pBuf[pos + 2] = rsobj.bPowered;
		}
	}

	FILE* fp = nullptr;
	errno_t err = _wfopen_s(&fp, strFileName, L"wt+");
	if (err)
	{
		MessageBox(nullptr, L"保存失败", L"ERROR", MB_OK);
		return false;
	}
	fwrite(pBuf, len * sizeof(int), 1, fp);
	fclose(fp);

	return true;
}
