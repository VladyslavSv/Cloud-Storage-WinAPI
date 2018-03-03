#pragma once
#include "includes.h"
struct Item
{
	char fileName[MAX_PATH];
	char size[MAX_PATH];
};
class MainDialog
{
public:
	MainDialog(WSADATA& wsd, SOCKET& sClient, sockaddr_in& server, HINSTANCE& hInst);
	~MainDialog();
public:

	//-----------
	int                 iHeight;
	static HIMAGELIST   hDragImageList;
	HIMAGELIST          hOneImageList, hTempImageList;
	LPNMHDR             pnmhdr;
	static BOOL         bDragging;
	LVHITTESTINFO       lvhti;
	BOOL                bFirst;
	IMAGEINFO           imf;
	POINT p;
	//-----------

	HINSTANCE hInst;
	WSADATA& wsd;
	SOCKET& sClient;
	sockaddr_in& srv;
	static MainDialog* ptr;
	CRITICAL_SECTION* criticalSection;
	mutex moveMutex;
	HWND hWindow, hEditControlPath, hButtonBrowse, hProgressBar, hListView;
	HMENU hMenu;
	vector <Item*> ListEls;
	static char* newDirectoryPath;
	static BOOL CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);

	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);
	void OnColumnDoubleClick(LPNMLISTVIEW item);

	static int getAllFilesInFolder();
	int getFileName(char* fileName, char* globalPath);
	bool browse(char* pth);
	static bool queryToServer(char* queryNum);
	HWND createListView(int x, int y, int width, int height, HWND * hwndParent);
	BOOL InitListViewColumns(HWND hWndListView);
	BOOL InsertListViewItems(HWND hWndListView, int cItems);
	VOID UpdateUserList();
};
struct InfoForFile {
	int type;
	char pathForDownload[MAX_PATH];
	char fileName[MAX_PATH];
	char fullFileNameOnServer[MAX_PATH];
	HWND hWindow;
};
struct MoveFileInfo 
{
	char** movingFiles;
	int countOfMovingFiles;
	char* destination;
};
