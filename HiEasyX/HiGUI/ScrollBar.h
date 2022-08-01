////////////////////////////////////
//
//	ScrollBar.h
//	HiGUI �ؼ���֧���������ؼ�
//

#pragma once

#include "Button.h"

#include "../HiMouseDrag.h"

#include <time.h>

namespace HiEasyX
{
	// �������ؼ�
	class ScrollBar : public ControlBase
	{
	public:

		// ��������Ϣ
		struct ScrollBarInfo
		{
			int btnW, btnH;			// ��ͨ��ť�ߴ�
			int slider_free_len;	// �������ռ�ݵ����س���
			int slider_move_len;	// ���鶥������ƶ������س���
			int slider_len;			// �������س���
		};

	protected:

		int m_nBtnHeight = 20;				// ��ť�߶ȣ���ֱ����ʱ��Ч��
		int m_nBtnWidth = 20;				// ��ť��ȣ�ˮƽ����ʱ��Ч��

		bool m_bHorizontal = false;			// �Ƿ�ˮƽ����

		int m_nDrawInterval = 6;			// ���Ƽ�϶

		// ��ť
		Button m_btnTop;
		Button m_btnUp;
		Button m_btnDown;
		Button m_btnBottom;
		Button m_btnDrag;
		MouseDrag m_MouseDrag;
		bool m_bDragging = false;			// �Ƿ������϶�
		int m_nSliderSpeed = 20;			// ��ס��ťʱ����ÿ�����е����ݳ���
		clock_t m_tPressed = 0;				// ��ť��ס��ʱ

		float m_fPos = 0;					// ��������λ��
		int m_nLen = 0;						// �����ܳ���
		float m_fPosRatio = 0;				// ��������λ�ñ���

		ScrollBarInfo m_info = {};			// ��������Ϣ

		int m_nViewLen = 10;				// ��Ұ���ݳ���
		float m_fViewRatio = 1;				// ��Ұ��Χռ�ܳ��ȵı�

		bool m_bSliderPosChanged = false;	// ��ǻ���λ�øı�

		RECT m_rctOnWheel = { 0 };			// ��Ӧ������Ϣ�����򣨿ͻ������꣩
		bool m_bSetOnWheelRct = false;		// �Ƿ������˹�����Ϣ����Ӧ����

		// ��ʼ��
		virtual void Init();

		// ��Ӧ���ⰴť��Ϣ��Top �� Bottom��
		static void OnSpecialButtonMsg(void* pThis, ControlBase* pCtrl, int msgid, ExMessage msg);

		// ��Ӧ��ͨ��ť��Ϣ��Up �� Down��
		virtual void OnNormalButtonMsg();

		// ���»���λ������
		virtual void UpdateSliderRect();

		// ����λ�ñ���
		void UpdatePosRatio();

		// ������Ұ����
		void UpdateViewRatio();

		// ���¹�������Ϣ
		void UpdateScrollBarInfo();

	public:

		ScrollBar();

		ScrollBar(int x, int y, int w, int h, int len, int pos, bool bHorizontal = false);

		int GetButtonHeight() const { return m_nBtnHeight; }

		// ���ð�ť�߶ȣ���ֱ����ʱ��Ч��
		void SetButtonHeight(int h);

		int GetButtonWidth() const { return m_nBtnWidth; }

		// ���ð�ť��ȣ�ˮƽ����ʱ��Ч��
		void SetButtonWidth(int w);

		// �Ƿ������϶�
		bool isDragging() const { return m_bDragging; }

		// ��ȡ�������س���
		int GetSliderLength() const { return m_info.slider_len; }

		// ��ȡ��������λ��
		int GetSliderContentPos() const { return (int)m_fPos; }

		// ���û�������λ��
		void SetSliderContentPos(float pos);

		// �ƶ����������λ��
		void MoveSlider(float d);

		// ��ȡ���������ݳ���
		int GetContentLength() const { return m_nLen; }

		// ���ù��������ݳ���
		void SetContentLength(int len);

		int GetViewLength() const { return m_nViewLen; }

		// ������Ұ���ݳ���
		void SetViewLength(int len);

		int GetSliderSpeed() const { return m_nSliderSpeed; }

		// ���ð��°�ťʱ����������ٶȣ�ÿ�뾭�������ݳ��ȣ�
		void SetSliderSpeed(int speed);

		bool isHorizontal() const { return m_bHorizontal; }

		// ����ˮƽ����
		void EnableHorizontal(bool enable);

		// ����λ���Ƿ�ı�
		bool isSliderPosChanged();

		// ��ȡ��Ӧ������Ϣ������δ�Զ���ʱ���ؿ�����
		RECT GetOnWheelRect() const { return m_rctOnWheel; }

		// ������Ӧ������Ϣ�����򣨿ͻ������꣩
		void SetOnWheelRect(RECT rct);

		void UpdateRect(RECT rctOld) override;

		void UpdateMessage(ExMessage msg) override;

		void Draw(bool draw_child = true) override;

	};
}

