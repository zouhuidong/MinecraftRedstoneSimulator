#include "ProgressCtrl.h"

namespace HiEasyX
{
	void ProgressCtrl::Init()
	{
		m_bAlwaysRedrawAndRender = true;
	}

	ProgressCtrl::ProgressCtrl()
	{
		Init();
		SetBarColor(GREEN);
	}

	ProgressCtrl::ProgressCtrl(int x, int y, int w, int h, int len)
	{
		Init();
		m_nLen = len;
		SetRect(x, y, w, h);
		SetBarColor(GREEN);
	}

	void ProgressCtrl::SetContentLength(int len)
	{
		m_nLen = len;

		MarkNeedRedrawAndRender();
	}

	void ProgressCtrl::SetProcess(int pos)
	{
		m_nPos = pos;

		MarkNeedRedrawAndRender();
	}

	void ProgressCtrl::Step()
	{
		if (m_nPos + 1 <= m_nLen)
		{
			m_nPos++;
		}

		MarkNeedRedrawAndRender();
	}

	void ProgressCtrl::SetBarColor(COLORREF cBar)
	{
		float h, s, l;
		RGBtoHSL(cBar, &h, &s, &l);
		m_fH = h;
		m_fS = s;
		m_cBar = HSLtoRGB(h, s, m_fBarColorLightness);

		MarkNeedRedrawAndRender();
	}

	void ProgressCtrl::EnableAnimation(bool enable)
	{
		m_bEnableAnimation = enable;

		// ���ö����������ػ�
		m_bAlwaysRedrawAndRender = m_bEnableAnimation;

		MarkNeedRedrawAndRender();
	}

	void ProgressCtrl::Draw(bool draw_child)
	{
		if (m_bRedraw)
		{
			ControlBase::Draw(false);

			// ���ƽ���λ��
			int nEnd = (int)((m_nPos / (float)m_nLen) * (GetWidth() - 2));

			m_canvas.SolidRectangle(
				{ 1,1,nEnd,GetHeight() - 2 },
				true, m_cBar
			);

			// ���ƶ���Ч��
			if (m_bEnableAnimation)
			{
				// ��������
				if (m_tClock == 0)	m_tClock = clock();
				float fProcess = (
					((clock() - m_tClock) / (float)CLOCKS_PER_SEC)	// ����ʱ��
					/ (1 / m_fSpeedRatio)							// ��ʱ��
					);
				if (fProcess >= 1)
				{
					fProcess = 0;
					m_tClock = clock();
				}

				int nAnimationLen = (int)(m_nPos / (float)m_nLen * GetWidth() * m_fLenRatio);		// ����Ч������
				int nAnimationLenHalf = nAnimationLen / 2;										// ����Ч���볤
				int draw_pos = (int)((nEnd + nAnimationLen) * fProcess) - nAnimationLenHalf;	// ��Դ���� x ����
				for (int i = -nAnimationLenHalf; i < nAnimationLenHalf; i++)
				{
					int pos = i + draw_pos;		// ��ǰ���� x ����

					// ��ǰ��������
					float l = (m_fBarColorLightness - m_fBarLightLightness) * (abs(i) / (float)nAnimationLenHalf) + m_fBarLightLightness;
					if (pos >= 1 && pos <= nEnd)
					{
						m_canvas.Line(
							pos, 1, pos, GetHeight() - 2,
							true, HSLtoRGB(m_fH, m_fS, l)
						);
					}
				}
			}
		}

		if (draw_child)
		{
			DrawChild();
		}
	}
}
