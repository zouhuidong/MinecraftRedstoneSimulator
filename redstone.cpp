#include "redstone.h"

#include "HiEasyX/HiFunc.h"

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

// ���ҹ�������Ƿ���ĳ����Դ
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

// ��Ӷ������Դ�������
// ������ӳɹ��ĸ���������ԭ���ظ�����ĸ�����
int AddToPowerList(RsObj* pObj, Power* pPowerList, int nCount)
{
	Power* pNewList = new Power[pObj->nPowerCount + nCount];
	int sum = 0;	// ȥ�غ������е�����
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

// �ж�һ�����Ƿ�Ϊ�ź�Դ����
bool isPowerObj(RsObj* obj)
{
	return obj->nType == RS_ROD || obj->nType == RS_BUTTON || obj->nType == RS_TORCHE;
}

// �Ƿ�Ϊ��ͨ���飨��һ�пɱ����ܵ�ʵ�巽�飬������ʯ��ѣ������������Դ���飩
bool isNormalObj(RsObj* obj)
{
	return (obj->nType != RS_NULL && !isPowerObj(obj)) || obj->nType == RS_TORCHE;
}

// �Ƿ�Ϊ�ɵ��緽�飨ɸѡ��ʽ 1��
// ����һ��ʵ�巽�飨���˺�ʯ�ƣ��������е�Դ��
// ��������Է��磬����Լ�������ķ���
bool isConductiveObj(RsObj* obj)
{
	return obj->nType != RS_NULL && obj->nType != RS_LIGHT;
}

// �Ƿ�Ϊ�ɵ��緽�飨ɸѡ��ʽ 2��
// �������˵ơ���Դ�����ʵ�巽��
// �����Լ�������ķ���
// ��ɸѡ��ʽ 1 ���������ڣ���Դ����
bool isConductiveObj2(RsObj* obj)
{
	return obj->nType != RS_NULL && obj->nType != RS_LIGHT && !isPowerObj(obj);
}

void SetPowerRelation(RsMap*, int, int, Power);

// �ɳ��ܷ������ӵ���Դ
// x, y		��ǰ����λ��
// cx, cy	������λ�ã�������������ǰ���飩
// power	�����Դλ��
// �����Ƿ����ӳɹ�����֮ǰ���ӹ��˵�Դ���򷵻� false
bool AssociateWithPower(RsMap* pMap, int x, int y, int cx, int cy, Power power)
{
	RsObj& obj = pMap->map[y][x];		// ��ǰ����
	RsObj cobj = pMap->map[cy][cx];		// ���緽��
	int dx = cx - x, dy = cy - y;		// ���緽����Ե�ǰ�����λ��

	bool associated = false;			// ��ʶ�Ƿ�ɹ����ӵ���Դ

	// ���ݵ�ǰ�������ͣ�Լ����������
	switch (obj.nType)
	{
		// ��ʯ���ֻ�����м�������
	case RS_TORCHE:
	{
		if (cobj.nType != RS_RELAY)		// �м������򲻻������Ϊ����ʱ���Ѿ������˴�������
		{
			return false;
		}
	}
	break;

	// �м���ֻ����ͬ����
	case RS_RELAY:
	{
		switch (obj.nTowards)			// �����緽�򲻶ԣ�������ʧ��
		{
		case RS_TO_UP:		if (!(dx == 0 && dy == 1))	return false;	break;
		case RS_TO_DOWN:	if (!(dx == 0 && dy == -1))	return false;	break;
		case RS_TO_LEFT:	if (!(dx == 1 && dy == 0))	return false;	break;
		case RS_TO_RIGHT:	if (!(dx == -1 && dy == 0))	return false;	break;
		}
	}
	break;
	}

	// ���ݸ��������ͣ�������ͬ���ж�
	switch (cobj.nType)
	{
		// ���� CROSS ��Ҫ�ж����緽��
	case RS_CROSS:
		if (dx)			associated = cobj.bHaveHorizonPower;
		else if (dy)	associated = cobj.bHaveUprightPower;
		break;

		// ���෽�����ֱ�ӱ��
	default:
		associated = true;
		break;
	}

	// ���ӵ�Դ
	if (associated)
	{
		Power mypower = power;

		if (obj.nType == RS_CROSS)	// ���ڽ����ߣ��������¼����ϸ�ĵ�Դ��Ϣ
		{
			bool horizon = dx;		// ��¼��Դ����
			bool upright = dy;

			obj.bHaveHorizonPower = horizon;	// �ڽ������м�¼������ĳ������ĵ�Դ
			obj.bHaveUprightPower = upright;

			mypower.isPowerOfCross = true;		// �ڵ�Դ�м�¼�����ǽ����ߵ�ĳ������ĵ�Դ
			mypower.horizon = horizon;
			mypower.upright = upright;
		}

		associated = AddToPowerList(&obj, &mypower, 1);
	}

	return associated;
}

// �ѵ�Դ�Ϳɳ��ܷ�����ϵ����������ʯ��ѣ�
// x, y		��ǰ����λ��
// cx, cy	������λ�ã�������������ǰ���飩
// power	�����Դλ��
void AssociatePowerToObj(RsMap* pMap, int x, int y, int cx, int cy, Power power)
{
	RsObj* pObj = &pMap->map[y][x];

	// �˷������ɱ�����
	if (!isNormalObj(pObj))
		return;

	// ������ĵ�Դ��˵�����ϵ
	if (AssociateWithPower(pMap, x, y, cx, cy, power))
	{
		// ����ǿ��Լ������������壬�����
		if (isConductiveObj2(pObj))
		{
			// ����������Դ��ϵ
			SetPowerRelation(pMap, x, y, power);
		}
	}
}

// ���õ�Դ�͵�������ӹ�ϵ
// power	��Դλ��
void SetPowerRelation(RsMap* pMap, int x, int y, Power power)
{
	// �����ܵĴ�������
	POINT pConduct[4] = { {-1,0},{1,0},{0,-1},{0,1} };

	// ������ʼ�ͽ�����
	int begin = 0, end = 0;

	switch (pMap->map[y][x].nType)
	{
		// �м���ֻ��һ�����򵼵�
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

	// ���෽��ȫ�򵼵�
	// �˴�δ���������������⣬�������򵼵�������������ӵ�Դ��ʱ����ȥ����
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

// ȷ�ϵ�Դ״̬
// power		��Դλ��
// flagFirst	�Ƿ�Ϊ��һ�ε���
void CheckPower(RsMap* pMap, Power pPower, bool flagFirst = false)
{
	static RsObj pVisited;	// ��������� power ��洢�ѱ�����
	static int nCount = 0;

	RsObj* pObj = &pMap->map[pPower.y][pPower.x];

	// ֻ�л����Ҫȷ��״̬
	if (pObj->nType == RS_TORCHE)
	{
		// ��¼�㼣
		AddToPowerList(&pVisited, &pPower, 1);
		nCount++;

		// �ݹ�ȷ�����е�Դ��״̬
		for (int i = 0; i < pObj->nPowerCount; i++)
		{
			// �ظ���ټ��飬�����ݹ鱬ջ
			if (!SearchPowerInList(pVisited.pPowerList, nCount, pObj->pPowerList[i]))
			{
				CheckPower(pMap, pObj->pPowerList[i]);
			}
		}

		//
		// ���������ַ�����
		// 1.	�� CheckPower ��ȫִ����Ϻ�����ȫ����ÿ����Դ�Ĺ����������綼��Ч�����Լ�Ұ��Ч������Ҫ�����Լ���Ч��
		// 2.	ֱ�����·��жϣ����ȫ����Ч��Դ�����Լ�Ҳ��Ч������Ҫ���Լ�����Ϊ��Ч
		//

		// ������ͨ���Դ����Ϩ��
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

	// λ�ڵݹ�ͷ�������ڴ�
	if (flagFirst && pVisited.pPowerList != NULL)
	{
		delete[] pVisited.pPowerList;
		pVisited.pPowerList = NULL;
		nCount = 0;
	}
}

// ���õ�ͼ״̬
void ResetRsMap(RsMap* pMap)
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
					pMap->map[j][i].bPowered = true;
				}
			}
			else
			{
				pMap->map[j][i].bPowered = false;
			}

			// ��ս����߳���״̬
			pMap->map[j][i].bUprightPowered = false;
			pMap->map[j][i].bHorizonPowered = false;

			// ��ս����ߵ�Դ��ʶ
			pMap->map[j][i].bHaveUprightPower = false;
			pMap->map[j][i].bHaveHorizonPower = false;

			// ������й����
			if (pMap->map[j][i].pPowerList != NULL)
			{
				delete[] pMap->map[j][i].pPowerList;
				pMap->map[j][i].pPowerList = NULL;
				pMap->map[j][i].nPowerCount = 0;
			}
		}
	}
}

