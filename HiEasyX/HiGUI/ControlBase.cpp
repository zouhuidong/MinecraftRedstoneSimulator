#include "ControlBase.h"

namespace HiEasyX
{
	void ControlBase::Init()
	{
		// TODO���� EasyX �޸������
		/*m_canvas.SetTypeface(L"Arial");
		m_canvas.SetBkColor(m_cBackground);
		m_canvas.SetTextColor(m_cText);
		m_canvas.SetLineColor(m_cBorder);*/
	}

	ControlBase::ControlBase()
	{
		Init();
	}

	ControlBase::ControlBase(std::wstring wstrText)
	{
		Init();
		SetText(wstrText);
	}

	ControlBase::ControlBase(int x, int y, int w, int h, std::wstring wstrText)
	{
		Init();
		SetRect(x, y, w, h);
		SetText(wstrText);
	}

	ControlBase::~ControlBase()
	{
	}

	void ControlBase::UpdateRect(RECT rctOld)
	{
		Container::UpdateRect(rctOld);

		m_canvas.Resize(GetWidth(), GetHeight());

		// ��ʶ�ػ����Ⱦ
		MarkNeedRedrawAndRender();

		// ���ǵ�һ������λ��ʱ����Ҫ���������
		if (m_bCompleteFirstSetRect)
		{
			// ��ʶ��վ�����
			MarkNeedClearRect(rctOld);
		}
		else
		{
			m_bCompleteFirstSetRect = true;
		}

		if (m_pParent)
		{
			m_pParent->ChildRectChanged(this);
		}
	}

	void ControlBase::MarkNeedRedrawAndRender()
	{
		m_bRedraw = true;
		m_bRender = true;

		// �ػ����ȾʱҲҪ��Ⱦ�ӿؼ�������Ḳ��
		for (auto& child : m_listChild)
			child->m_bRender = true;
	}

	void ControlBase::MarkNeedClearRect(RECT rct)
	{
		m_bClear = true;
		m_rctClear = rct;
	}

	void ControlBase::SetEnable(bool enable)
	{
		m_bEnabled = enable;

		MarkNeedRedrawAndRender();
	}

	void ControlBase::SetParent(ControlBase* p)
	{
		if (m_pParent)
		{
			m_pParent->RemoveChild(this);
		}
		if (p)
		{
			m_pParent = p;
			p->AddChild(this);
		}

		MarkNeedRedrawAndRender();
	}

	void ControlBase::EnableAutoSizeForChild(bool enable)
	{
		m_bAutoSizeForChild = enable;
	}

	std::list<ControlBase*>& ControlBase::GetChildList()
	{
		return m_listChild;
	}

	int ControlBase::GetChildCount()
	{
		size_t sum = m_listChild.size();
		for (auto& child : m_listChild)
			sum += child->GetChildCount();
		return sum;
	}

	void ControlBase::AddChild(ControlBase* p, int offset_x, int offset_y)
	{
		for (auto& child : m_listChild)
			if (child == p)
				return;
		p->MoveRel(offset_x, offset_y);
		p->m_pParent = this;
		m_listChild.push_back(p);
		ChildRectChanged(p);

		MarkNeedRedrawAndRender();
	}

	void ControlBase::RemoveChild(ControlBase* p)
	{
		m_listChild.remove(p);

		MarkNeedRedrawAndRender();
	}

	void ControlBase::SetVisible(bool bVisible)
	{
		m_bVisible = bVisible;

		// ��Ϊ�ɼ�ʱ������Ⱦ
		if (m_bVisible)
		{
			m_bRender = true;
		}
		// ��Ϊ���ɼ�ʱ���
		else
		{
			MarkNeedClearRect(m_rct);
		}
	}

	void ControlBase::EnableAutoRedraw(bool enable)
	{
		m_bAutoRedrawWhenReceiveMsg = enable;
	}

	void ControlBase::SetBkColor(COLORREF color)
	{
		m_cBackground = color;
		m_canvas.SetBkColor(color);

		MarkNeedRedrawAndRender();
	}

	void ControlBase::SetTextColor(COLORREF color)
	{
		m_cText = color;
		m_canvas.SetTextColor(color);

		MarkNeedRedrawAndRender();
	}

	void ControlBase::EnableBorder(bool bEnableBorder, COLORREF color, int thickness)
	{
		m_bEnableBorder = bEnableBorder;
		if (bEnableBorder)
		{
			m_cBorder = color;
			m_nBorderThickness = thickness;

			m_canvas.SetLineColor(m_cBorder);
		}

		MarkNeedRedrawAndRender();
	}

