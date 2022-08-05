#pragma once

#include "redstone.h"

// �ж��ַ��Ƿ�Ϊ����
bool isNum(char ch);

// ��ʼ����ʯ��ͼ
RsMap InitRsMap(int w, int h);

// ɾ����ͼ
void DeleteRsMap(RsMap* map);

// �����ͼ��С
void ResizeRsMap(RsMap* map, int w, int h);

// ������Ʒ����ͼ
void PutObjectToRsMap(RsMap* map, int x, int y, int object_id, int direction = RS_TO_UP);

// ����Ŀ
RsMap OpenProject(const WCHAR* strFileName);

// ������Ŀ
bool SaveProject(RsMap map, const WCHAR* strFileName);

// ������Ŀ
// out		����Ŀ
// in		��������Ŀ
// x, y		��������Ŀλ������Ŀ�����Ͻ�����
void ImportProject(RsMap* out, RsMap in, int x, int y);

// �ں�ʯ��ͼ�л��ƺ�ʯֱ�ߣ���������������ָ����������ʯֱ�ߵ���Ʒ
// �����Ƿ���Ƴɹ������Ʋ��ɹ���ԭ����ֱ��������
bool LineRsMap(RsMap* map, int x1, int y1, int x2, int y2, int object);

// �����ʯ��ͼ�е�һ������
void ClearRsMap(RsMap* map, int x1, int y1, int x2, int y2);

// ���ƺ�ʯ��ͼ
void CopyRsMap(RsMap* pDst, RsMap* pSrc);

// �ȽϺ�ʯ��ͼ���ػ棨�Ƚ����ݣ�ÿ����Ļ���Ч����
// pChange		���ڼ�¼�����������ַ���ǵ����ⲿ delete[]��
// ע�⣺����ͼ���������
void CmpRsMapForRedraw(RsMap* pOld, RsMap* pNew, POINT** pChange, int* pCount);
