////////////////////////////
//
//	HiDrawingProperty.h
//	HiEasyX ��Ļ�ͼ���Դ洢ģ��
//

#pragma once

#include <graphics.h>

namespace HiEasyX
{

	// ��ͼ����
	// setorigin �� setcliprgn �Ƕ��ڻ�ͼ�豸��˵�ģ��ʲ�����
	class DrawingProperty
	{
	private:
		bool m_isSaved = false;

	public:
		IMAGE* m_pImg;
		float m_xasp, m_yasp;
		COLORREF m_cBk;
		int m_nBkMode;
		COLORREF m_cFill;
		FILLSTYLE m_fillstyle;
		COLORREF m_cLine;
		LINESTYLE m_linestyle;
		int m_nPolyFillMode;
		int m_nRop2Mode;
		COLORREF m_cText;
		LOGFONT m_font;

		void SaveProperty();				// ���浱ǰ���еĻ�ͼ����
		void SaveWorkingImageOnly();		// ֻ���浱ǰ��ͼ����
		void ApplyProperty();				// Ӧ�ñ�������л�ͼ����
		void ApplyWorkingImageOnly();		// ֻ�ָ���ͼ����
		bool isSaved();						// �Ƿ񱣴��˻�ͼ����
		void Reset();						// ���ñ���״̬

	};

};
