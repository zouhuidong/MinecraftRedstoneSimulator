/////////////////////////////////////////////////////////////////////
//
//	EasyWin32.h
//	基于 EasyX 图形库的 Win32 拓展库
//
//	作　　者：huidong <huidong_mail@163.com>
//	版　　本：Ver 2.6.0
//	编译环境：VisualStudio 2022 | EasyX_20220116 | Windows 10 
//	项目地址：https://github.com/zouhuidong/EasyWin32
//	创建日期：2020.12.06
//	最后修改：2022.04.03
//

#pragma once

#include <graphics.h>
#include <vector>
#include <string>
#include <thread>

// 补充绘图窗口初始化参数
#define EW_NORMAL 0

#define EASY_WIN32_BEGIN	namespace EasyWin32 {
#define EASY_WIN32_END		};

EASY_WIN32_BEGIN

////////////****** 结构体定义 ******////////////

// 窗口
struct EasyWindow
{
	HWND hWnd;							// 窗口句柄
	HWND hParent;						// 父窗口句柄
	IMAGE* pImg;						// 窗口图像
	IMAGE* pBufferImg;					// 图像缓冲区
										// 窗口消息处理函数
	bool(*funcWndProc)(HWND, UINT, WPARAM, LPARAM, HINSTANCE);
	std::vector<ExMessage> vecMouseMsg;	// 鼠标消息队列
	int nGetMouseMsgIndex;				// 获取鼠标消息的进度索引（现在获取到了数组中的哪一条）
	bool isNewSize;						// 窗口大小是否改变
	bool isSentCreateMsg;				// 是否模拟发送了 WM_CREATE 的消息
	bool isBusyProcessing;				// 是否正忙于处理内部消息
};

////////////****** 窗体相关函数 ******////////////

// 创建支持 win32 的绘图窗口（默认支持窗口双击消息）
// w, h				窗口大小
// isCmd			是否显示 cmd 窗口（如果是控制台应用程序）
// strWndTitle		窗口标题
// WindowProcess	窗口消息处理函数的指针，为空表示使用默认消息处理函数（详见头文件中“窗口消息处理函数规范”）
// hParent			父窗口句柄，为空则表示该窗口独立存在（若填写，该窗口将成为模态窗口）
HWND initgraph_win32(
	int w = 640,
	int h = 480,
	int flag = EW_NORMAL,
	LPCTSTR strWndTitle = L"",
	bool(*WindowProcess)(HWND, UINT, WPARAM, LPARAM, HINSTANCE) = NULL,
	HWND hParent = NULL
);

//
// 窗口消息处理函数规范
// 
// 函数标准形态：bool WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, HINSTANCE hInstance);
// 
// 注意：
// 相比于标准的 Win32 窗口过程函数，增加了一个 HINSTANCE 类型形参。
// 返回值的意义也不相同，见下文。
// 
// 返回值：
// true		表示使用系统默认方法处理该消息
// false	表示不再需要系统默认方法处理该消息
// 
// 注意事项：
// 1. 接受 WM_CREATE 消息时，wParam 和 lParam 是空的，你无法获得 CREATESTRUCT 结构体信息
// 2. 接受 WM_CLOSE 消息时，返回 true 或 false 表示是否关闭窗口，但如果关闭窗口，您无需编写销毁窗口的代码
//

// 关闭某一绘图窗口，若句柄为 NULL 则关闭所有绘图窗口
void closegraph_win32(HWND hWnd = NULL);

// 结束初始化窗口（阻塞）
void init_end();

// 设置：当窗口都被销毁时，自动退出程序
void AutoExit();

// 是否还存在未销毁的绘图窗口
bool isAnyWindow();

// 判断一窗口是否还存在（不含已被关闭的窗口）
bool isAliveWindow(HWND hWnd);

// 得到当前绘图窗口的句柄
HWND GetHWnd_win32();

// 阻塞等待当前绘图任务完成
void WaitForTask();

// 等待窗口内部消息处理完成
void WaitForProcessing(EasyWindow* pWnd);

// 得到当前绘图窗口的详细信息
EasyWindow GetWorkingWindow();

// 设置当前绘图窗口（同时设置绘图对象为窗口对应的 IMAGE 对象），返回是否设置成功
// 若有绘图任务进行中，则等待当前绘图任务完成
// （建议使用 BEGIN_TASK 宏）
bool SetWorkingWindow(HWND hWnd);

// 强制重绘当前绘图窗口（正常在 WM_PAINT 消息内绘图不需要使用此函数）
void EnforceRedraw();

// 宣告开始一次绘制（建议使用 BEGIN_TASK 宏）
void BeginTask();

// 输出绘图缓冲，并表示当前绘图任务告一段落
//（建议使用 FLUSH_DRAW 或 END_TASK 宏）
void EndTask();

// 获取已创建的窗口的数组（不含已被关闭的窗口）
std::vector<EasyWindow> GetCreatedWindowList();

// 判断窗口大小是否改变
bool isWindowSizeChanged(HWND hWnd = NULL);

// 判断自定义程序图标的启用状态
bool GetCustomIconState();

// 使用图标资源作为程序图标
// 参数传入图标资源 ID（大图标和小图标）
// 注：必须在第一次创建窗口前就调用该函数才能生效。默认情况下，程序将自绘 EasyX 程序图标
void SetCustomIcon(int nIcon, int nIconSm);

