////////////////////////////////////
//
//	ControlBase.h
//	HiGUI �ؼ���֧���ؼ�����
//

#pragma once

#include "Container.h"

#include "../HiMacro.h"
#include "../HiFunc.h"
#include "../HiCanvas.h"

namespace HiEasyX
{
	// �ؼ���Ϣ
	enum CtrlMessage
	{
		CM_OTHER,						// δ�����ʶ��������Ϣ
		CM_HOVER,						// ��ͣ
		CM_HOVER_OVER,					// ��ͣ����
		CM_PRESS,						// ����
		CM_PRESS_OVER,					// ���½���
		CM_CLICK,						// ����
		CM_DOUBLE_CLICK,				// ˫��
		CM_FOCUS,						// ��ȡ����
		CM_FOCUS_OVER,					// ��ʧ����

	};

	class ControlBase;

	// �ؼ���Ϣ������
	// _Ctrl	����ؼ�ָ��
	// _MsgId	������Ϣ��ʶ����
	// _ExMsg	������ϸ��Ϣ�������ѱ任���ؼ���
	typedef void (*MESSAGE_PROC_FUNC)(ControlBase* _Ctrl, int _MsgId, ExMessage _ExMsg);

	// ֧�־�̬�ຯ����Ϊ�ؼ���Ϣ������
	typedef void (*MESSAGE_PROC_FUNC_CLASS)(void* _This, ControlBase* _Ctrl, int _MsgId, ExMessage _ExMsg);

	// �ؼ�����
	class ControlBase : public Container
	{
	protected:

		bool m_bEnabled = true;										// �Ƿ����
		bool m_bVisible = true;										// �Ƿ�ɼ�

		// �ػ����Ⱦ��־
		bool m_bAutoRedrawWhenReceiveMsg = true;					// ��Ĭ����Ϣ���������ܵ���Ϣʱ���Ƿ��Զ���ʶ�ػ����Ⱦ
		bool m_bRedraw = true;										// ��ʶ��Ҫ�ػ�
		bool m_bRender = true;										// ��ʶ��Ҫ��Ⱦ
		bool m_bClear = false;										// ��ʶ��Ҫ���ĳ����
		RECT m_rctClear = { 0 };									// ��¼��Ҫ��յ�����
		bool m_bAlwaysRedrawAndRender = false;						// �����ػ����Ⱦ��ռ�ø��ߣ�

		std::wstring m_wstrText;									// �ؼ��ı�

		Canvas m_canvas;											// ����
		BYTE m_alpha = 255;											// ͸����
		bool m_bUseCanvasAlpha = false;								// �Ƿ�ʹ�û��������͸������Ϣ
		bool m_isAlphaCalculated = false;							// �����Ƿ��Ѿ�����͸�������ɫ

		COLORREF m_cBorder = MODEN_BORDER_GRAY;						// �߿���ɫ
		COLORREF m_cBackground = CLASSICGRAY;						// ����ɫ
		COLORREF m_cText = BLACK;									// �ı���ɫ

		bool m_bEnableBorder = true;								// �Ƿ���Ʊ߿�
		int m_nBorderThickness = 1;									// �߿��ϸ

		bool m_bCompleteFirstSetRect = false;						// �Ƿ��Ѿ���ɵ�һ����������

		ControlBase* m_pParent = nullptr;							// ���ؼ�
		std::list<ControlBase*> m_listChild;						// �ӿؼ�

		bool m_bAutoSizeForChild = false;							// Ϊ�ӿؼ��Զ��ı��С�����ɿؼ�

		MESSAGE_PROC_FUNC m_funcMessageProc = nullptr;				// ��Ϣ������
		MESSAGE_PROC_FUNC_CLASS m_funcMessageProc_Class = nullptr;	// ���󶨵���Ϣ�������Ǿ�̬�ຯ�������¼���ַ
		void* m_pCalledClass = nullptr;								// ���󶨵���Ϣ�������Ǿ�̬�ຯ�������¼����ָ��

