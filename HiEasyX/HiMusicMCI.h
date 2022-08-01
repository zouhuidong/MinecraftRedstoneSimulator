/////////////////////////////////
//
//	HiMusicMCI.h
//	HiEasyX �������ģ��
//	
//	ԭ���ߣ���Զ�Ĳ�� <2237505658@qq.com>
//

#pragma once

#include <Windows.h>

/************************************************
 *                class MusicMCI				*
 * ���ֲ��Ų�����									*
 * ���еײ����ֲ���								*
 * ��ͣ���رգ�������������ȡ����ʱ��Ȳ���			*
 ************************************************/

namespace HiEasyX
{

	class MusicMCI
	{
	private:
		MCIDEVICEID nDeviceID;									// �豸ID

	public:
		MusicMCI()noexcept;										// Ĭ�Ϲ��캯��
		virtual ~MusicMCI();									// ����������
		bool open(LPCWSTR music)noexcept;						// ������
		bool play()noexcept;									// ��������
		bool pause()noexcept;									// ��ͣ����
		bool stop()noexcept;									// ֹͣ����
		bool close()noexcept;									// �ر�����
		bool getCurrentTime(DWORD& pos)noexcept;				// ��ȡ��ǰ����ʱ��
		bool getTotalTime(DWORD& time)noexcept;					// ��ȡ������ʱ��
		bool setVolume(size_t volume)noexcept;					// ����������С
		bool setStartTime(size_t start)noexcept;				// ���ò���λ��
	};

};
