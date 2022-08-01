//////////////////////////
//
//	HiGif.h
//	HiEasyX ��Ķ�ͼģ��
//	
//	ԭ���ߣ���ϡ_yixy
//

#pragma once

#include <graphics.h>
#include <gdiplus.h>
#include <time.h>
#include <stdio.h>

namespace HiEasyX
{
	// ���ء����� GIF
	// 
	// 1. load ����ͼ��
	// 2. bind ����� HDC 
	// 3. ����ѡ��setPos ����λ��
	// 4. ����ѡ��setSize �������Ŵ�С��Ϊ 0 ��ʾԭͼ��С��
	// 5. play ��ʼ����
	// 6. draw ���Ƶ� HDC
	//
	// ��֪ Bug
	// 1. �ͷ�ʱ���ܱ���
	// 2. �԰󶨵� HDC ������С���ܵ��±���
	class Gif
	{
	private:
		int x, y;
		int width, height;
		int frameCount;					// ֡��

		HDC hdc;						// �豸���
		Gdiplus::Graphics* graphics;	// ͼ�ζ���

		Gdiplus::Bitmap* gifImage;		// gif ͼ��
		Gdiplus::PropertyItem* pItem;	// ֡��ʱ����

		int curFrame;					// ��ǰ֡
		clock_t pauseTime;				// ��ͣʱ��

		clock_t	frameBaseTime;			// ֡��׼ʱ��
		clock_t	curDelayTime;			// ��ǰ֡���Ѳ���ʱ��
		clock_t	frameDelayTime;			// ��ǰ֡������ʱʱ��

		bool playing;					// �Ƿ񲥷�
		bool visible;					// �Ƿ�ɼ�

	public:
		Gif(const WCHAR* gifFileName = nullptr, HDC hdc = GetImageHDC());
		Gif(const Gif& gif);

		virtual ~Gif();

		Gif& operator=(const Gif& gif);

		// ����ͼ��
		void load(const WCHAR* gifFileName);

		// ���豸
		void bind(HDC hdc);

		// ��ռ���ͼ��
		void clear();

		// λ��
		void setPos(int x, int y);
		void setSize(int width, int height);

		int getX() const { return x; }
		int getY() const { return y; }

		// ���ú��ͼ���С��Ϊ 0 ��ʾʹ��ԭͼ��С��
		int getWidth() const { return width; }
		int getHeight() const { return height; }

		// ԭͼ��С
		int getOrginWidth() const;
		int getOrginHeight() const;

		// ֡��Ϣ
		int getFrameCount() const { return frameCount; }
		int getCurFrame() const { return curFrame; }

		// ��ʱʱ���ȡ������
		int getDelayTime(int frame) const;
		void setDelayTime(int frame, long time_ms);
		void setAllDelayTime(long time_ms);

		// ����ʱ�䣬���㵱ǰ֡
		void updateTime();

		// ���Ƶ�ǰ֡��ָ��֡
		void draw();
		void draw(int x, int y);
		void drawFrame(int frame);
		void drawFrame(int frame, int x, int y);

		// ��ȡͼ��
		void getimage(IMAGE* pimg, int frame);

		// ����״̬����
		void play();
		void pause();
		void toggle();

		bool isPlaying()const { return playing; }

		void setVisible(bool enable) { visible = enable; }
		bool isVisible() const { return visible; }

		bool isAnimation() const { return frameCount > 1; }

		// ���ò���״̬
		void resetPlayState();

		void info() const;

	private:
		void init();	// ��ʼ��
		void read();	// ��ȡͼ����Ϣ
		void copy(const Gif& gif);
	};

};
