#include "FolderDialog.h"
FolderDialog* FolderDialog::ptr=NULL;
BOOL CALLBACK FolderDialog::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}
BOOL FolderDialog::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hWindow = hwnd;

	hNameFolder = GetDlgItem(hwnd, IDC_F_EDIT);

	HICON hIcon1 = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hwnd, WM_SETICON, 1, (LPARAM)hIcon1);

	return TRUE;
}
void FolderDialog::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_APPLY) 
	{
		char name[MAX_PATH];
		GetWindowText(hNameFolder, name, MAX_PATH);
		strcpy(MainDialog::newDirectoryPath, name);
		EndDialog(hWindow, 0);
	}
}
FolderDialog::FolderDialog() 
{
	ptr = this;
}
FolderDialog::~FolderDialog() 
{
}
void FolderDialog::Cls_OnClose(HWND hwnd)
{
	EndDialog(hWindow, 0);
}