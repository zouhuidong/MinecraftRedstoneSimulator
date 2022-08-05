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

			// ���������ɾ��ԭ�ȵĹ��������ҪԤ�Ƚ����ÿ�
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
	if (!pDst)
	{
		*pDst = InitRsMap(pSrc->w, pSrc->h);
	}
	else
	{
		ResizeRsMap(pDst, pSrc->w, pSrc->h);
	}

	for (int i = 0; i < pSrc->w; i++)
	{
		for (int j = 0; j < pSrc->h; j++)
		{
			pDst->map[j][i] = pSrc->map[j][i];
			pDst->map[j][i].pPowerList = nullptr;
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

void CmpRsMapForRedraw(RsMap* pOld, RsMap* pNew, POINT** pChange, int* pCount)
{
	int count = 0;
	POINT* p = new POINT[pNew->w * pNew->h];
	POINT neighborhood[4] = { {-1,0},{1,0},{0,-1},{0,1} };
	for (int i = 0; i < pNew->w; i++)
	{
		for (int j = 0; j < pNew->h; j++)
		{
			if (!isEqualEffect(pOld->map[j][i], pNew->map[j][i]))
			{
				p[count] = { i,j };
				count++;

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
								if (p[n].x == x && p[n].y == y)
									repeat = true;
							if (!repeat)
							{
								p[count] = { x,y };
								count++;
							}
						}
					}
				}
			}
		}
	}
	*pChange = p;
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

// ����Ŀ
RsMap OpenProject(const WCHAR* strFileName)
{
	if (!lstrlen(strFileName))
	{
		return RsMap{};
	}

	FILE* fp;
	int nSize = WideCharToMultiByte(CP_ACP, 0, strFileName, -1, 0, 0, nullptr, nullptr);
	char* chFileName = new char[nSize];
	memset(chFileName, 0, nSize);
	WideCharToMultiByte(CP_ACP, 0, strFileName, -1, chFileName, nSize, nullptr, nullptr);

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

			RsObj rsobj;
			rsobj.nType = nType;
			rsobj.nTowards = nTowards;
			rsobj.bPowered = (bool)nPower;

			map.map[i][j] = rsobj;
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

	FILE* fp;
	int nSize = WideCharToMultiByte(CP_ACP, 0, strFileName, -1, nullptr, 0, nullptr, nullptr);
	char* chFileName = new char[nSize];
	memset(chFileName, 0, nSize);
	WideCharToMultiByte(CP_ACP, 0, strFileName, -1, chFileName, nSize, nullptr, nullptr);

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
			if (map.map[i][j].bPowered && map.map[i][j].nType != RS_BUTTON)
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
