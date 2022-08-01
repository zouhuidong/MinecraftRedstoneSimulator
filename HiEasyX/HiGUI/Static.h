////////////////////////////////////
//
//	Static.h
//	HiGUI �ؼ���֧����̬�ؼ�
//

#pragma once

#include "ControlBase.h"

namespace HiEasyX
{
	// ��̬���ı���ͼ�񣩿ؼ�
	class Static : public ControlBase
	{
	public:

		struct Char
		{
			TCHAR ch;
			COLORREF cText;
			COLORREF cBk;
		};

	protected:

		std::vector<Char> m_vecText;
		ImageBlock* m_pImgBlock = nullptr;

		virtual void Init();

	public:

		Static();

		Static(int x, int y, int w, int h, std::wstring wstrText = L"");

		virtual std::vector<Char> Convert(std::wstring wstrText);

		virtual std::wstring Convert(std::vector<Char> vecText);

		virtual std::vector<Char>& GetTextVector() { return m_vecText; }

		virtual void ClearText();

		// ����ı�����������ɫ������������ʹ�õ�ǰ��ɫ��
		virtual void AddText(
			std::wstring wstr,
			bool isSetTextColor = false,
			COLORREF cText = 0,
			bool isSetBkColor = false,
			COLORREF cBk = 0
		);

		void SetText(std::wstring wstrText) override;

		void SetText(std::vector<Char> vecText);

		// ��ȡ�����ͼ��
		virtual ImageBlock* GetImage() { return m_pImgBlock; }

		// ������ʾͼ��
		virtual void SetImage(ImageBlock* pImgBlockmg);

		void Draw_Text(int nTextOffsetX = 0, int nTextOffsetY = 0) override;

		void Draw(bool draw_child = true) override;
	};
}

