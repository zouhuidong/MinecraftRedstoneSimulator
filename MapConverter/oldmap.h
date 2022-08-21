#pragma once

#include "../map_operation.h"

namespace oldmap
{
	// 打开项目
	RsMap OpenProject(const WCHAR* strFileName);

	// 保存项目
	bool SaveProject(RsMap map, const WCHAR* strFileName);
}

