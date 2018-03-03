#pragma once
#include "includes.h"

class LoginDialog
{
public:
	LoginDialog(WSADATA& wsd,SOCKET& sClient,sockaddr_in& server, HINSTANCE& hInst);
	~LoginDialog();
public:
	HINSTANCE hInst;
	static bool isConnected;
	static LoginDialog* ptr;
	WSADATA& wsd; 
	SOCKET& sClient;
	sockaddr_in& srv;
	HWND hWindow;

	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);
	bool SendUserNameAndPassword();
	//вспомогательные функции
	static int sendBuffer(SOCKET& currentSocket, char* buffer, int length);
	static int receiveBuffer(SOCKET& currentSocket,char* buffer, int length);
	static int sendFile(SOCKET& currentSocket, char* fileName);
	static int receiveFile(SOCKET& currentSocket, const  char* fileName);
	static long getFileSize(char *fileName);
};
struct message_l {
	int type;
	LoginDialog* currentUser;
};