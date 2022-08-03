#pragma once

#include <Windows.h>

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

struct Power
{
	bool isPowerOfCross = false;	// �Ƿ�Ϊ�����ߵĵ�Դ

	// �涨�����ͬһ����Դ���������ߵ�����������Ӧ���洢������Դ���ֱ��ʶΪˮƽ��Դ����ֱ��Դ
	bool horizon = false;			// �ǽ����ߵ�ˮƽ�����Դ�������ڽ����ߣ�
	bool upright = false;			// �ǽ����ߵĴ�ֱ�����Դ�������ڽ����ߣ�

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

// ��ʯ���鶨��
typedef struct RedstoneObject
{
	int nType = RS_NULL;				// ��������
	int nTowards = RS_TO_UP;			// ����

	bool bPowered = false;				// �Ƿ���ܣ����ڽ����ߣ����ⷽ����ܶ����ʶ��

	bool bUprightPowered = false;		// ��ֱ�����Ƿ���ܣ������ڽ����ߣ�
	bool bHorizonPowered = false;		// ˮƽ�����Ƿ���ܣ������ڽ����ߣ�

	bool bHaveUprightPower = false;		// ��ֱ�����Ƿ���ڹ���Դ�������ڽ����ߣ�
	bool bHaveHorizonPower = false;		// ˮƽ�����Ƿ���ڹ���Դ�������ڽ����ߣ�

	int nPowerCount = 0;				// ��������
	Power* pPowerList = NULL;			// �����Դ����

}RsObj;

// ��ʯ��ͼ
typedef struct RedstoneMap
{
	int w, h;		// ��ͼ���
	RsObj** map;	// ��ͼ
}RsMap;


// ��ʼ����ʯ��ͼ
RsMap InitRsMap(int w, int h);

// ɾ����ͼ
void DeleteRsMap(RsMap* map);

// �����ͼ��С
void ResizeRsMap(RsMap* map, int w, int h);

// ���к�ʯ��ͼ
void RunRsMap(RsMap* pMap);
