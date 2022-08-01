////////////////////////////
//
//	HiFunc.h
//	HiEasyX ��ĳ��ú���ģ��
//

#pragma once

#include <graphics.h>

// ��ȡͼ��ߴ�
// ���Է���ش��� IMAGE ָ��Ϊ�գ���ָ������ͼ���ڵ����
void GetImageSize(IMAGE* pImg, int& width, int& height);

// �õ� IMAGE ����� HBITMAP
HBITMAP Image2Bitmap(IMAGE* img);

// HBITMAP ת HICON
HICON Bitmap2Icon(HBITMAP hBmp);

// ��ȷ��ʱ����(���Ծ�ȷ�� 1ms������ ��1ms)
// by yangw80<yw80@qq.com>, 2011-5-4
void HpSleep(int ms);

// ���Ƿ�λ�ھ�����
bool isInRect(int x, int y, RECT rct);

// ��ȡ ExMessage �� EM ��Ϣ����
UINT GetExMessageType(ExMessage msg);