	void ControlBase::SetAlpha(BYTE alpha, bool bUseCanvasAlpha, bool isAlphaCalculated)
	{
		m_alpha = alpha;
		m_bUseCanvasAlpha = bUseCanvasAlpha;
		m_isAlphaCalculated = isAlphaCalculated;

		MarkNeedRedrawAndRender();
	}

	void ControlBase::SetText(std::wstring wstr)
	{
		m_wstrText = wstr;

		MarkNeedRedrawAndRender();
	}

	void ControlBase::Draw_Text(int nTextOffsetX, int nTextOffsetY)
	{
		m_canvas.SetBkColor(m_cBackground);
		m_canvas.SetTextColor(m_cText);
		int w = m_canvas.TextWidth(m_wstrText.c_str());
		int h = m_canvas.TextHeight(m_wstrText.c_str());
		m_canvas.OutTextXY(
			(GetWidth() - w) / 2 + nTextOffsetX,
			(GetHeight() - h) / 2 + nTextOffsetY,
			m_wstrText.c_str()
		);
	}

	void ControlBase::Redraw()
	{
		MarkNeedRedrawAndRender();
		Draw();
	}

	void ControlBase::Draw(bool draw_child)
	{
		if (m_bRedraw || m_bAlwaysRedrawAndRender)
		{
			m_canvas.SetLineThickness(m_nBorderThickness);
			m_canvas.FillRectangle(
				0, 0, GetWidth() - 1, GetHeight() - 1,
				true, m_bEnableBorder ? m_cBorder : m_cBackground, m_cBackground
			);

			if (!m_bAlwaysRedrawAndRender)
			{
				m_bRedraw = false;
			}
		}

		if (draw_child)
		{
			DrawChild();
		}
	}

	void ControlBase::DrawChild()
	{
		for (auto& child : m_listChild)
		{
			child->Draw();
		}
	}

	void ControlBase::Render(Canvas* dst, RECT* pRct, int* pCount)
	{
		// ��ǰ�ؼ���ȡ�����ػ���μ�¼
		int size = m_bRender ? 1 : GetChildCount();		// ��������Ҫ���ƣ���ֻ��Ҫ��¼һ������
		RECT* my_rct = new RECT[size];
		int my_count = 0;								// ��ǰ�ؼ���ȡ�����ػ���������ͳ��

		// Ϊ��ǰ�ؼ����ӿؼ�ִ���������
		// ������ Render ǰִ�У�������ܸ���Ч��
		for (auto& child : m_listChild)
		{
			if (child->m_bClear)
			{
				m_canvas.ClearRectangle(child->m_rctClear);
				MOVE_RECT(child->m_rctClear, m_rct.left, m_rct.top);
				dst->ClearRectangle(child->m_rctClear);

				child->m_bClear = false;
				child->m_rctClear = { 0 };
			}
		}

		// �ؼ��������ʾ����Ⱦ
		if (m_bVisible)
		{
			// �ӿؼ����ܺܶ࣬������������
			m_canvas.BeginBatchDrawing();

			// ���������Ҫ��Ⱦ����ֻ��Ҫ��¼�������
			if (m_bRender || m_bAlwaysRedrawAndRender)
			{
				// �ӿؼ��Ȼ��Ƶ��˿ؼ������ϣ��ٻ��Ƶ� dst
				for (auto& child : m_listChild)
					child->Render(&m_canvas);

				my_rct[my_count++] = { 0,0,GetWidth(),GetHeight() };

				if (!m_bAlwaysRedrawAndRender)
					m_bRender = false;
			}

			// ��������Ⱦ��������¼�ӿؼ���Ⱦ����
			else
			{
				for (auto& child : m_listChild)
				{
					child->Render(&m_canvas, my_rct, &my_count);
				}
			}

			m_canvas.EndBatchDrawing();

			// ��Ⱦ�и��µ�����
			for (int i = 0; i < my_count; i++)
			{
				dst->PutImageIn_Alpha(
					m_rct.left, m_rct.top,
					&m_canvas,
					my_rct[i],
					m_alpha, m_bUseCanvasAlpha, m_isAlphaCalculated
				);
			}

			// Ϊ�˷���������Ϣ�����ؼ�����Ҫת����������ο�ϵ
			if (pRct)
			{
				for (int i = 0; i < my_count; i++)
				{
					MOVE_RECT(my_rct[i], m_rct.left, m_rct.top);
					pRct[(*pCount)++] = my_rct[i];
				}
			}
		}

		delete[] my_rct;

		/*for (auto& child : m_listChild)
			child->Render(&m_canvas);
		dst->PutImageIn_Alpha(
			m_rct.left, m_rct.top,
			&m_canvas,
			{ 0 },
			m_alpha, m_bUseCanvasAlpha, m_isAlphaCalculated
		);*/
	}

