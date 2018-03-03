#include "includes.h"
#include "LoginDialog.h"
#include "CheckInDialog.h"
#include "MainDialog.h"
WSADATA wsd;
SOCKET sClient;
sockaddr_in server;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpszCmdLine, int nCmdMode)
{
	LoginDialog lDlg(wsd,sClient,server,hInst);
	CheckInDialog cDlg(wsd, sClient, server, hInst);
	MainDialog mDlg(wsd, sClient, server, hInst);

	INT_PTR returnsCode = DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, LoginDialog::DlgProc);
	while (returnsCode != 0) {
		if (returnsCode == 125) {
			returnsCode = DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), 0, CheckInDialog::DlgProc);
		}
		else if (returnsCode == 126) {
			returnsCode = DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, LoginDialog::DlgProc);
		}
		else if (returnsCode == 127) {
			returnsCode = DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), 0, MainDialog::DlgProc);
		}
	}
	return returnsCode;
}