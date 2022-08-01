////////////////////////////////////////////////////
//
//	HiEasyX.h
//	HiEasyX ��ͷ�ļ�
//	�ÿ���� EasyX ʵ�����ڴ��ڡ���ͼ��UI ����෽���ȫ������
//
//	�桡������Ver 0.1(alpha)
//	�������ߣ�huidong <mailhuid@163.com>
//	���뻷����VisualStudio 2022 | EasyX_20220610 | Windows 10 
//	��Ŀ��ַ��https://github.com/zouhuidong/HiEasyX
//	�������ڣ�2022.07.20
//	����޸ģ�2022.08.01
//

#pragma once

#define __HIEASYX_H__

#include "HiEasyX/HiMacro.h"
#include "HiEasyX/HiFunc.h"
#include "HiEasyX/HiFPS.h"
#include "HiEasyX/HiDrawingProperty.h"
#include "HiEasyX/HiMouseDrag.h"
#include "HiEasyX/HiMusicMCI.h"
#include "HiEasyX/HiGif.h"

// ���� NATIVE_EASYX ��ʹ��ԭ�� EasyX ���ڣ����� HiWindow
#ifndef NATIVE_EASYX
#include "HiEasyX/HiWindow.h"
#endif

#include "HiEasyX/HiCanvas.h"
#include "HiEasyX/HiGUI/HiGUI.h"


// ���ݾɰ� EasyWin32 �����ռ䣬�뾡��ʹ�� HiEasyX �����ռ�
namespace EasyWin32 = HiEasyX;