	void ControlBase::SetMsgProcFunc(MESSAGE_PROC_FUNC func)
	{
		m_funcMessageProc = func;
		m_funcMessageProc_Class = nullptr;
		m_pCalledClass = nullptr;
	}

	void ControlBase::SetMsgProcFunc(MESSAGE_PROC_FUNC_CLASS static_class_func, void* _this)
	{
		m_funcMessageProc = nullptr;
		m_funcMessageProc_Class = static_class_func;
		m_pCalledClass = _this;
	}

	ExMessage& ControlBase::TransformMessage(ExMessage& msg)
	{
		switch (GetExMessageType(msg))
		{
		case EM_MOUSE:
			msg.x -= GetX();
			msg.y -= GetY();
			break;
		default:
			break;
		}
		return msg;
	}

	void ControlBase::CallUserMsgProcFunc(int msgid, ExMessage msg)
	{
		if (m_funcMessageProc)
		{
			m_funcMessageProc(this, msgid, msg);
		}
		else if (m_funcMessageProc_Class)
		{
			m_funcMessageProc_Class(m_pCalledClass, this, msgid, msg);
		}
	}

	void ControlBase::ChildRectChanged(ControlBase* pChild)
	{
		if (m_bAutoSizeForChild)
		{
			int _w = GetWidth(), _h = GetHeight();
			if (pChild->m_rct.right > _w)		_w += pChild->m_rct.right - _w;
			if (pChild->m_rct.bottom > _h)		_h += pChild->m_rct.bottom - _h;
			Resize(_w, _h);
		}
	}

	void ControlBase::UpdateMessage(ExMessage msg)
	{
		if (m_bVisible && m_bEnabled)
		{
			TransformMessage(msg);

			// ��ʶ����Ϣ�Ƿ�ֵ���ػ�
			bool msg_worth_redraw = false;

			// �����������
			if (isInRect(msg.x, msg.y, { 0,0,GetWidth(),GetHeight() }))
			{
				// �������
				if (!m_bHovered)
				{
					m_bHovered = true;
					CallUserMsgProcFunc(CM_HOVER, msg);
				}

				switch (msg.message)
				{
				case WM_LBUTTONDOWN:
				{
					m_bPressed = true;
					CallUserMsgProcFunc(CM_PRESS, msg);
					if (!m_bFocused)
					{
						m_bFocused = true;
						CallUserMsgProcFunc(CM_FOCUS, msg);
					}
				}
				break;

				case WM_LBUTTONUP:
				{
					m_bPressed = false;
					if (m_bFocused)
					{
						CallUserMsgProcFunc(CM_PRESS_OVER, msg);
						CallUserMsgProcFunc(CM_CLICK, msg);
					}
				}
				break;

				case WM_LBUTTONDBLCLK:
				{
					CallUserMsgProcFunc(CM_DOUBLE_CLICK, msg);
				}
				break;

				default:
				{
					CallUserMsgProcFunc(CM_OTHER, msg);
				}
				break;

				}

				msg_worth_redraw = true;
			}

			// ��겻��������
			else
			{
				// �Ƴ�
				if (m_bHovered)
				{
					m_bHovered = false;
					CallUserMsgProcFunc(CM_HOVER_OVER, msg);

					msg_worth_redraw = true;
				}

				// �뿪���򣬰���ʧЧ
				if (m_bPressed)
				{
					m_bPressed = false;
					CallUserMsgProcFunc(CM_PRESS_OVER, msg);

					msg_worth_redraw = true;
				}

				switch (msg.message)
				{
				case WM_LBUTTONDOWN:
				{
					if (m_bFocused)
					{
						m_bFocused = false;
						CallUserMsgProcFunc(CM_FOCUS_OVER, msg);

						msg_worth_redraw = true;
					}
				}
				break;

				default:
				{
					CallUserMsgProcFunc(CM_OTHER, msg);
				}
				break;

				}
			}

			// Ĭ���ػ棬����������
			if (m_bAutoRedrawWhenReceiveMsg && msg_worth_redraw)
			{
				MarkNeedRedrawAndRender();
			}

			for (auto& child : m_listChild)
			{
				child->UpdateMessage(msg);
			}
		}
	}
}
