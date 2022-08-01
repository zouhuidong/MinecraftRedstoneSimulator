////////////////////////////
//
//	HiMacro.h
//	HiEasyX ��ĳ��ú궨��ģ��
//

#pragma once

#include <graphics.h>


////// ͸�����

// ��� rgb ɫ�ʺ� alpha ͸����
// �Դ��еĴ洢��ʽ��argb
// COLORREF �Ĵ洢��ʽ��abgr
#define RGBA(r, g, b, a)				(COLORREF)( ((b)<<16) | ((g)<<8) | (r) | ((a)<<24) )

// ��ȡ rgba ɫֵ��͸����
#define GetAValue(rgba)					(BYTE)( ((rgba)>>24) & 0xFF )

// �Ƴ� rgba ɫֵ��͸���ȣ���Ϊ 255 ��͸����
#define REMOVE_ALPHA(rgba)				(COLORREF)( rgba | 0xFF000000 )

// �� rgb ɫֵ����͸����
#define SET_ALPHA(rgb, a)				(COLORREF)( ((rgb) & 0xFFFFFF) | ((a)<<24) )

// ��ȡĳ�Ҷȶ�Ӧ�� rgb ɫֵ
#define GRAY_COLOR(gray)				RGB(gray, gray, gray)

// ��ȡĳ�Ҷȶ�Ӧ�� rgb ɫֵ����͸���ȣ�
#define GRAY_COLOR_ALPHA(gray, a)		RGBA(gray, gray, gray, a)

////// EasyX ��ͼ�����궨��

#define rectangle_RECT(rct)				rectangle(rct.left,rct.top,rct.right,rct.bottom);
#define fillrectangle_RECT(rct)			fillrectangle(rct.left,rct.top,rct.right,rct.bottom);
#define solidrectangle_RECT(rct)		solidrectangle(rct.left,rct.top,rct.right,rct.bottom);

////// ��ѧ

// Բ����
#define PI 3.1415926535

////// ����

// �� RECT �߿����� d ����
#define EXPAND_RECT(rct, d)				(rct).left		-= d;\
										(rct).top		-= d;\
										(rct).right		+= d;\
										(rct).bottom	+= d

// �� RECT �߿��С d ����
#define REDUCE_RECT(rct, d)				(rct).left		+= d;\
										(rct).top		+= d;\
										(rct).right		-= d;\
										(rct).bottom	-= d

// �Ƿ��������
#define IS_INCLUDE_RECT(rctParent, rctChild)	(	(rctChild).left		>= (rctParent).left		\
												&&	(rctChild).right	<= (rctParent).right	\
												&&	(rctChild).top		>= (rctParent).top		\
												&&	(rctChild).bottom	<= (rctParent).bottom	)

// �ƶ�����
#define MOVE_RECT(rct, dx, dy)			(rct).left		+= dx;\
										(rct).top		+= dy;\
										(rct).right		+= dx;\
										(rct).bottom	+= dy


// ����ɫ����չ
enum EXTRA_COLORS
{
	DARKBLUE = RGB(0x00, 0x00, 0x8B),
	DARKCYAN = RGB(0x00, 0x8B, 0x8B),
	DARKGOLDENROD = RGB(0xB8, 0x86, 0x0B),
	DARKGREEN = RGB(0x00, 0x64, 0x00),
	DARKRED = RGB(0x8B, 0x00, 0x00),
	DEEPPINK = RGB(0xFF, 0x14, 0x93),
	DEEPSKYBLUE = RGB(0x00, 0xBF, 0xFF),
	FORESTGREEN = RGB(0x22, 0x8B, 0x22),
	GOLD = RGB(0xFF, 0xD7, 0x00),
	GRAY = RGB(0x80, 0x80, 0x80),
	GREENYELLOW = RGB(0xAD, 0xFF, 0x2F),
	LIGHTPINK = RGB(0xFF, 0xB6, 0xC1),
	LIGHTSKYBLUE = RGB(0x87, 0xCE, 0xFA),
	LIGHTYELLOW = RGB(0xFF, 0xFF, 0xE0),
	DARKYELLOW = RGB(255, 201, 14),
	ORANGE = RGB(0xFF, 0xA5, 0x00),
	ORANGERED = RGB(0xFF, 0x45, 0x00),
	PINK = RGB(0xFF, 0xC0, 0xCB),
	PINKWHITE = RGB(255, 230, 250),
	PURPLE = RGB(0x80, 0x00, 0x80),
	SKYBLUE = RGB(0x87, 0xCE, 0xEB),
	SNOW = RGB(0xFF, 0xFA, 0xFA),
	SPRINGGREEN = RGB(0x00, 0xFF, 0x7F),
	STEELBLUE = RGB(0x46, 0x82, 0xB4),
	TOMATO = RGB(0xFF, 0x63, 0x47),
	WHITESMOKE = RGB(0xF5, 0xF5, 0xF5),
	YELLOWGREEN = RGB(0x9A, 0xCD, 0x32),
	
	CLASSICGRAY = RGB(0xF0, 0xF0, 0xF0),			// Windows �����

	MODEN_BORDER_GRAY = 0xadadad,					// �ִ��߿��
	MODEN_FILL_GRAY = 0xe1e1e1,						// �ִ�����

	MODEN_BORDER_BLUE = 0xd77800,					// �ִ��߿���
	MODEN_FILL_BLUE = 0xfbf1e5,						// �ִ������

	MODEN_BORDER_PRESSED_BLUE = 0x995400,			// �ִ��߿��������£�
	MODEN_FILL_PRESSED_BLUE = 0xf7e4cc,				// �ִ�����������£�
};