// 获取 EasyWin32 自绘默认窗口图标的 IMAGE
IMAGE GetDefaultIconImage();

// 获取当前窗口样式
long GetWindowStyle();

// 设置当前窗口样式
int SetWindowStyle(long lNewStyle);

// 获取当前窗口扩展样式
long GetWindowExStyle();

// 设置当前窗口扩展样式
int SetWindowExStyle(long lNewExStyle);

////////////****** 鼠标消息相关函数 ******////////////

//// MOUSEMSG 式函数

// 检查是否存在鼠标消息
bool MouseHit_win32();

// 阻塞等待，直到获取到一个新的鼠标消息
ExMessage GetMouseMsg_win32();

// 获取一个新的鼠标消息，立即返回是否获取成功
bool PeekMouseMsg_win32(ExMessage* pMsg, bool bRemoveMsg = true);

// 清空鼠标消息
void FlushMouseMsgBuffer_win32();

//// ExMessage 式函数

// 阻塞等待，直到获取到一个新消息（暂仅支持鼠标消息 EM_MOUSE）
ExMessage getmessage_win32(BYTE filter = -1);

// 阻塞等待，直到获取到一个新消息（暂仅支持鼠标消息 EM_MOUSE）
void getmessage_win32(ExMessage* msg, BYTE filter = -1);

// 获取一个消息，立即返回是否获取成功（暂仅支持鼠标消息 EM_MOUSE）
bool peekmessage_win32(ExMessage* msg, BYTE filter = -1, bool removemsg = true);

// 清除所有消息记录（暂仅支持鼠标消息 EM_MOUSE）
void flushmessage_win32(BYTE filter = -1);

//// 转换

// MOUSEMSG 转 ExMessage
ExMessage To_ExMessage(MOUSEMSG msg);

// ExMessage 转 MOUSEMSG
MOUSEMSG To_MouseMsg(ExMessage msgEx);

// 兼容旧版 MOUSEMSG
bool PeekMouseMsg_win32_old(MOUSEMSG* pMsg, bool bRemoveMsg = true);

//
//	鼠标消息兼容旧版 MOUSEMSG，同时支持 ExMessage
//	但 ExMessage 系列函数暂时只能获取 EM_MOUSE 即鼠标消息
//

EASY_WIN32_END

////////////****** 任务指令宏定义 ******////////////

// 启动一段（绘图）任务（绘制到当前绘图窗口）
#define BEGIN_TASK()\
	EasyWin32::WaitForTask();\
	if(EasyWin32::isAliveWindow(EasyWin32::GetHWnd_win32()))\
	{\
		EasyWin32::BeginTask()

// 启动一段（绘图）任务（指定目标绘图窗口）
#define BEGIN_TASK_WND(hWnd)\
	/* 设置工作窗口时将自动等待当前任务结束 */\
	if (EasyWin32::SetWorkingWindow(hWnd))\
	{\
		EasyWin32::BeginTask()

// 结束一段（绘图）任务，并输出绘图缓存（须与 BEGIN_TASK 连用）
#define END_TASK()\
		EasyWin32::EndTask();\
	}(0)	/* 此处强制要求加分号 */

// 要求窗口重绘
#define FLUSH_DRAW()			EasyWin32::EnforceRedraw()

////////////****** 窗口样式宏定义 ******////////////

// 是否禁用当前窗口改变大小
#define DisableResizing(b)		(b ? EasyWin32::SetWindowStyle(EasyWin32::GetWindowStyle() & ~WS_SIZEBOX & ~WS_MAXIMIZEBOX) :\
								EasyWin32::SetWindowStyle(EasyWin32::GetWindowStyle() | WS_SIZEBOX | WS_MAXIMIZEBOX))

////////////****** 键盘消息宏定义 ******////////////

// 判断当前活动窗口是否接受到某按键消息
#define KEY_DOWN(VK_NONAME) (GetForegroundWindow() == EasyWin32::GetHWnd_win32() && (GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

////////////****** EasyX 原生函数的宏替换 ******////////////

// 若使用 EasyX 原生函数创建窗口，则关闭窗口时自动退出程序
#define initgraph(w, h)			EasyWin32::initgraph_win32(w, h);\
								EasyWin32::AutoExit()

#define closegraph				EasyWin32::closegraph_win32

// 默认使用双缓冲，故 BeginBatchDraw 无意义
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

////////////****** 其他宏定义 ******////////////

#define rectangle_RECT(rct)			rectangle(rct.left,rct.top,rct.right,rct.bottom);
#define fillrectangle_RECT(rct)		fillrectangle(rct.left,rct.top,rct.right,rct.bottom);
#define solidrectangle_RECT(rct)	solidrectangle(rct.left,rct.top,rct.right,rct.bottom);


////////////****** 其他函数 ******////////////


// 精确延时函数(可以精确到 1ms，精度 ±1ms)
// by yangw80<yw80@qq.com>, 2011-5-4
void HpSleep(int ms);

// 得到 IMAGE 对象的 HBITMAP
HBITMAP GetImageHBitmap(IMAGE* img);

// HBITMAP 转 HICON
HICON HICONFromHBitmap(HBITMAP hBmp);

// 常用色彩扩展
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

	// Windows 经典灰
	CLASSICGRAY = RGB(0xF0, 0xF0, 0xF0)
};

