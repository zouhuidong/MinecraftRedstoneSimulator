/////////////////////////////////////////////////////////////////////
//
//	EasyWin32.h
//	���� EasyX ͼ�ο�� Win32 ��չ��
//
//	�������ߣ�huidong <huidong_mail@163.com>
//	�桡������Ver 2.6.0
//	���뻷����VisualStudio 2022 | EasyX_20220116 | Windows 10 
//	��Ŀ��ַ��https://github.com/zouhuidong/EasyWin32
//	�������ڣ�2020.12.06
//	����޸ģ�2022.04.03
//

#pragma once

#include <graphics.h>
#include <vector>
#include <string>
#include <thread>

// �����ͼ���ڳ�ʼ������
#define EW_NORMAL 0

#define EASY_WIN32_BEGIN	namespace EasyWin32 {
#define EASY_WIN32_END		};

EASY_WIN32_BEGIN

////////////****** �ṹ�嶨�� ******////////////

// ����
struct EasyWindow
{
	HWND hWnd;							// ���ھ��
	HWND hParent;						// �����ھ��
	IMAGE* pImg;						// ����ͼ��
	IMAGE* pBufferImg;					// ͼ�񻺳���
										// ������Ϣ������
	bool(*funcWndProc)(HWND, UINT, WPARAM, LPARAM, HINSTANCE);
	std::vector<ExMessage> vecMouseMsg;	// �����Ϣ����
	int nGetMouseMsgIndex;				// ��ȡ�����Ϣ�Ľ������������ڻ�ȡ���������е���һ����
	bool isNewSize;						// ���ڴ�С�Ƿ�ı�
	bool isSentCreateMsg;				// �Ƿ�ģ�ⷢ���� WM_CREATE ����Ϣ
	bool isBusyProcessing;				// �Ƿ���æ�ڴ����ڲ���Ϣ
};

////////////****** ������غ��� ******////////////

// ����֧�� win32 �Ļ�ͼ���ڣ�Ĭ��֧�ִ���˫����Ϣ��
// w, h				���ڴ�С
// isCmd			�Ƿ���ʾ cmd ���ڣ�����ǿ���̨Ӧ�ó���
// strWndTitle		���ڱ���
// WindowProcess	������Ϣ��������ָ�룬Ϊ�ձ�ʾʹ��Ĭ����Ϣ�����������ͷ�ļ��С�������Ϣ�������淶����
// hParent			�����ھ����Ϊ�����ʾ�ô��ڶ������ڣ�����д���ô��ڽ���Ϊģ̬���ڣ�
HWND initgraph_win32(
	int w = 640,
	int h = 480,
	int flag = EW_NORMAL,
	LPCTSTR strWndTitle = L"",
	bool(*WindowProcess)(HWND, UINT, WPARAM, LPARAM, HINSTANCE) = NULL,
	HWND hParent = NULL
);

//
// ������Ϣ�������淶
// 
// ������׼��̬��bool WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance);
// 
// ע�⣺
// ����ڱ�׼�� Win32 ���ڹ��̺�����������һ�� HINSTANCE �����βΡ�
// ����ֵ������Ҳ����ͬ�������ġ�
// 
// ����ֵ��
// true		��ʾʹ��ϵͳĬ�Ϸ����������Ϣ
// false	��ʾ������ҪϵͳĬ�Ϸ����������Ϣ
// 
// ע�����
// 1. ���� WM_CREATE ��Ϣʱ��wParam �� lParam �ǿյģ����޷���� CREATESTRUCT �ṹ����Ϣ
// 2. ���� WM_CLOSE ��Ϣʱ������ true �� false ��ʾ�Ƿ�رմ��ڣ�������رմ��ڣ��������д���ٴ��ڵĴ���
//

// �ر�ĳһ��ͼ���ڣ������Ϊ NULL ��ر����л�ͼ����
void closegraph_win32(HWND hWnd = NULL);

// ������ʼ�����ڣ�������
void init_end();

// ���ã������ڶ�������ʱ���Զ��˳�����
void AutoExit();

// �Ƿ񻹴���δ���ٵĻ�ͼ����
bool isAnyWindow();

// �ж�һ�����Ƿ񻹴��ڣ������ѱ��رյĴ��ڣ�
bool isAliveWindow(HWND hWnd);

// �õ���ǰ��ͼ���ڵľ��
HWND GetHWnd_win32();

// �����ȴ���ǰ��ͼ�������
void WaitForTask();

// �ȴ������ڲ���Ϣ�������
void WaitForProcessing(EasyWindow* pWnd);