		bool m_bHovered = false;									// ����Ƿ���ͣ
		bool m_bPressed = false;									// ����Ƿ���
		bool m_bFocused = false;									// �Ƿ�ӵ�н���

		// ����������Ϣ����
		void UpdateRect(RECT rctOld) override;

		// �����Ҫ�ػ����Ⱦ
		void MarkNeedRedrawAndRender();

		// �����Ҫ��վ�������
		void MarkNeedClearRect(RECT rct);

		// �����ӿؼ�
		virtual void DrawChild();

		// ת����Ϣ
		virtual ExMessage& TransformMessage(ExMessage& msg);

		// �ַ���Ϣ���û�����
		virtual void CallUserMsgProcFunc(int msgid, ExMessage msg);

		// �ӿؼ��������
		virtual void ChildRectChanged(ControlBase* pChild);

	private:

		void Init();

	public:

		ControlBase();

		ControlBase(std::wstring wstrText);

		ControlBase(int x, int y, int w = 0, int h = 0, std::wstring wstrText = L"");

		virtual ~ControlBase();

		ControlBase* GetParent() { return m_pParent; }

		// ���ø��ؼ����ڲ����� AddChild��
		virtual void SetParent(ControlBase* p);

		virtual bool isAutoSizeForChild() const { return m_bAutoSizeForChild; }

		// Ϊ�ӿؼ��Զ��ı��С�����ɿؼ��������ɸ����겿�֣�
		virtual void EnableAutoSizeForChild(bool enable);

		std::list<ControlBase*>& GetChildList();

		// ��ȡ�ӿؼ�����
		int GetChildCount();

		virtual void AddChild(ControlBase* p, int offset_x = 0, int offset_y = 0);

		virtual void RemoveChild(ControlBase* p);

		virtual bool isEnabled() const { return m_bEnabled; }

		virtual void SetEnable(bool enable);

		virtual bool isVisible() const { return m_bVisible; }

		virtual void SetVisible(bool visible);

		virtual bool GetAutoRedrawState() const { return m_bAutoRedrawWhenReceiveMsg; }

		// �����Զ��ػ棨���ܵ�������Ϣ�¼�ʱ�Զ���ʶ��Ҫ�ػ棩
		virtual void EnableAutoRedraw(bool enable);

		virtual Canvas& GetCanvas() { return m_canvas; }

		virtual COLORREF GetBkColor() const { return m_cBackground; }

		virtual void SetBkColor(COLORREF color);

		virtual COLORREF GetTextColor() const { return m_cText; }

		virtual void SetTextColor(COLORREF color);

		virtual void EnableBorder(bool bEnableBorder, COLORREF color = BLACK, int thickness = 1);

		virtual void SetAlpha(BYTE alpha, bool bUseCanvasAlpha, bool isAlphaCalculated);

		virtual std::wstring GetText() const { return m_wstrText; }

		virtual void SetText(std::wstring wstr);

		virtual void Draw_Text(int nTextOffsetX = 0, int nTextOffsetY = 0);

		// �ػ�ؼ�
		virtual void Redraw();

		// ���ƿؼ�����ѡ�����ӿؼ���
		virtual void Draw(bool draw_child = true);

		// ��Ⱦ�ؼ����ⲿ
		virtual void Render(Canvas* dst, RECT* pRct = nullptr, int* pCount = 0);

		// ������Ϣ��Ӧ����
		virtual void SetMsgProcFunc(MESSAGE_PROC_FUNC func);

		// ������Ϣ��Ӧ����Ϊ��̬�ຯ��
		virtual void SetMsgProcFunc(MESSAGE_PROC_FUNC_CLASS static_class_func, void* _this);

		// ������Ϣ
		virtual void UpdateMessage(ExMessage msg);

		// �Ƿ���ͣ
		virtual bool isHovered() const { return m_bHovered; }

		// �Ƿ�ӵ�н���
		virtual bool isFocused() const { return m_bFocused; }

		// �Ƿ���
		virtual bool isPressed() const { return m_bPressed; }
	};
}