// ���ú�ʯ��ͼ��Դ��ϵ
void SetRsMapPowerRelation(RsMap* pMap)
{
	for (int i = 0; i < pMap->w; i++)
	{
		for (int j = 0; j < pMap->h; j++)
		{
			if (isPowerObj(&pMap->map[j][i]) && pMap->map[j][i].bPowered)
			{
				// �ӵ�����Դ����
				SetPowerRelation(pMap, i, j, Power({ i, j }));
			}
		}
	}
}

// ����ͼ�еĵ�Դ״̬
void CheckRsMapPower(RsMap* pMap)
{
	// ����Դ״̬����ʯ��ѿ������ã�
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

// �������еĵ�Դ��ϵ����
void ConductPower(RsMap* pMap)
{
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
					Power p = pMap->map[j][i].pPowerList[k];
					if (pMap->map[p.y][p.x].bPowered)
					{
						pMap->map[j][i].bPowered = true;

						// ���ڽ����ߣ�����Ҫ�������ĸ�����ͨ�� 
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

// ���к�ʯ��ͼ
void RunRsMap(RsMap* pMap)
{
	// ���õ�Դ״̬
	ResetRsMap(pMap);

	// ���絼��͵�Դ
	SetRsMapPowerRelation(pMap);

	// ��Դ�ض�״̬
	CheckRsMapPower(pMap);

	// ����
	ConductPower(pMap);
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