// �õ���ǰ��ͼ���ڵ���ϸ��Ϣ
EasyWindow GetWorkingWindow();

// ���õ�ǰ��ͼ���ڣ�ͬʱ���û�ͼ����Ϊ���ڶ�Ӧ�� IMAGE ���󣩣������Ƿ����óɹ�
// ���л�ͼ��������У���ȴ���ǰ��ͼ�������
// ������ʹ�� BEGIN_TASK �꣩
bool SetWorkingWindow(HWND hWnd);

// ǿ���ػ浱ǰ��ͼ���ڣ������� WM_PAINT ��Ϣ�ڻ�ͼ����Ҫʹ�ô˺�����
void EnforceRedraw();

// ���濪ʼһ�λ��ƣ�����ʹ�� BEGIN_TASK �꣩
void BeginTask();

// �����ͼ���壬����ʾ��ǰ��ͼ�����һ����
//������ʹ�� FLUSH_DRAW �� END_TASK �꣩
void EndTask();

// ��ȡ�Ѵ����Ĵ��ڵ����飨�����ѱ��رյĴ��ڣ�
std::vector<EasyWindow> GetCreatedWindowList();

// �жϴ��ڴ�С�Ƿ�ı�
bool isWindowSizeChanged(HWND hWnd = NULL);

// �ж��Զ������ͼ�������״̬
bool GetCustomIconState();

// ʹ��ͼ����Դ��Ϊ����ͼ��
// ��������ͼ����Դ ID����ͼ���Сͼ�꣩
// ע�������ڵ�һ�δ�������ǰ�͵��øú���������Ч��Ĭ������£������Ի� EasyX ����ͼ��
void SetCustomIcon(int nIcon, int nIconSm);

// ��ȡ EasyWin32 �Ի�Ĭ�ϴ���ͼ��� IMAGE
IMAGE GetDefaultIconImage();

// ��ȡ��ǰ������ʽ
long GetWindowStyle();

// ���õ�ǰ������ʽ
int SetWindowStyle(long lNewStyle);

// ��ȡ��ǰ������չ��ʽ
long GetWindowExStyle();

// ���õ�ǰ������չ��ʽ
int SetWindowExStyle(long lNewExStyle);

////////////****** �����Ϣ��غ��� ******////////////

//// MOUSEMSG ʽ����

// ����Ƿ���������Ϣ
bool MouseHit_win32();

// �����ȴ���ֱ����ȡ��һ���µ������Ϣ
ExMessage GetMouseMsg_win32();

// ��ȡһ���µ������Ϣ�����������Ƿ��ȡ�ɹ�
bool PeekMouseMsg_win32(ExMessage* pMsg, bool bRemoveMsg = true);

// ��������Ϣ
void FlushMouseMsgBuffer_win32();

//// ExMessage ʽ����

// �����ȴ���ֱ����ȡ��һ������Ϣ���ݽ�֧�������Ϣ EM_MOUSE��
ExMessage getmessage_win32(BYTE filter = -1);

// �����ȴ���ֱ����ȡ��һ������Ϣ���ݽ�֧�������Ϣ EM_MOUSE��
void getmessage_win32(ExMessage* msg, BYTE filter = -1);

// ��ȡһ����Ϣ�����������Ƿ��ȡ�ɹ����ݽ�֧�������Ϣ EM_MOUSE��
bool peekmessage_win32(ExMessage* msg, BYTE filter = -1, bool removemsg = true);

// ���������Ϣ��¼���ݽ�֧�������Ϣ EM_MOUSE��
void flushmessage_win32(BYTE filter = -1);

//// ת��

// MOUSEMSG ת ExMessage
ExMessage To_ExMessage(MOUSEMSG msg);

// ExMessage ת MOUSEMSG
MOUSEMSG To_MouseMsg(ExMessage msgEx);

// ���ݾɰ� MOUSEMSG
bool PeekMouseMsg_win32_old(MOUSEMSG* pMsg, bool bRemoveMsg = true);

//
//	�����Ϣ���ݾɰ� MOUSEMSG��ͬʱ֧�� ExMessage
//	�� ExMessage ϵ�к�����ʱֻ�ܻ�ȡ EM_MOUSE �������Ϣ
//

EASY_WIN32_END

////////////****** ����ָ��궨�� ******////////////

