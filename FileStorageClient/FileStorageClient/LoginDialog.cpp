#include "LoginDialog.h"
#include "MainDialog.h"
LoginDialog* LoginDialog::ptr = NULL;
bool LoginDialog::isConnected = false;
LoginDialog::LoginDialog(WSADATA& wsdA, SOCKET& sClientA, sockaddr_in& srvA, HINSTANCE& hInst):wsd(wsdA),sClient(sClientA),srv(srvA),hInst(hInst)
{
	ptr = this;
}
LoginDialog::~LoginDialog()
{

}
int connectToServer()
{
	LoginDialog::ptr->sClient = socket(AF_INET /*The Internet Protocol version 4 (IPv4) address family*/,
		SOCK_STREAM /*The type specification for the new socket*/,
		IPPROTO_TCP /*Protocol to be used with the socket that is specific to the indicated address family*/);
	// The socket function creates a socket that is bound to a specific transport service provider.
	if (LoginDialog::ptr->sClient == SOCKET_ERROR)
	{ 
		messageAboutError(WSAGetLastError());
		WSACleanup(); // The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
		return 1;
	}
	LoginDialog::ptr->srv.sin_family = AF_INET; // Address family (must be AF_INET).
	LoginDialog::ptr->srv.sin_port = htons(11525); // IP port
	char ip_address[20] = "195.138.81.175";
	LoginDialog::ptr->srv.sin_addr.S_un.S_addr = inet_addr(ip_address); // IP address.
	//													 // The inet_addr function converts a string containing an IPv4 dotted-decimal address into a proper address for the IN_ADDR structure.

	if (connect(LoginDialog::ptr->sClient /*ТСР-сокет для установления соединения */, (sockaddr*)&(LoginDialog::ptr->srv), sizeof(LoginDialog::ptr->srv)) == SOCKET_ERROR)
	{
		closesocket(LoginDialog::ptr->sClient); // The closesocket function closes an existing socket.
		WSACleanup(); // The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
		return 1;
	}
	return 0;
}
bool LoginDialog::SendUserNameAndPassword() {

	//массив для получения ответа от сервера
	char response[1] = { '0' };
	//получим лоигн и пароль из текстовых полей
	char login[50];
	char password[50];
	GetWindowText(GetDlgItem(ptr->hWindow, IDC_EDIT_LOGIN), login, 50);
	GetWindowText(GetDlgItem(ptr->hWindow, IDC_EDIT_PASSWORD), password, 50);
	//отправим запрос на вход
	MainDialog::queryToServer("101");
	//отправим логин и пароль
	sendBuffer(LoginDialog::ptr->sClient, login, 50);
	sendBuffer(LoginDialog::ptr->sClient, password, 50);
	//получим ответ
	receiveBuffer(LoginDialog::ptr->sClient, response, 1);

	if (response[0] == '0')
		return true;
	else
		return false;
}

