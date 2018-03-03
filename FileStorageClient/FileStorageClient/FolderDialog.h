#pragma once
#include "MainDialog.h"
#include "includes.h"
class FolderDialog 
{
public:
	FolderDialog();
	~FolderDialog();
	HWND hNameFolder,hWindow;
	static FolderDialog* ptr;
	static BOOL CALLBACK DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);
};