// ����һ�Σ���ͼ�����񣨻��Ƶ���ǰ��ͼ���ڣ�
#define BEGIN_TASK()\
	EasyWin32::WaitForTask();\
	if(EasyWin32::isAliveWindow(EasyWin32::GetHWnd_win32()))\
	{\
		EasyWin32::BeginTask()

// ����һ�Σ���ͼ������ָ��Ŀ���ͼ���ڣ�
#define BEGIN_TASK_WND(hWnd)\
	/* ���ù�������ʱ���Զ��ȴ���ǰ������� */\
	if (EasyWin32::SetWorkingWindow(hWnd))\
	{\
		EasyWin32::BeginTask()

// ����һ�Σ���ͼ�����񣬲������ͼ���棨���� BEGIN_TASK ���ã�
#define END_TASK()\
		EasyWin32::EndTask();\
	}(0)	/* �˴�ǿ��Ҫ��ӷֺ� */

// Ҫ�󴰿��ػ�
#define FLUSH_DRAW()			EasyWin32::EnforceRedraw()

////////////****** ������ʽ�궨�� ******////////////

// �Ƿ���õ�ǰ���ڸı��С
#define DisableResizing(b)		(b ? EasyWin32::SetWindowStyle(EasyWin32::GetWindowStyle() & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX) :\
								EasyWin32::SetWindowStyle(EasyWin32::GetWindowStyle() | WS_SIZEBOX | WS_MAXIMIZEBOX))

////////////****** ������Ϣ�궨�� ******////////////

// �жϵ�ǰ������Ƿ���ܵ�ĳ������Ϣ
#define KEY_DOWN(VK_NONAME) (GetForegroundWindow() == EasyWin32::GetHWnd_win32() && (GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

////////////****** EasyX ԭ�������ĺ��滻 ******////////////

// ��ʹ�� EasyX ԭ�������������ڣ���رմ���ʱ�Զ��˳�����
#define initgraph(w, h)			EasyWin32::initgraph_win32(w, h);\
								EasyWin32::AutoExit()

#define closegraph				EasyWin32::closegraph_win32

// Ĭ��ʹ��˫���壬�� BeginBatchDraw ������
#define BeginBatchDraw()
#define FlushBatchDraw()		FLUSH_DRAW()
#define EndBatchDraw()			FLUSH_DRAW()

#define GetHWnd					EasyWin32::GetHWnd_win32

#define getmessage				EasyWin32::getmessage_win32
#define peekmessage				EasyWin32::peekmessage_win32
#define flushmessage			EasyWin32::flushmessage_win32

#define MouseHit				EasyWin32::MouseHit_win32
#define GetMouseMsg()			EasyWin32::To_MouseMsg(EasyWin32::GetMouseMsg_win32())
#define PeekMouseMsg			EasyWin32::PeekMouseMsg_win32_old
#define FlushMouseMsgBuffer		EasyWin32::FlushMouseMsgBuffer_win32

////////////****** �����궨�� ******////////////

#define rectangle_RECT(rct)			rectangle(rct.left,rct.top,rct.right,rct.bottom);
#define fillrectangle_RECT(rct)		fillrectangle(rct.left,rct.top,rct.right,rct.bottom);
#define solidrectangle_RECT(rct)	solidrectangle(rct.left,rct.top,rct.right,rct.bottom);


////////////****** �������� ******////////////


// ��ȷ��ʱ����(���Ծ�ȷ�� 1ms������ ��1ms)
// by yangw80<yw80@qq.com>, 2011-5-4
void HpSleep(int ms);

// �õ� IMAGE ����� HBITMAP
HBITMAP GetImageHBitmap(IMAGE* img);

// HBITMAP ת HICON
HICON HICONFromHBitmap(HBITMAP hBmp);

// ����ɫ����չ
enum COLORS {
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
	ORANGE = RGB(0xFF, 0xA5, 0x00),
	ORANGERED = RGB(0xFF, 0x45, 0x00),
	PINK = RGB(0xFF, 0xC0, 0xCB),
	PURPLE = RGB(0x80, 0x00, 0x80),
	SKYBLUE = RGB(0x87, 0xCE, 0xEB),
	SNOW = RGB(0xFF, 0xFA, 0xFA),
	SPRINGGREEN = RGB(0x00, 0xFF, 0x7F),
	STEELBLUE = RGB(0x46, 0x82, 0xB4),
	TOMATO = RGB(0xFF, 0x63, 0x47),
	WHITESMOKE = RGB(0xF5, 0xF5, 0xF5),
	YELLOWGREEN = RGB(0x9A, 0xCD, 0x32),

	// Windows �����
	CLASSICGRAY = RGB(0xF0, 0xF0, 0xF0)
};