void LoginDialog::Cls_OnClose(HWND hwnd)
{
	//сообщение о разрыве соединения
	MainDialog::queryToServer("99");
	//высвободим все ресурсы
	shutdown(sClient, SD_BOTH); // SD_BOTH запрещает как прием, так и отправку данных
	closesocket(sClient); // The closesocket function closes an existing socket.
	WSACleanup(); // The WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll).
	EndDialog(hwnd, 0);
}
BOOL LoginDialog::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	hWindow = hwnd;

	HICON hIcon1 = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hWindow, WM_SETICON, 1, (LPARAM)hIcon1);


	if (LoginDialog::isConnected == false) {
		if (WSAStartup(WINSOCK_VERSION, &wsd))
		{
			messageAboutError(WSAGetLastError());
		}
		//пытаемся присоединится к серверу
		if (connectToServer() == 0)
		{
			LoginDialog::isConnected = true;
		}
		else
		{
			LoginDialog::isConnected = false;
			MessageBox(hWindow, "Не получилось соединится с сервером.", "Ошибка", 0);
		}

	}
	return TRUE;
}
void LoginDialog::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
		if (id == IDC_ENTER) 
		{
			if (isConnected) 
			{
				//отправим запрос на вход
				if (SendUserNameAndPassword())
				{
					EndDialog(hWindow, 127);
				}
				else
				{
					MessageBox(hWindow, "Неправильный логин или пароль.", "Ошибка", 0);
				}
			}
			else 
			{
				MessageBox(hWindow, "Отсутствует соединение.", "Ошибка", 0);
			}
		}
		if (id == IDB_CHECKIN_L)
		{
			if (LoginDialog::isConnected == true) 
			{
				EndDialog(hWindow, 125);
			}
			else 
			{
				MessageBox(hWindow, "Отсутствует соединение.", "Ошибка", 0);
			}
		}

}
BOOL CALLBACK LoginDialog::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}
int LoginDialog::sendBuffer(SOCKET& currentSocket, char* buffer, int length)
{
	unsigned int bytesSend = 0;
	while (bytesSend < length) {
		int ret = send(currentSocket, buffer + bytesSend, length - bytesSend, 0);
		if (ret <= 0) {
			return 1;
		}
		bytesSend += ret;
	}
	return 0;
}
int LoginDialog::receiveBuffer(SOCKET& currentSocket, char* buffer, int length)
{
	unsigned int bytesReceived = 0;
	while (bytesReceived < length) {
		int ret = recv(currentSocket, buffer + bytesReceived, length - bytesReceived, 0);
		if (ret <= 0) {
			return 1;
		}
		bytesReceived += ret;
	}
	return 0;
}
long LoginDialog::getFileSize(char *fileName)
{
	fstream file(fileName);
	long size = 0;
	file.seekg(0, std::ios::end);
	size = file.tellg();
	return size;
}
int LoginDialog::sendFile(SOCKET& currentSocket, char* fileName)
{
	//размер файла
	long fileSize = getFileSize(fileName);

	int percents = 1;
	//создается файл и открывается(для чтения)
	FILE* file;
	int openFileResult = fopen_s(&file, fileName, "rb");
	//буфер для отправки файла
	char buf[128];
	//буфер для отправки available
	char availableBuf[10];
	//количество считанных байт
	int available = 0;
	//всего отправлено байт
	long totalAvailable = 0;

	do {
		//получим available(количество байт которые считали из файла)
		available = fread(buf, sizeof(char), 128, file);
		//перевод перевод числа в массив
		sprintf(availableBuf, "%d", available);
		//отправим available
		sendBuffer(currentSocket, availableBuf, 10);
		//отправим байты
		sendBuffer(currentSocket, buf, available);
		//прибавим количество отправленных байт
		totalAvailable += available;
		if (fileSize != -1&&totalAvailable > (fileSize / 100 * percents))
		{
			if (totalAvailable == fileSize&&percents==1) 
			{
				for (int i = 0; i < 100; i++) 
				{
					SendMessage(MainDialog::ptr->hProgressBar, PBM_STEPIT, 0, 0);
				}
			}
			else {
				int difference = (totalAvailable / (fileSize / 100) - percents);
				for (int i = 0; i < difference; i++)
				{
					percents++;
					if (percents <= 100) {
						SendMessage(MainDialog::ptr->hProgressBar, PBM_STEPIT, 0, 0);
					}
				}
			}	
		}

	} while (!feof(file));
	//отправим оповещение о конце файла
	available = -1;
	sprintf(availableBuf, "%d", available);
	sendBuffer(currentSocket, availableBuf, 10);
	//освободим ресурсы
	fclose(file);
	SendMessage(MainDialog::ptr->hProgressBar, PBM_SETPOS, 100, 0);
	return 0;
}
int LoginDialog::receiveFile(SOCKET& currentSocket,const char* fileName)
{
	//получим размер файла
	char helpBuffer[20];
	receiveBuffer(currentSocket, helpBuffer, 20);
	long fileSize = strtol(helpBuffer, nullptr, 0);
	//процент скачивания файла
	int percents = 1;
	//всего получено байт
	long totalAvailable = 0;
	//создается файл и открывается(для записи)
	FILE* file;
	int openFileResult = fopen_s(&file, fileName, "wb");
	//буфер для получения файла
	char buf[128];
	//receivedNumber вспомогательный буфер для получение значения available(количество получаемых байт)
	char availableBuf[10];
	//количество байт которые считал клиент из файла
	int available = 0;
	do {
		//получим available
		receiveBuffer(currentSocket, availableBuf, 10);
		//конвертируем массив в число
		available = strtol(availableBuf, nullptr, 0);
		//available -1 приходит когда файл закончился
		if (available == -1)
			break;
		//получим байты
		receiveBuffer(currentSocket, buf, available);
		//прибавим количество прочитанных байт
		totalAvailable += available;
		if (fileSize != 0 && totalAvailable > (fileSize / 100 * percents))
		{
			if (totalAvailable == fileSize&&percents == 1)
			{
				for (int i = 0; i < 100; i++)
				{
					SendMessage(MainDialog::ptr->hProgressBar, PBM_STEPIT, 0, 0);
				}
			}
			else {
				int difference = (totalAvailable / (fileSize / 100) - percents);
				for (int i = 0; i < difference; i++)
				{
					percents++;
					if (percents <= 100) {
						SendMessage(MainDialog::ptr->hProgressBar, PBM_STEPIT, 0, 0);
					}
				}
			}

		}
		//запишем байты в новый файл
		fwrite(buf, sizeof(char), available, file);
	} while (true);
	//высвобождаем ресурсы
	fclose(file);	
	SendMessage(MainDialog::ptr->hProgressBar, PBM_SETPOS, 100, 0);
	return 0;
}