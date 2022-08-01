////////////////////////////////////
//
//	Page.h
//	HiGUI �ؼ���֧��ҳ�ؼ�
//

#pragma once

#include "ControlBase.h"

namespace HiEasyX
{
	// ҳ�ؼ�
	class Page : public ControlBase
	{
	protected:

		Canvas* m_pCanvas = nullptr;

		virtual void Init(COLORREF cBk = WHITE);

	public:

		Page();

		Page(int w, int h, COLORREF cBk = WHITE);

		Page(Canvas* pCanvas);

		// �󶨵�����
		virtual void BindToCanvas(Canvas* pCanvas);

		// ����ؼ�
		virtual void push(ControlBase* pCtrl, int offset_x = 0, int offset_y = 0);

		virtual void push(const std::list<ControlBase*> list);

		// �Ƴ��ؼ�
		virtual void remove(ControlBase* pCtrl);

		// ��Ⱦ�����Ѱ󶨻�������Ĭ��������󶨻�����
		void Render(Canvas* dst = nullptr, RECT* pRct = nullptr, int* pCount = 0) override;

		// ���»�����...�����Ѱ󶨻�������Ĭ��������󶨻�����
		void UpdateImage(Canvas* pCanvas = nullptr);
	};
}

