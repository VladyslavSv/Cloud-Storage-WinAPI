#include "includes.h"
void messageAboutError(DWORD dwError)
{
	LPVOID lpMsgBuf = NULL;
	char szBuf[300];
	//������� FormatMessage ����������� ������ ���������
	BOOL fOK = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM /* ���� �������� �������, ��� ����� ������, ��������������� ���� ������, ������������ � ������� */
		| FORMAT_MESSAGE_ALLOCATE_BUFFER, //����� �������� ��������������� ���� ������ ��� �������� ������
		NULL, //��������� �� ������, ���������� ����� ���������
		dwError, //��� ������
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),//����, �� ������� ��������� �������� ������ (���� ������������ �� ���������)
		(LPTSTR)&lpMsgBuf, //��������� �� �����, � ������� ��������� ����� ���������
		0, //����������� ������ ������ ��� ��������� ������ - ������ �������� �������
		NULL //������ ���������� ��������������
	);
	if (lpMsgBuf != NULL)
	{
		wsprintf(szBuf, "������ %d: %s", dwError, lpMsgBuf);
		MessageBox(0, szBuf, "��������� �� ������", MB_OK | MB_ICONSTOP);
		LocalFree(lpMsgBuf); //����������� ������, ���������� ��������
	}
}