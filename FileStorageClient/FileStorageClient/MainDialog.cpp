#include "MainDialog.h"
#include "LoginDialog.h"
#include "FolderDialog.h"
#define IDC_LISTVIEW 0x8816
MainDialog* MainDialog::ptr = NULL;
char currentDirectory[MAX_PATH];
char* MainDialog::newDirectoryPath=new char[MAX_PATH];
BOOL MainDialog::bDragging;
HIMAGELIST MainDialog::hDragImageList;
//���� ����� � ������� �������� �� ���������� ��� �������
void HandleWM_NOTIFY(LPARAM lParam)
{
	NMLVDISPINFO* plvdi;

	switch (((LPNMHDR)lParam)->code)
	{
		case LVN_GETDISPINFO:
		{
			plvdi = (NMLVDISPINFO*)lParam;

			switch (plvdi->item.iSubItem)
			{
			case 0:
				if (MainDialog::ptr->ListEls.size() != 0)
				{
					StringCchCopy(plvdi->item.pszText, plvdi->item.cchTextMax, MainDialog::ptr->ListEls[plvdi->item.iItem]->fileName);
				}
				break;
			case 1:
				if (MainDialog::ptr->ListEls.size() != 0)
				{
					StringCchCopy(plvdi->item.pszText, plvdi->item.cchTextMax, MainDialog::ptr->ListEls[plvdi->item.iItem]->size);
				}
				break;
			}
			break;
		}
	}
}
//�������� �������
HWND MainDialog::createListView(int x, int y, int width, int height, HWND * hwndParent)
{
	INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	//RECT rcClient;                       // The parent window's client area.

	//GetClientRect(hwndParent, &rcClient);

	// Create the list-view window in report view with label editing enabled.
	HWND hWndListView = CreateWindow(WC_LISTVIEW,
		TEXT(""),
		WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE | WS_BORDER | LVS_OWNERDATA,
		x, y,
		width,
		height,
		*hwndParent,
		(HMENU)IDC_LISTVIEW,
		NULL,
		NULL);

	return (hWndListView);
}
//������������� �������
BOOL MainDialog::InitListViewColumns(HWND hWndListView)
{
	TCHAR szText[256];     // Temporary buffer.
	LVCOLUMN lvc;
	int iCol;

	// Initialize the LVCOLUMN structure.
	// The mask specifies that the format, width, text,
	// and subitem members of the structure are valid.
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	// Add the columns.
	for (iCol = 0; iCol < /*C_COLUMNS*/2; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = szText;
		switch (iCol) {
		case 0:lvc.cx = 250; break;
		case 1:lvc.cx = 100; break;
		}


		if (iCol < 6)
			lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.
		else
			lvc.fmt = LVCFMT_RIGHT; // Right-aligned column.

									// Load the names of the column headings from the string resources.
		LoadString(NULL,
			IDS_NAME + iCol,
			szText,
			sizeof(szText) / sizeof(szText[0]));

		// Insert the columns into the list view.
		if (ListView_InsertColumn(hWndListView, iCol, &lvc) == -1)
			return FALSE;
	}
	UpdateUserList();
	return TRUE;
}
//��������� ������ ������ � �������
BOOL MainDialog::InsertListViewItems(HWND hWndListView, int cItems)
{
	LVITEM lvI;

	// Initialize LVITEM members that are common to all items.
	lvI.pszText = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
	lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem = 0;
	lvI.state = 0;

	// Initialize LVITEM members that are different for each item.
	for (int index = 0; index < cItems; index++)
	{
		lvI.iItem = index;
		lvI.iImage = index;

		// Insert items into the list.
		if (ListView_InsertItem(hWndListView, &lvI) == -1)
			return FALSE;
	}
	return TRUE;
}
//���������� �������
VOID MainDialog::UpdateUserList() {
	ListView_DeleteAllItems(hListView);

	InsertListViewItems(hListView, ListEls.size());
}
bool findSymbol(const char* str, char symbol) 
{
	for (int i = 0; str[i] != '\0'; i++) 
	{
		if (str[i] == symbol) 
		{
			return true;
		}
	}
	return false;
}
char* getLastWordOfPath(const std::string& subject)
{
	std::vector<std::string> container;

	container.clear();
	size_t len = subject.length() + 1;
	char* s = new char[len];
	memset(s, 0, len * sizeof(char));
	memcpy(s, subject.c_str(), (len - 1) * sizeof(char));
	for (char *p = strtok(s, "\\"); p != NULL; p = strtok(NULL, "\\"))
	{
		container.push_back(p);
	}
	delete[] s;
	char* retVal = new char[50];
	strcpy(retVal, (container[container.size() - 1].c_str()));
	return retVal;
}
bool isDirectoryExists(const char *filename)
{
	DWORD dwFileAttributes = GetFileAttributes(filename);
	if (dwFileAttributes == 0xFFFFFFFF)
		return false;
	return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}
