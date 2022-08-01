////////////////////////////
//
//	HiMouseDrag.h
//	HiEasyX �������϶�����ģ��
//

#pragma once

#include <graphics.h>

namespace HiEasyX
{

	// ����϶���Ϣ
	// ���÷�����
	//		��Ҫ�������Ϣѭ����ÿ�ζ����� UpdateMessage ���������Ϣ
	//		���� isLeftDrag��isMiddleDrag��isRightDrag �����ж������϶�����갴��
	//		���� GetDragX��GetDragY ��ȡ����϶�ʱ�������ı仯��
	class MouseDrag
	{
	private:
		ExMessage old = { 0 }, msg = { 0 };
		int dx = 0, dy = 0;
		bool lbtn = false, mbtn = false, rbtn = false;
		bool ldrag = false, mdrag = false, rdrag = false;
		bool newmsg = false;

		bool UpdateDragInfo(bool& btn, int msgid_down, int msgid_up);
		
	public:

		void UpdateMessage(ExMessage m);	// ���������Ϣ

		bool isLeftDrag();					// �������Ƿ��϶�
		bool isMiddleDrag();				// ����м��Ƿ��϶�
		bool isRightDrag();					// ����Ҽ��Ƿ��϶�

		int GetDragX();						// ��ȡ�϶��� x ����ƫ����
		int GetDragY();						// ��ȡ�϶��� y ����ƫ����
	};

};
