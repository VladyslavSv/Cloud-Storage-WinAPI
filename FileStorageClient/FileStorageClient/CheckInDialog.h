#pragma once
#include "includes.h"

class CheckInDialog
{
public:
	CheckInDialog(WSADATA& wsd, SOCKET& sClient, sockaddr_in& server, HINSTANCE& hInst);
	~CheckInDialog();
	HINSTANCE hInst;
	WSADATA& wsd;
	SOCKET& sClient;
	sockaddr_in& srv;
	HWND hWindow;
	static CheckInDialog* ptr;

	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);
	bool addNewUser();

};
struct message_c {
	int type;
	CheckInDialog* currentUser;
};