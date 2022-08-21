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
		printf("Minecraft Redstone Simulator Ver 2.1 ��ͼת����\n");
		printf("��ͼ·����\n");

		// ��ռ��̻�����
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
			MessageBox(nullptr, L"���ļ�ʧ��", L"ERROR", MB_OK);
			return -1;
		}

		printf("��ͼ�����С������Ե�\n");

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

		printf("\n��ͼ���ͣ�%s\n", old ? "�ɰ��ͼ" : "�°��ͼ");
		printf("��ͼ�ߴ磺%d x %d\n", map.w, map.h);

		printf("\n�����������ͼת��Ϊ%s\n", old ? "�°�" : "�ɰ�");
		printf("�� ESC ȡ��ת��\n");
		if (_getch() == 27)
			continue;
		
		// ����
		std::wstring wstrBackup = wstr;
		wstrBackup.insert(wstrBackup.rfind(L'.'), L"_backup");

		if (!CopyFile(wstr.c_str(), wstrBackup.c_str(), false))
		{
			printf("����ԭ��ͼ�ļ�ʧ�ܣ����ֶ����ݺ����������\n");
			while (_kbhit())
				_getch();
			Sleep(1000);
			_getch();
		}

		if (old)
			SaveProject(map, wstr.c_str());
		else
			oldmap::SaveProject(map, wstr.c_str());

		printf("\n��ɡ�");
		_getch();
	}

	return 0;
}