MainDialog::MainDialog(WSADATA& wsdA, SOCKET& sClientA, sockaddr_in& serverA,HINSTANCE& hInst):wsd(wsdA), sClient(sClientA), srv(serverA),hInst(hInst)
{
	criticalSection = new CRITICAL_SECTION();	
	InitializeCriticalSection(criticalSection);
	ptr = this;
}
MainDialog::~MainDialog()
{

}
void MainDialog::Cls_OnClose(HWND hwnd)
{
	//��������� � ������� ����������
	queryToServer("99");
	//���������� ��� �������
	shutdown(sClient, SD_BOTH); // SD_BOTH ��������� ��� �����, ��� � �������� ������
	closesocket(sClient); // The closesocket function closes an existing socket.
	WSACleanup(); // The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
	EndDialog(hwnd, 0);
}
int MainDialog::getAllFilesInFolder()
{
	int size = 0;
	int amount = 0;
	char helpStr[50];
	char* buffer=nullptr;

	//�������� ���� � �����
	LoginDialog::sendBuffer(MainDialog::ptr->sClient, currentDirectory, MAX_PATH);
	//������� ������ � ���������� ������
	MainDialog::ptr->ListEls.clear();
	//������� ���������� ������ � �����
	LoginDialog::receiveBuffer(MainDialog::ptr->sClient, helpStr,50);
	//����������� ������ � �����
	amount=strtol(helpStr, nullptr, 0);

	for (int i = 0; i < amount; i++) {
		Item* item = new Item;
		//������� ����� ������
		LoginDialog::receiveBuffer(MainDialog::ptr->sClient, helpStr, 50);
		size = strtol(helpStr, nullptr, 0);
		buffer = new char[size+1];
		//������� ������ � �������
		LoginDialog::receiveBuffer(MainDialog::ptr->sClient, buffer, size);
		buffer[size] = '\0';
		//������� ��� � ������
		strcpy_s(item->fileName, buffer);
		//������� ������ �����
		size = 10;
		buffer = new char[size];
		LoginDialog::receiveBuffer(MainDialog::ptr->sClient, buffer, size);
		//������� ������ � ������
		strcpy_s(item->size, buffer);
		//������� ����� ������ � ������
		MainDialog::ptr->ListEls.push_back(item);
	}

	if (strcmp(currentDirectory, "")) 
	{
		Item* lstItem = new Item();

		strcpy(lstItem->fileName, "...");
		strcpy(lstItem->size, " ");

		MainDialog::ptr->ListEls.insert(MainDialog::ptr->ListEls.begin(), lstItem);
	}
	MainDialog::ptr->UpdateUserList();
	return 0;
}
bool MainDialog::browse(char* pth)
{
	BROWSEINFO* binfo = new BROWSEINFO{0};
	(*binfo).hwndOwner = hWindow;
	(*binfo).ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_BROWSEINCLUDEFILES;
	(*binfo).lpszTitle = "";
	LPITEMIDLIST ptr = SHBrowseForFolder(binfo);//<<<<<----------��� ��������� exception ��� ������ ����� � �����
	if (ptr)									//�������� ������ ����� � ������� ��������� ���� 2 ����												
	{											//��� ���������� ��� �� ��
		SHGetPathFromIDList(ptr, pth);
		delete binfo;
		return true;
	}
	else return false;
}
int MainDialog::getFileName(char* fileName,char* globalPath) {
		char* ptr = new char(100);
		strcpy_s(ptr, 100, globalPath);
		ptr = strtok(ptr, "\\");
		while (ptr)
		{
			strcpy_s(fileName, 100, ptr);

			ptr = strtok(nullptr, "\\");
		}
		delete[] ptr;
	return 0;
}
bool MainDialog::queryToServer(char* queryNum) 
{
	char query[4];
	wsprintf(query, queryNum);
	LoginDialog::sendBuffer(LoginDialog::ptr->sClient, query, 4);

	return true;
}
DWORD WINAPI ThreadForFile(LPVOID lpParam)
{
	InfoForFile* info = (InfoForFile*)lpParam;

	EnterCriticalSection(MainDialog::ptr->criticalSection);

	if (info->type == 1) 
	{
		//�������� ������ �� ���������� �����
		MainDialog::queryToServer("104");
	}
	else if (info->type == 2) 
	{
		//�������� ������ �� �������� �����
		MainDialog::queryToServer("105");
	}

	EnableMenuItem(MainDialog::ptr->hMenu, IDC_UPLOAD, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(MainDialog::ptr->hMenu, ID_DOWNLOAD, MF_BYCOMMAND | MF_GRAYED);
	EnableMenuItem(MainDialog::ptr->hMenu, ID_REMOVE, MF_BYCOMMAND | MF_GRAYED);

	WSADATA file_wsd;
	SOCKET file_sClient;
	sockaddr_in file_server;

	file_sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (file_sClient == SOCKET_ERROR)
	{
		messageAboutError(WSAGetLastError());
		WSACleanup();
		return 0;
	}

	file_server.sin_family = AF_INET;
	file_server.sin_port = htons(11526);
	char ip_address[20] = "195.138.81.175";
	file_server.sin_addr.S_un.S_addr = inet_addr(ip_address);

	if (connect(file_sClient, (sockaddr*)&(file_server), sizeof(file_server)) == SOCKET_ERROR)
	{
		messageAboutError(WSAGetLastError());
		closesocket(file_sClient);
		WSACleanup();
		return 0;
		MessageBox(MainDialog::ptr->hWindow, "�� ������� ���������� ���������� ��� ��������� ������", "������", 0);
	}

	char host[50];
	gethostname(host, 50);

	//�������� ������ ����� ��� ����� �� ������
	LoginDialog::sendBuffer(file_sClient, info->fullFileNameOnServer, MAX_PATH);
	if (info->type == 1) 
	{
		SetWindowText(GetDlgItem(MainDialog::ptr->hWindow, IDC_FILE_NAME), info->fileName);
		//��������� ���� ��� ����������
		string str(info->pathForDownload);
		str += "\\";

		//�������� ��� ����� � �������
		char finalName[MAX_PATH]="";
		LoginDialog::receiveBuffer(file_sClient, finalName, MAX_PATH);
		str += finalName;
		if (strcmp(finalName, "")) 
		{
			MessageBox(MainDialog::ptr->hWindow, "��� ����� ��� ���������� �� ��������", finalName,0);
		}
		//�������� ��� ����
		LoginDialog::receiveFile(file_sClient, str.c_str());

		string strShow;
		strShow += "���� '";
		strShow += finalName;
		strShow += "' �������.";
		MessageBox(MainDialog::ptr->hWindow, strShow.c_str(), "����������", 0);
		SetWindowText(GetDlgItem(MainDialog::ptr->hWindow, IDC_FILE_NAME), "");
	}
	else if (info->type == 2)
	{
		SetWindowText(GetDlgItem(MainDialog::ptr->hWindow, IDC_FILE_NAME), info->fileName);
		//�������� ����
		LoginDialog::sendFile(file_sClient, info->pathForDownload);
		//������� ������
		MainDialog::queryToServer("103");
		//������� ��� ����� �� ������� ����������
		MainDialog::getAllFilesInFolder();

		string strShow;
		strShow += "���� '";
		strShow += info->fileName;
		strShow += "' ���������.";
		MessageBox(MainDialog::ptr->hWindow, strShow.c_str() , "����������", 0);
		SetWindowText(GetDlgItem(MainDialog::ptr->hWindow, IDC_FILE_NAME), "");
	}

	shutdown(file_sClient, SD_BOTH);
	closesocket(file_sClient);
	
	EnableMenuItem(MainDialog::ptr->hMenu, IDC_UPLOAD, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(MainDialog::ptr->hMenu, ID_DOWNLOAD, MF_BYCOMMAND | MF_ENABLED);
	EnableMenuItem(MainDialog::ptr->hMenu, ID_REMOVE, MF_BYCOMMAND | MF_ENABLED);

	LeaveCriticalSection(MainDialog::ptr->criticalSection);
	return 0;
}
DWORD WINAPI moveFileThread(LPVOID lpParam) 
{
	MainDialog::ptr->moveMutex.lock();
	//���������� ������ �� ����������� ������
	MainDialog::queryToServer("108");
	//�������� ���������� ��� ����������� ������
	MoveFileInfo* m_info = (MoveFileInfo*)lpParam;
	//������������� ����������
	WSADATA m_file_wsd;
	SOCKET m_file_sClient;
	sockaddr_in m_file_server;

	m_file_sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_file_sClient == SOCKET_ERROR)
	{
		messageAboutError(WSAGetLastError());
		WSACleanup();
		return 0;
	}

	m_file_server.sin_family = AF_INET;
	m_file_server.sin_port = htons(11527);
	char ip_address[20] = "195.138.81.175";
	m_file_server.sin_addr.S_un.S_addr = inet_addr(ip_address);

	if (connect(m_file_sClient, (sockaddr*)&(m_file_server), sizeof(m_file_server)) == SOCKET_ERROR)
	{
		messageAboutError(WSAGetLastError());
		closesocket(m_file_sClient);
		WSACleanup();
		MessageBox(MainDialog::ptr->hWindow, "�� ������� ���������� ���������� ��� ����������� ������", "������", 0);
		return 0;		
	}
	
	const int helpArraySize=10;
	char helpArray[helpArraySize];
	//�������� ���� ����������
	LoginDialog::sendBuffer(m_file_sClient, m_info->destination, MAX_PATH);
	//�������� ���������� ������ ������� ����� ������������
	sprintf(helpArray,"%d",m_info->countOfMovingFiles);
	LoginDialog::sendBuffer(m_file_sClient, helpArray, helpArraySize);
	//�������� ����� ������ ������� ����� ����������� �� �������
	for (int i = 0; i < m_info->countOfMovingFiles; i++) 
	{
		LoginDialog::sendBuffer(m_file_sClient, m_info->movingFiles[i], MAX_PATH);
	}
	//�������� ����� �� �������
	LoginDialog::receiveBuffer(m_file_sClient, helpArray, helpArraySize);
	if (!strcmp(helpArray, "0")) 
	{
		//������� ������(�������� ������ �� ��������� ������ � ������� �����)
		MainDialog::queryToServer("103");
		//������� ��� ����� �� ������� ����������
		MainDialog::getAllFilesInFolder();
	}
	else 
	{
		MessageBox(MainDialog::ptr->hWindow, "�� ������� ����������� �����", "������", 0);
	}

	shutdown(m_file_sClient, SD_BOTH);
	closesocket(m_file_sClient);
	MainDialog::ptr->moveMutex.unlock();
	return 0;
}
BOOL MainDialog::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	//��������� ������� ����������
	strcpy(currentDirectory, "");

	hWindow = hwnd;
	hButtonBrowse = GetDlgItem(hWindow, IDC_BROWSE);

	//������� ���������� �������� ����
	hProgressBar = GetDlgItem(hWindow, IDC_PROGRESS1);
	//������������� �������� �������� ����
	SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
	//��������� ������ ���� ��� �������� ����
	SendMessage(hProgressBar, PBM_SETSTEP, (WPARAM)1, 0);

	HICON hIcon1 = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hWindow, WM_SETICON, 1, (LPARAM)hIcon1);

	hEditControlPath = GetDlgItem(hwnd, IDC_DOWNLOAD_PATH);

	HMENU hMenuTemp=LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU1));
	MainDialog::hMenu = hMenuTemp;
	SetMenu(hwnd, hMenu);

	//�������� ������ �� ��������� ���� ������ �� �����
	queryToServer("103");

	//������� ��� ����� �� ������� ����������
	getAllFilesInFolder();

	//����������������� ������ � ������� ������
	hListView = createListView(11, 10, 350, 335, &hwnd);
	InitListViewColumns(hListView);

	return TRUE;
}
void MainDialog::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	if (id == IDC_EXIT) {
		if (MessageBox(hwnd, "�� �������� ��� ������ �����?", "�����", MB_YESNO)==IDYES) {
			//����� ������� �������� ������ �� ����

			//�������� ������ �� �����
			queryToServer("99");

			LoginDialog::isConnected = false;

			EndDialog(hWindow, 126);
		}
	}
	else if (id == ID_DOWNLOAD) {
		for (int itemIndex = -1; (itemIndex = SendMessage(hListView, LVM_GETNEXTITEM, itemIndex, LVNI_SELECTED)) != -1; )
		{
			if (itemIndex != LB_ERR)
			{

				char textBuffer[MAX_PATH]="";
				strcpy(textBuffer, ListEls[itemIndex]->fileName);

				string fullPathToFileOnServer(currentDirectory);
				fullPathToFileOnServer += "\\";
				fullPathToFileOnServer += textBuffer;

				if (strcmp(textBuffer, ""))
				{
					char pathToDownloadFolder[MAX_PATH];
					GetWindowText(hEditControlPath, pathToDownloadFolder, MAX_PATH);

					if (isDirectoryExists(pathToDownloadFolder)) {
						//�������� ��������� � �����������
						InfoForFile* info = new InfoForFile;
						info->type = 1;
						info->hWindow = hWindow;

						strcpy_s(info->pathForDownload, pathToDownloadFolder);
						strcpy_s(info->fileName, ListEls[itemIndex]->fileName);
						strcpy_s(info->fullFileNameOnServer, fullPathToFileOnServer.c_str());
						//������� ���� ��� ���������
						HANDLE hThread = CreateThread(0, 0, ThreadForFile, info, 0, 0);
						CloseHandle(hThread);
					}
					else {
						MessageBox(0, "������� ���� � �����", "������", 0);
						return;
					}
				}
			}
		}
	}
	else if (id == IDC_UPLOAD) {

		char userPth[MAX_PATH]="-";
		browse(userPth);

		bool isExists = false;
		isExists = isDirectoryExists(userPth);

		if (isExists)
			MessageBox(hWindow, "�������� ����, � �� �����", "������", 0);
		else if(isExists==false&&strcmp(userPth,"-"))
		{
			//�������� ��������� � �����������
			InfoForFile* info = new InfoForFile;
			info->type = 2;
			info->hWindow = hWindow;
			strcpy_s(info->pathForDownload, userPth);
			strcpy_s(info->fileName, getLastWordOfPath(userPth));
		
			strcpy_s(info->fullFileNameOnServer, "");
			strcat(info->fullFileNameOnServer, currentDirectory);
			if (strcmp(currentDirectory, "")) {
				strcat(info->fullFileNameOnServer, "\\");
			}
			strcat(info->fullFileNameOnServer, info->fileName);
			//������� ���� ��� ���������
			HANDLE hThread = CreateThread(0, 0, ThreadForFile, info, 0, 0);
			CloseHandle(hThread);
		}

	}
	else if (id == IDC_BROWSE) {

		char path[MAX_PATH]="-";
		browse(path);
		bool isExists = false;
		isExists = isDirectoryExists(path);
		if (isExists==true)
			SetWindowText(hEditControlPath, path);
		else if(isExists==false&&strcmp(path,"-"))
			MessageBox(hWindow,"�������� �����, � �� ����.","������",0);


	}
	else if (id == ID_REMOVE) 
	{
		char answer[1] = { '1' };
		int itemIndex = SendMessage(hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
		if (itemIndex == LB_ERR)
		{
			MessageBox(0, "�������� ����� ��� ��������.", "������", 0);
			return;
		}

		if (MessageBox(hwnd, "�� �������� ��� ������ ������� ����(�)?", "��������", MB_YESNO) == IDYES) 
		{
			
			do
			{
					char textBuffer[MAX_PATH];
					strcpy(textBuffer, ListEls[itemIndex]->fileName);

					if ( strcmp(textBuffer, "") && strcmp(textBuffer,"..."))
					{
						//�������� ������ �� �������� �����
						queryToServer("106");
						string pth(currentDirectory);
						
						if (strcmp(currentDirectory, "")) 
						{
							pth += "\\";
						}
						pth += textBuffer;
						strcpy(textBuffer, pth.c_str());

						LoginDialog::sendBuffer(MainDialog::ptr->sClient, textBuffer, MAX_PATH);

						LoginDialog::receiveBuffer(MainDialog::ptr->sClient, answer, 1);

						string input = "�� ���������� ������� ����";
						input += "'";
						input += textBuffer;
						input += "'.";
						if (answer[0] == '1')
						{
							MessageBox(0, input.c_str(), "������", 0);
						}
					}
				
			} while ((itemIndex = SendMessage(hListView, LVM_GETNEXTITEM, itemIndex, LVNI_SELECTED)) !=-1);

			//������ �� ��������� ���� ������
			queryToServer("103");

			//������� ��� ����� �� ������� ����������
			getAllFilesInFolder();

			if (answer[0] == '0') {
				MessageBox(0, "����� ������� �������!", "����������", 0);
			}
		}
	}
	else if (id == ID_CREATE_DIR) 
	{
		char answer[1] = {'1'};


		FolderDialog fd;
		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG4), hWindow, FolderDialog::DlgProc);
		//������ �� �������� �����
		queryToServer("107");
		//�������� ������ ���� �� ����� �� �������
		string newFolderPath(currentDirectory);
		if (strcmp(currentDirectory, "")) 
		{
			newFolderPath += "\\";
		}
		newFolderPath += newDirectoryPath;
		strcpy(newDirectoryPath, newFolderPath.c_str());
		//�������� ����
		LoginDialog::sendBuffer(sClient, newDirectoryPath, MAX_PATH);
		//������� �����
		LoginDialog::receiveBuffer(sClient, answer, 1);
		if (answer[0] == '0') 
		{
			MessageBox(hwnd, "����� �������", "����������", 0);
		}
		else 
		{
			MessageBox(hwnd, "�� ���������� ������� �����", "����������", 0);
		}
		//�������
		queryToServer("103");
		getAllFilesInFolder();
	}
}
void MainDialog::OnColumnDoubleClick(LPNMLISTVIEW item)
{
	if (item->iItem != -1) {
		if (!strcmp(ListEls[item->iItem]->size, "�����"))
		{
			if (strcmp(currentDirectory, "")) 
			{
				strcat(currentDirectory, "\\");
			}
			strcat(currentDirectory, ListEls[item->iItem]->fileName);

			queryToServer("103");
			getAllFilesInFolder();

			MainDialog::ptr->UpdateUserList();
		}
		else if (!strcmp(ListEls[item->iItem]->fileName, "..."))
		{
			string resStr(currentDirectory);
			string sub = resStr.substr(0, resStr.find_last_of("\\"));
			if (!strcmp(sub.c_str(), currentDirectory)) 
			{
				strcpy(currentDirectory, "");
			}
			else 
			{
				strcpy(currentDirectory, sub.c_str());
			}

			queryToServer("103");
			getAllFilesInFolder();

			MainDialog::ptr->UpdateUserList();
		}
	}
}
BOOL CALLBACK MainDialog::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
		case WM_NOTIFY: 
			{
				if ((((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW) &&
					(((LPNMHDR)lParam)->code == NM_DBLCLK))
				{
					MainDialog::ptr->OnColumnDoubleClick((LPNMLISTVIEW)lParam);
				}
				else if ((((LPNMHDR)lParam)->idFrom == IDC_LISTVIEW) &&
						(((LPNMHDR)lParam)->code == LVN_BEGINDRAG))
				{

					MainDialog::ptr->p.x = 8;
					MainDialog::ptr->p.y = 8;

					// Ok, now we create a drag-image for all selected items
					MainDialog::ptr->bFirst = TRUE;
					LRESULT iPos = ListView_GetNextItem(ptr->hListView, -1, LVNI_SELECTED);
					while (iPos != -1) {
						if (ptr->bFirst) {
							// For the first selected item,
							// we simply create a single-line drag image
							hDragImageList = ListView_CreateDragImage(ptr->hListView, iPos, &ptr->p);
							ImageList_GetImageInfo(hDragImageList, 0, &ptr->imf);
							ptr->iHeight = ptr->imf.rcImage.bottom;
							ptr->bFirst = FALSE;
						}
						else {
							// For the rest selected items,
							// we create a single-line drag image, then
							// append it to the bottom of the complete drag image
							ptr->hOneImageList = ListView_CreateDragImage(MainDialog::ptr->hListView, iPos, &ptr->p);
							ptr->hTempImageList = ImageList_Merge(hDragImageList,
								0, ptr->hOneImageList, 0, 0, ptr->iHeight);
							ImageList_Destroy(ptr->hDragImageList);
							ImageList_Destroy(ptr->hOneImageList);
							hDragImageList = ptr->hTempImageList;
							ImageList_GetImageInfo(hDragImageList, 0, &ptr->imf);
							ptr->iHeight = ptr->imf.rcImage.bottom;
						}
						iPos = ListView_GetNextItem(MainDialog::ptr->hListView, iPos, LVNI_SELECTED);
					}

					// Now we can initialize then start the drag action
					ImageList_BeginDrag(hDragImageList, 0, 0, 0);

					POINT pt = ((NM_LISTVIEW*)((LPNMHDR)lParam))->ptAction;
					ClientToScreen(MainDialog::ptr->hListView, &pt);

					ImageList_DragEnter(GetDesktopWindow(), pt.x, pt.y);

					bDragging = TRUE;

					// Don't forget to capture the mouse
					SetCapture(MainDialog::ptr->hWindow);
				}
				else 
				{
					HandleWM_NOTIFY(lParam);
					break;
				}

			}
		case WM_MOUSEMOVE:
		{
			if (!bDragging)
				break;

			ptr->p.x = LOWORD(lParam);
			ptr->p.y = HIWORD(lParam);

			ClientToScreen(ptr->hWindow, &ptr->p);
			ImageList_DragMove(ptr->p.x, ptr->p.y);
			break;
		}
		case WM_LBUTTONUP:
		{
			//������ ����������
			LVITEM LvItem = { 0 };
			int iPos;
			int index = 0;
			string fDestName;
			string fSendedItem;
			vector<char*> sendedFiles;
			char* helpArrayLeftButtonUp=new char[MAX_PATH];
			MoveFileInfo* move_info=new MoveFileInfo();
			// End the drag-and-drop process
			bDragging = FALSE;
			ImageList_DragLeave(ptr->hListView);
			ImageList_EndDrag();
			ImageList_Destroy(hDragImageList);

			ReleaseCapture();

			// Determine the dropped item
			ptr->lvhti.pt.x = LOWORD(lParam);
			ptr->lvhti.pt.y = HIWORD(lParam);
			ClientToScreen(ptr->hWindow, &ptr->lvhti.pt);
			ScreenToClient(ptr->hListView, &ptr->lvhti.pt);
			ListView_HitTest(ptr->hListView, &ptr->lvhti);

			// Out of the ListView?
			if (ptr->lvhti.iItem == -1)
				break;
			// Not in an item?
			if ((ptr->lvhti.flags & LVHT_ONITEMLABEL == 0) &&
				(ptr->lvhti.flags & LVHT_ONITEMSTATEICON == 0))
				break;

			// Dropped item is selected?
			LVITEM lvi;
			lvi.iItem = ptr->lvhti.iItem;
			lvi.iSubItem = 0;
			lvi.mask = LVIF_STATE;
			lvi.stateMask = LVIS_SELECTED;
			ListView_GetItem(ptr->hListView, &lvi);

			if (lvi.state & LVIS_SELECTED)
				break;
			//Get dropped item text
			const int SIZZE = 256;
			char buf[SIZZE];

			//�������� listview item 
			memset(&lvi, 0, sizeof(lvi));
			lvi.mask = LVIF_TEXT;
			lvi.iSubItem = 0;
			lvi.pszText = buf;
			lvi.cchTextMax = SIZZE;
			lvi.iItem = ptr->lvhti.iItem;

			//��������� ����� � �������� �� ������� ��������� mouseup
			SendMessage(ptr->hListView, LVM_GETITEMTEXT, ptr->lvhti.iItem, (LPARAM)&lvi);
			//�������� ������ ���� 
			fDestName = currentDirectory;
			if (strcmp(currentDirectory, "")) {
				fDestName += "\\";
			}
			fDestName += lvi.pszText;

			if (!strcmp(getLastWordOfPath(fDestName.c_str()), "..."))
			{
				fDestName.erase(fDestName.size() - 4, 5);
				index = fDestName.find_last_of('\\');
				if (index == -1) 
				{
					fDestName = "";
				}
				else 
				{
					fDestName.erase(index, fDestName.size() - index);
				}
			}
			if (findSymbol(fDestName.c_str(), '.'))
			{
				break;
			}
			//�������� ������ ���������� ������� � listview
			iPos = ListView_GetNextItem(ptr->hListView, -1, LVNI_SELECTED);

			//�������� ��� ���������� �������� � listview		
			while (iPos != -1) {
				//�������� ������� listview
				memset(&LvItem, 0, sizeof(LvItem));

				//��������� ������ ���������� ��� ������� listview item
				LvItem.mask = LVIF_TEXT;
				LvItem.iSubItem = 0;
				LvItem.pszText = buf;
				LvItem.cchTextMax = SIZZE;
				LvItem.iItem = iPos;

				//����������� ����� �� ����������� ��������
				SendMessage(ptr->hListView, LVM_GETITEMTEXT, iPos, (LPARAM)&LvItem);

				if (!strcmp(LvItem.pszText, "...")) 
				{
					iPos = ListView_GetNextItem(ptr->hListView, iPos, LVNI_SELECTED);
					continue;
				}
				//�������� ������ ���� � ����� � ����� ������
				fSendedItem = currentDirectory;
				if (strcmp(currentDirectory, "")) {
					fSendedItem += "\\";
				}
				fSendedItem += LvItem.pszText;
				//����������� ������
				helpArrayLeftButtonUp = new char[MAX_PATH];
				//�������� �� ������ � ��������������� ������
				strcpy(helpArrayLeftButtonUp, fSendedItem.c_str());

				//��������� � ��������� ��� �����
				sendedFiles.push_back(helpArrayLeftButtonUp);

				//��������� ���� ��� ��������
				MessageBox(0, helpArrayLeftButtonUp, "selected caption", 0);

				//������� ��������� ���������� ����
				iPos = ListView_GetNextItem(ptr->hListView, iPos, LVNI_SELECTED);
			}
			//�������� ������ ���� ���������
			move_info->destination = new char[MAX_PATH];
			strcpy(move_info->destination,fDestName.c_str());
			move_info->countOfMovingFiles = sendedFiles.size();
			move_info->movingFiles = new char*[move_info->countOfMovingFiles];
			//����������������� ������ � ������� �������
			for (int i = 0; i < move_info->countOfMovingFiles; i++) 
			{
				move_info->movingFiles[i] = new char[MAX_PATH];
				strcpy(move_info->movingFiles[i],sendedFiles[i]);
			}
			//������� ���� ��� ��������� ����������� �����
			HANDLE hThread = CreateThread(0, 0, moveFileThread, move_info, 0, 0);
			CloseHandle(hThread);
			}

			break;
		}

		return FALSE;
}