#pragma once

#include "../map_operation.h"

namespace oldmap
{
	// ����Ŀ
	RsMap OpenProject(const WCHAR* strFileName);

	// ������Ŀ
	bool SaveProject(RsMap map, const WCHAR* strFileName);
}

