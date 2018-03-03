#include "CheckInDialog.h"
#include "LoginDialog.h"
#include "MainDialog.h"
CheckInDialog* CheckInDialog::ptr = NULL;
CheckInDialog::CheckInDialog(WSADATA& wsdA, SOCKET& sClientA, sockaddr_in& srvA, HINSTANCE& hInst):wsd(wsdA),sClient(sClientA),srv(srvA), hInst(hInst)
{
	ptr = this;
}
CheckInDialog::~CheckInDialog()
{

}
bool CheckInDialog::addNewUser()
{
	regex reg("[\\w]*");
	//�������� ����� � ������ �� ��������� �����
	char login[50];
	char password[50];
	GetWindowText(GetDlgItem(CheckInDialog::ptr->hWindow, IDC_REG_LOGIN), login, 50);
	GetWindowText(GetDlgItem(CheckInDialog::ptr->hWindow, IDC_REG_PASSWORD1), password, 50);
	//������� ������ � ������ �� �� ���������� �������
	if (regex_match(login, reg) && regex_match(login, reg)&&strlen(login)>=6&&strlen(password)>=6) 
	{
		//�������� ������ �� ������ (�� �����������)
		MainDialog::queryToServer("102");

		char response[1] = { '0' };
		//��������  ����� � ������
		LoginDialog::sendBuffer(CheckInDialog::ptr->sClient, login, 50);
		LoginDialog::sendBuffer(CheckInDialog::ptr->sClient, password, 50);
		//������� �����
		LoginDialog::receiveBuffer(CheckInDialog::ptr->sClient, response, 1);

		if (response[0] == '0')//������� ��������� ����������� (0 ��� ������)
		{
			return true;
		}
		else if (response[0])
		{
			MessageBox(CheckInDialog::ptr->hWindow, "����������� �� �������.", "������", 0);
			return false;
		}
	}
	else if(!regex_match(login, reg) || !regex_match(login, reg))
	{
		MessageBox(CheckInDialog::ptr->hWindow, "������������ ������� � ������ ��� ������.", "������", 0);
		return false;
	}
	else if (strlen(login) < 6 || strlen(password) < 6) 
	{
		MessageBox(CheckInDialog::ptr->hWindow, "����� ������ � ������ ������ ���� 6 ��� ����� ��������.", "������", 0);
		return false;
	}
	return false;
}
void CheckInDialog::Cls_OnClose(HWND hwnd)
{
	//��������� � ������� ����������
	MainDialog::queryToServer("99");
	//���������� ��� �������
	shutdown(sClient, SD_BOTH); // SD_BOTH ��������� ��� �����, ��� � �������� ������
	closesocket(sClient); // The closesocket function closes an existing socket.
	WSACleanup(); // The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
	EndDialog(hwnd, 0);
}
BOOL CheckInDialog::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hWindow = hwnd;
	//������
	HICON hIcon1 = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hWindow, WM_SETICON, 1, (LPARAM)hIcon1);
	return TRUE;
}
void CheckInDialog::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	if (id == IDC_BACK) {
		EndDialog(hWindow, 126);
	}
	if (id == IDC_CHECKIN_C) {
		//�������� ���������� �������
		char password[50];
		char password2[50];
		GetWindowText(GetDlgItem(ptr->hWindow, IDC_REG_PASSWORD1), password, 50);
		GetWindowText(GetDlgItem(ptr->hWindow, IDC_REG_PASSWORD2), password2, 50);

		if (strcmp(password, password2)) {
			MessageBox(hWindow, "������ �� ���������", "������", 0);
		}
		else {
			if (addNewUser())
			{
				MessageBox(hWindow, "�� ������� ����������������.", "����������", 0);
				EndDialog(hwnd, 126);
			}
		}
	}
}
BOOL CALLBACK CheckInDialog::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}