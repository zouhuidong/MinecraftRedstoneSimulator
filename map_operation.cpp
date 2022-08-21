#include "map_operation.h"

#include "HiEasyX/HiFunc.h"

#include <stdio.h>
#include <io.h>

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
	}

	return map;
}

// ɾ����ͼ
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

			// �˴�������ÿ������ĸ���������ٽ�ԭ��ͼɾ��
			// ɾ����ͼ������Ĺ�������Ұָ�룬���Դ˴�ֱ���ÿ�
			newmap.map[j][i].pPowerList = nullptr;
			newmap.map[j][i].nPowerCount = 0;
		}
	}

	DeleteRsMap(map);
	*map = newmap;
}

// ������Ʒ����ͼ
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

// �����ж�������������ʾЧ�����Ƿ���ȣ����ڲ���ȣ�
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
				// ����
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

				// �쳣���
				if (count > pNew->w * pNew->h)
				{
					DebugBreak();
					return;
				}

				// ����һ������ĸı����Ӱ�쵽���ܵĺ�ʯ�����������ڽ��ĺ�ʯ�ػ�
				for (int k = 0; k < 4; k++)
				{
					int x = i + neighborhood[k].x;
					int y = j + neighborhood[k].y;
					if (isInRect(x, y, { 0,0,pNew->w - 1,pNew->h - 1 }))
					{
						if (pNew->map[y][x].nType == RS_POWDER)
						{
							// ���Ѵ��ڴ˵������ظ�
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

// �ж��ַ��Ƿ�Ϊ����
bool isNum(char ch)
{
	return (ch >= '0' && ch <= '9') || ch == '-';
}

// �ж��Ƿ�Ϊ�ɰ��ͼ
// ���ݣ���ͷ�������ַ���������һ���ո񣨿� �ߵĽṹ��
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

// ����Ŀ
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
		MessageBox(nullptr, L"���ļ�ʧ��", L"ERROR", MB_OK);
		return RsMap{};
	}

	int len = _filelength(_fileno(fp));
	int* pBuf = new int[len];
	ZeroMemory(pBuf, len * sizeof(int));
	fread(pBuf, len * sizeof(int), 1, fp);
	fclose(fp);

	if (isOldMap(pBuf, len))
	{
		MessageBox(nullptr, L"�˵�ͼΪ�ɰ��ͼ����Ҫת��Ϊ�°��ͼ���������򿪡�", L"ERROR", MB_OK);
		return RsMap{};
	}

	// ��ͼ���
	int w = pBuf[0];
	int h = pBuf[1];
	RsMap map = InitRsMap(w, h);

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			int index = i + j * w;		// Ԫ������
			int pos = 2 + index * 3;	// ʵ�ʶ�ȡλ��

			RsObj rsobj;
			rsobj.nType = pBuf[pos];
			rsobj.nTowards = pBuf[pos + 1];
			rsobj.bPowered = pBuf[pos + 2];

			map.map[j][i] = rsobj;
		}
	}

	return map;
}

// ������Ŀ
bool SaveProject(RsMap map, const WCHAR* strFileName)
{
	if (!lstrlen(strFileName))
	{
		return false;
	}

	int len = 2 + (map.w * map.h) * 3;
	int* pBuf = new int[len];
	ZeroMemory(pBuf, len * sizeof(int));

	// ��ͼ���
	pBuf[0] = map.w;
	pBuf[1] = map.h;

	for (int i = 0; i < map.w; i++)
	{
		for (int j = 0; j < map.h; j++)
		{
			int index = i + j * map.w;	// Ԫ������
			int pos = 2 + index * 3;	// ʵ�ʶ�ȡλ��

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
		MessageBox(nullptr, L"����ʧ��", L"ERROR", MB_OK);
		return false;
	}
	fwrite(pBuf, len * sizeof(int), 1, fp);
	fclose(fp);

	return true;
}
