#include "oldmap.h"
#include <stdio.h>
#include <conio.h>
#include <io.h>
#include <string>
#include <locale.h>

int main()
{
	setlocale(LC_ALL, "chs");

	while (true)
	{
		system("cls");
		printf("Minecraft Redstone Simulator Ver 2.1 地图转换器\n");
		printf("地图路径：\n");

		// 清空键盘缓冲区
		rewind(stdin);

		TCHAR buf[256] = { 0 };
		wscanf_s(L"%[^\n]", buf, 256);

		int offset = 0;
		if (buf[0] == L'\"')
			offset = 1;
		std::wstring wstr = buf + offset;
		if (offset)
			*(wstr.end() - 1) = L'\0';

		FILE* fp = nullptr;
		errno_t err = _wfopen_s(&fp, wstr.c_str(), L"r+");
		if (err)
		{
			MessageBox(nullptr, L"打开文件失败", L"ERROR", MB_OK);
			return -1;
		}

		printf("地图加载中……请稍等\n");

		int len = _filelength(_fileno(fp));
		int* pBuf = new int[len];
		ZeroMemory(pBuf, len * sizeof(int));
		fread(pBuf, len * sizeof(int), 1, fp);
		fclose(fp);

		RsMap map;
		bool old = isOldMap(pBuf, len);

		if (old)
			map = oldmap::OpenProject(wstr.c_str());
		else
			map = OpenProject(wstr.c_str());

		printf("\n地图类型：%s\n", old ? "旧版地图" : "新版地图");
		printf("地图尺寸：%d x %d\n", map.w, map.h);

		printf("\n按任意键将地图转换为%s\n", old ? "新版" : "旧版");
		printf("按 ESC 取消转换\n");
		if (_getch() == 27)
			continue;
		
		// 备份
		std::wstring wstrBackup = wstr;
		wstrBackup.insert(wstrBackup.rfind(L'.'), L"_backup");

		if (!CopyFile(wstr.c_str(), wstrBackup.c_str(), false))
		{
			printf("备份原地图文件失败，请手动备份后按任意键继续\n");
			while (_kbhit())
				_getch();
			Sleep(1000);
			_getch();
		}

		if (old)
			SaveProject(map, wstr.c_str());
		else
			oldmap::SaveProject(map, wstr.c_str());

		printf("\n完成。");
		_getch();
	}

	return 0;
}
