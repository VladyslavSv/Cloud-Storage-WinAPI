#include "FTPServer.h"
#include "zip.h"
#include "unzip.h"
#define SUCCESS 0;
#define FAILTURE 1;
int userNameSize = 30;
bool isDirectoryExists(const char *filename)
{
	DWORD dwFileAttributes = GetFileAttributes(filename);
	if (dwFileAttributes == 0xFFFFFFFF)
		return false;
	return dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
}
char* ZipFolder(const char* pathToFolder, const char* pathToZip, HZIP hz)
{
	string currentPath(pathToFolder);
	currentPath += "\\*";

	_finddata_t fd;
	int OK = _findfirst(currentPath.c_str(), &fd);
	if (OK != -1) {
		int result = OK;

		result = _findnext(OK, &fd);
		result = _findnext(OK, &fd);

		string fullPath;
		string zipPath;

		while (result != -1)
		{
			fullPath.clear();
			fullPath += pathToFolder;
			fullPath += "\\";
			fullPath += fd.name;

			zipPath.clear();

			zipPath += pathToZip;
			zipPath += "\\";
			zipPath += fd.name;

			if (isDirectoryExists(fullPath.c_str()))
			{
				ZipAddFolder(hz, zipPath.c_str());
				ZipFolder(fullPath.c_str(), zipPath.c_str(), hz);
			}
			else
			{
				ZipAdd(hz, zipPath.c_str(), fullPath.c_str());
			}
			result = _findnext(OK, &fd);
		}
	}
	return "";
}
char FTPServer::pathToUsersDirectories[MAX_PATH] = "users";
int deleteFileOrDirectory(string path)
{
	path.append("69");                           // добавляем в конец строки два символа, увеличивая её длину
	path[path.size() - 2] = 0;          // заменяем два последних символа, которые только что добавили
	path[path.size() - 1] = 0;

	SHFILEOPSTRUCT sfo = { 0 };

	sfo.hwnd = NULL;
	sfo.wFunc = FO_DELETE;

	sfo.pFrom = path.c_str();
	sfo.pTo = "";

	sfo.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR |
		FOF_MULTIDESTFILES | FOF_SILENT;
	sfo.lpszProgressTitle = "";
	int res = SHFileOperation(&sfo);

	return res;
}
char* getLastWordOfPath(const std::string& subject)
{
	vector<string> container;

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
FTPServer::FTPServer() 
{

}
DWORD WINAPI sendOrReceiveFile(LPVOID lpParam)
{
	InfoForFile* info = (InfoForFile*)lpParam;
	//настройка сокета
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s == SOCKET_ERROR)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
		WSACleanup();
		return 0;
	}

	sockaddr_in local;
	local.sin_addr.S_un.S_addr = INADDR_ANY;
	local.sin_family = AF_INET;
	local.sin_port = htons(11526);

	int res = bind(s, (sockaddr*)&local, sizeof(local));

	if (res == SOCKET_ERROR)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}

	res = listen(s, 10);

	if (res == SOCKET_ERROR)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}
	int iAddrSize = sizeof(sockaddr_in);
	ClientInfo clientinfo;
	clientinfo.socket = accept(s, (sockaddr*)&(clientinfo.addr), &iAddrSize);

	if (clientinfo.socket == INVALID_SOCKET)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
	}

	char fullFileNameOnServer[MAX_PATH];
	FTPServer::receiveBuffer(clientinfo.socket, fullFileNameOnServer, MAX_PATH);

	string str(FTPServer::pathToUsersDirectories);
	str += "\\";
	str += info->userName;
	str += "\\";
	str += fullFileNameOnServer;
	char finalPathChar[MAX_PATH];

	if (info->type == 1) {
		if (!isDirectoryExists(str.c_str())) 
		{
			strcpy_s(finalPathChar, str.substr(str.find_last_of("\\") + 1, string::npos).c_str());
			FTPServer::sendBuffer(clientinfo.socket, finalPathChar, MAX_PATH);
			//file
			FTPServer::sendFile(clientinfo.socket, str.c_str());
		}
		else 
		{
			string fullPathToDirectory("users");
			fullPathToDirectory += "\\";
			fullPathToDirectory += info->userName;
			fullPathToDirectory += "\\";
			fullPathToDirectory += fullFileNameOnServer;

			string zipPath("users");
			zipPath += "\\";
			zipPath += info->userName;
			zipPath += "\\";
			zipPath += fullFileNameOnServer;
			zipPath += ".zip";

			HZIP hz = CreateZip(zipPath.c_str(),0);

			ZipAddFolder(hz, fullPathToDirectory.substr(fullPathToDirectory.find_last_of("\\") + 1, string::npos).c_str());
			ZipFolder(fullPathToDirectory.c_str(), fullPathToDirectory.substr(fullPathToDirectory.find_last_of("\\") + 1, string::npos).c_str(), hz);

			CloseZip(hz);

			string finalPath=zipPath.substr(zipPath.find_last_of("\\") + 1, string::npos);
			strcpy_s(finalPathChar, finalPath.c_str());
			FTPServer::sendBuffer(clientinfo.socket, finalPathChar, MAX_PATH);

			FTPServer::sendFile(clientinfo.socket, zipPath.c_str());

			remove(zipPath.c_str());
		}

	}
	else if (info->type == 2) {
		FTPServer::receiveFile(clientinfo.socket, str.c_str());
	}

	shutdown(s, SD_BOTH);
	closesocket(s);

	return 0;
}
DWORD WINAPI moveFileThread(LPVOID lpParam)
{
	InfoForFile* info = (InfoForFile*)lpParam;

	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s == SOCKET_ERROR)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
		WSACleanup();
		return 0;
	}

	sockaddr_in local;
	local.sin_addr.S_un.S_addr = INADDR_ANY;
	local.sin_family = AF_INET;
	local.sin_port = htons(11527);

	int res = bind(s, (sockaddr*)&local, sizeof(local));

	if (res == SOCKET_ERROR)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}

	res = listen(s, 10);

	if (res == SOCKET_ERROR)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
		closesocket(s);
		WSACleanup();
		return 0;
	}
	int iAddrSize = sizeof(sockaddr_in);
	ClientInfo clientinfo;
	clientinfo.socket = accept(s, (sockaddr*)&(clientinfo.addr), &iAddrSize);

	if (clientinfo.socket == INVALID_SOCKET)
	{
		FTPServer::MessageAboutError(WSAGetLastError());
	}

	const int helpArraySize = 10;
	int countOfElems = -1;
	char destination[MAX_PATH];
	char singleFileName[MAX_PATH];
	char helpArray[helpArraySize];
	bool isSuccess = true;
	string singleFileNameStr;
	string fullFolderDest;
	string newFileName;
	//получим директорию для перемещения
	FTPServer::receiveBuffer(clientinfo.socket, destination, MAX_PATH);
	//сформируем полный путь для перемещения
	fullFolderDest = FTPServer::pathToUsersDirectories;
	fullFolderDest += "\\";
	fullFolderDest += info->userName;
	fullFolderDest += "\\";
	fullFolderDest += destination;
	//получим количество файлов для перемещения
	FTPServer::receiveBuffer(clientinfo.socket, helpArray, helpArraySize);
	//конвертируем в число
	countOfElems = atoi(helpArray);
	if (countOfElems != -1)
	{
		//перемещаем по одному
		for (int i = 0; i < countOfElems; i++) 
		{
			FTPServer::receiveBuffer(clientinfo.socket, singleFileName, MAX_PATH);
			//соберем полный путь к файлу
			singleFileNameStr = FTPServer::pathToUsersDirectories;
			singleFileNameStr += "\\";
			singleFileNameStr += info->userName;
			singleFileNameStr += "\\";
			singleFileNameStr += singleFileName;
			//получим путь к новому файлу
			newFileName = fullFolderDest;
			newFileName += "\\";
			newFileName += getLastWordOfPath(singleFileNameStr);

			if (!MoveFile(singleFileNameStr.c_str(), newFileName.c_str())) 
			{
				isSuccess = false;
			}
		}
	}
	if (isSuccess) 
	{
		strcpy(helpArray, "0");
		FTPServer::sendBuffer(clientinfo.socket, helpArray, helpArraySize);
	}
	else 
	{
		strcpy(helpArray, "1");
		FTPServer::sendBuffer(clientinfo.socket, helpArray, helpArraySize);
	}

	shutdown(s, SD_BOTH);
	closesocket(s);
}
DWORD WINAPI executorForUser(LPVOID lpParam)
{
	ClientInfo* infoAboutUser = (ClientInfo*)lpParam;
	char buffer[4];
	int  numberFromUser= 0;
	while (true) 
	{
		numberFromUser = FTPServer::receiveBuffer(infoAboutUser->socket,buffer,4);

		if (numberFromUser == SOCKET_ERROR)
		{
			FTPServer::MessageAboutError(WSAGetLastError());
			shutdown(infoAboutUser->socket, SD_BOTH); // SD_BOTH запрещает как прием, так и отправку данных
			closesocket(infoAboutUser->socket);
			return FAILTURE;
		}

		if (!strcmp(buffer, "101")) 
		{
			cout << infoAboutUser->ipName << " запрос на вход.\n";
			FTPServer::getUserInfoAndCheck(infoAboutUser);
		}
		else if (!strcmp(buffer, "102"))
		{
			cout << infoAboutUser->ipName << " запрос на регистрацию.\n";
			FTPServer::addNewUser(infoAboutUser->socket);
		}
		else if (!strcmp(buffer, "103")) 
		{
			cout << infoAboutUser->ipName << " запрос на получение всех файлов из папки пользователя.\n";
			if (strcmp(infoAboutUser->userName, "none")) 
			{
				//получим имя дериктории
				string path(infoAboutUser->userName);
				char pathToFolder[MAX_PATH];
				FTPServer::receiveBuffer(infoAboutUser->socket, pathToFolder, MAX_PATH);
				if (strcmp(pathToFolder, "")) 
				{
					path += "\\";
				}
				path += pathToFolder;
				//отправим имена файлов клиенту
				FTPServer::sendAllFilesInDirectory(infoAboutUser->socket, path.c_str());
			}
			else 
			{
				cout << "Попытка получения файлов неверифицированного пользователя.\n";
			}
		}
		else if (!strcmp(buffer, "104")) 
		{
			if (strcmp(infoAboutUser->userName, "none"))
			{
				cout << infoAboutUser->ipName << " запрос на получение файла.\n";

				InfoForFile* info = new InfoForFile;
				info->type = 1;
				info->userName = new char[userNameSize];
				strcpy(info->userName, infoAboutUser->userName);

				HANDLE hThread = CreateThread(0, 0, sendOrReceiveFile, info, 0, 0);
				CloseHandle(hThread);
			}
			else
			{
				cout << "Попытка получить файл неверифицированного пользователя.\n";
			}
		}
		else if (!strcmp(buffer, "105")) 
		{
			if (strcmp(infoAboutUser->userName, "none")) 
			{
				cout << infoAboutUser->ipName << " запрос на отправку файла.\n";

				InfoForFile* info = new InfoForFile;
				info->type = 2;
				info->userName = new char[userNameSize];
				strcpy(info->userName, infoAboutUser->userName);

				HANDLE hThread = CreateThread(0, 0, sendOrReceiveFile, info, 0, 0);
				CloseHandle(hThread);
			}
			else
			{
				cout << "Попытка отправить файл неверифицированного пользователя.\n";
			}
		}
		else if (!strcmp(buffer, "106")) 
		{
			if (strcmp(infoAboutUser->userName, "none")) {
				cout << infoAboutUser->ipName << " запрос на удаление файла.\n";

				char pathFromUser[MAX_PATH];
				FTPServer::receiveBuffer(infoAboutUser->socket, pathFromUser, MAX_PATH);

				char answer[1] = { '1' };

				string str(FTPServer::pathToUsersDirectories);
				str += "\\";
				str += infoAboutUser->userName;
				str += "\\";
				str += pathFromUser;

				//cout << pth<<endl;
				if (!deleteFileOrDirectory(str.c_str())) //успех
				{
					answer[0] = '0';
				}
				else//неудача
				{
					answer[0] = '1';
				}

				FTPServer::sendBuffer(infoAboutUser->socket, answer, 1);
			}
			else 
			{
				cout << "Попытка удалить файл неверифицированного пользователя.\n";
			}
		}
		else if (!strcmp(buffer, "107")) 
		{
			cout << infoAboutUser->ipName << " запрос на создание папки.\n";

			char answer[1] = { '1' };
			char pathToNewFolder[MAX_PATH];

			FTPServer::receiveBuffer(infoAboutUser->socket, pathToNewFolder, MAX_PATH);
			
			string str(FTPServer::pathToUsersDirectories);
			str += "\\";
			str += infoAboutUser->userName;
			str += "\\";
			str += pathToNewFolder;

			if (CreateDirectory(str.c_str(), NULL))
			{
				answer[0] = '0';
			}

			FTPServer::sendBuffer(infoAboutUser->socket, answer, 1);
		}
		else if (!strcmp(buffer, "108")) 
		{
			cout << infoAboutUser->ipName << " запрос на перемещение файла.\n";

			InfoForFile* info = new InfoForFile;
			info->type = 3;
			info->userName = new char[userNameSize];
			strcpy(info->userName, infoAboutUser->userName);

			HANDLE hThread = CreateThread(0, 0, moveFileThread, info, 0, 0);
			CloseHandle(hThread);
		}
		else if (!strcmp(buffer, "99")) 
		{
			closesocket(infoAboutUser->socket);
			break;
		}
	}
	cout << "Связь с клиентом " <<infoAboutUser->ipName << " потеряна.\n";
	return SUCCESS;
}

void FTPServer::MessageAboutError(DWORD dwError)
{
	setlocale(LC_ALL, "Russian");
	LPVOID lpMsgBuf = NULL;
	char szBuf[300];
	//Функция FormatMessage форматирует строку сообщения
	BOOL fOK = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM /* флаг сообщает функции, что нужна строка, соответствующая коду ошибки, определённому в системе */
		| FORMAT_MESSAGE_ALLOCATE_BUFFER, //нужно выделить соответствующий блок памяти для хранения текста
		NULL, //указатель на строку, содержащую текст сообщения
		dwError, //код ошибки
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // язык, на котором выводится описание ошибки (язык пользователя по умолчанию)
		(LPTSTR)&lpMsgBuf, //указатель на буфер, в который запишется текст сообщения
		0, //минимальный размер буфера для выделения памяти - память выделяет система
		NULL //список аргументов форматирования
	);
	if (lpMsgBuf != NULL)
	{
		wsprintf(szBuf, "Warning %d: %s \n", dwError, lpMsgBuf);
		cout << szBuf;
		LocalFree(lpMsgBuf); //освобождаем память, выделенную системой
	}
}
int FTPServer::initAndListen() 
{
	setlocale(LC_ALL, "Russian");

	WSAStartup(WINSOCK_VERSION,&wsd);
	mainServerSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

	if (mainServerSocket == SOCKET_ERROR)
	{
		MessageAboutError(WSAGetLastError());
		WSACleanup();
		return 0;
	}

	mainServerSocketAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	mainServerSocketAddr.sin_family = AF_INET;
	mainServerSocketAddr.sin_port = htons(11525);

	int res = bind(mainServerSocket,(sockaddr*)&mainServerSocketAddr,sizeof(mainServerSocketAddr));
	if (res == SOCKET_ERROR)
	{
		MessageAboutError(WSAGetLastError());
		closesocket(mainServerSocket);
		WSACleanup();
		return 0;
	}

	res = listen(mainServerSocket,10 /*максимальную длину очереди соединений*/);
	if (res == SOCKET_ERROR)
	{
		MessageAboutError(WSAGetLastError());
		closesocket(mainServerSocket);
		WSACleanup();
		return 0;
	}
	//ожидание подключений от клиентов
	cout << "Сервер ожидает подключения клиентов.\n";
	while (true)
	{

		int iAddrSize = sizeof(sockaddr_in);
		
		ClientInfo clientinfo;//<<------сдесь может быть ошибка (Возможно стоит сделать указатель)
		//accept принимает соединения клиентов и возвращает новый дескриптор сокета
		clientinfo.socket = accept(mainServerSocket,(sockaddr*)&(clientinfo.addr),&iAddrSize);

		if (clientinfo.socket == INVALID_SOCKET)
		{
			MessageAboutError(WSAGetLastError());
			break;
		}
		clientinfo.ipName = new char[50];
		clientinfo.userName = new char[userNameSize];
		wsprintf(clientinfo.userName,"none");
		wsprintf(clientinfo.ipName, "%s", inet_ntoa(clientinfo.addr.sin_addr));
		cout <<"Клиент "<<clientinfo.ipName <<" присоединился к серверу.\n";
		//отдельный поток для обработки запросов от клиента
		HANDLE hThread = CreateThread(0, 0, executorForUser, (LPVOID)&clientinfo, 0, 0);
		CloseHandle(hThread);
	}
	closesocket(mainServerSocket);
	WSACleanup();
	return 0;
}
bool FTPServer::sendAllFilesInDirectory(SOCKET& usersSocket,const char* pathFromClient) 
{
	char helpStr[50];
	int amount = 0;
	int size = 0;
	//формируем путь к папке
	std::string path(pathToUsersDirectories);
	path += "\\";
	path += pathFromClient;
	//получение количества файлов в папке
	amount = AmountOfFiles(path.c_str());
	sprintf_s(helpStr, "%d", amount - 2);
	//отправка количества файлов в папке
	sendBuffer(usersSocket, helpStr, 50);
	//для получения всех файлов из папки
	path += "\\*";
	//ищем первый файл
	_finddata_t fd;
	int OK = _findfirst(path.c_str(), &fd);
	int result = OK;
	//перескакиваем на 2 файла вперед
	result = _findnext(OK, &fd);
	result = _findnext(OK, &fd);
	//отправка файлов по одному
	while (result != -1)
	{
		//отправим размер строки (имени файла)
		size = strlen(fd.name);
		wsprintf(helpStr, "%d", size);
		sendBuffer(usersSocket, helpStr, 50);
		//отправим строку(имя файла)
		sendBuffer(usersSocket, fd.name, size);

		//буфер для размера файла
		char sizeBuf[10];

		string fullPath= pathToUsersDirectories;
		fullPath += "\\";
		fullPath += pathFromClient;
		fullPath += "\\";
		fullPath += fd.name;

		if (isDirectoryExists(fullPath.c_str()))
		{
			strcpy(sizeBuf, "Папка");
		}
		else 
		{
			double size = fd.size;

			int degree = 1;
			while (size > 1024)
			{
				size /= 1024;
				degree++;
			}
			size = round(size * 10) / 10;
			ostringstream strs;
			strs << size;
			switch (degree)
			{
			case 1:
				strs << " b";
				break;
			case 2:
				strs << " kb";
				break;
			case 3:
				strs << " mb";
				break;
			case 4:
				strs << " gb";
				break;
			}
			string str = strs.str();
			strcpy_s(sizeBuf, str.c_str());
		}
		sendBuffer(usersSocket, sizeBuf, 10);
		//получаем следующий файл(в случае удачи)
		result = _findnext(OK, &fd);
	}
	// Функция _findclose закрывает поиск
	_findclose(OK);

	return true;
}
bool FTPServer::writeToFileNewUser(const char* user) {

	std::ofstream fout;
	fout.open("users.txt", std::ios_base::app);
	if (!fout.is_open()) {
		return false;
	}
	fout << user;
	fout.close();
	return true;
}
bool FTPServer::addNewUser(SOCKET& usersSocket) 
{
	char login[50];
	char password[50];
	char response[1] = {'0'};

	receiveBuffer(usersSocket, login, 50);
	receiveBuffer(usersSocket, password, 50);

	string writedString;
	writedString += login;
	writedString += " ";
	writedString += password;
	writedString += "\n";

	if (checkUserInFile(writedString.c_str())) //если пользователь найден отправим код ошибки
	{
		response[0] = '1';
	}
	else {
		if (!writeToFileNewUser(writedString.c_str())) //если записать не удалось
		{
			response[0] = '1';
		}
		else //если записать удалось идём дальше
		{
			string path(pathToUsersDirectories);
			path += "\\";
			path += login;
			if (CreateDirectory(path.c_str(), NULL)) {
				response[0] = '0';
			}
			else {
				response[0] = '1';
			}
		}
	}

	sendBuffer(usersSocket, response, 1);

	if (response[0] == '0')
		return true;
	else
		return false;
}
void FTPServer::getUserInfoAndCheck(ClientInfo* clientInfo)
{
	char login[50];
	char password[50];
	char answer[1] = {'0'};
	//получим логин и пароль от клиента
	receiveBuffer(clientInfo->socket, login, 50);
	receiveBuffer(clientInfo->socket, password, 50);
	//соединим логин и пароль в одну строку
	std::string writingString;
	writingString += login;
	writingString += " ";
	writingString += password;
	//проверим пользователя в файле
	if (checkUserInFile(writingString.c_str())) {
		answer[0] = '0';
		strcpy(clientInfo->userName, login);
	}
	else
		answer[0] = '1';
	//отправим пользователю результат проверки
	sendBuffer(clientInfo->socket, answer, 1);
}
bool FTPServer::checkUserInFile(const char* user) {
	std::ifstream fin;
	fin.open("users.txt");
	if (!fin.is_open()) {
		return false;
	}
	char buff[50];
	fin.getline(buff, 50);
	while (strcmp(buff, "")) {
		if (!strcmp(buff, user)) {
			return true;
		}
		fin.getline(buff, 50);
	}
	return false;
}
int FTPServer::sendBuffer(SOCKET& currentSocket, char* buffer, unsigned int length)
{
	unsigned int bytesSend = 0;
	while (bytesSend < length) {
		int ret = send(currentSocket, buffer + bytesSend, length - bytesSend, 0);
		if (ret <= 0) {
			return FAILTURE;
		}
		bytesSend += ret;
	}
	return SUCCESS;
}
int FTPServer::receiveBuffer(SOCKET& currentSocket, char* buffer, int length)
{
	unsigned int bytesReceived = 0;
	while (bytesReceived < length) {
		int ret = recv(currentSocket, buffer + bytesReceived, length - bytesReceived, 0);
		if (ret <= 0) {
			return FAILTURE;
		}
		bytesReceived += ret;
	}
	return SUCCESS;
}
int FTPServer::sendFile(SOCKET& currentSocket,const char* fileName)
{
	//отправим размер файла
	char helpBuf[20];
	sprintf(helpBuf, "%d", fileSize(fileName));
	sendBuffer(currentSocket, helpBuf, 20);

	//создается файл и открывается(для чтения)
	FILE* file;
	int openFileResult = fopen_s(&file, fileName, "rb");
	//буфер для отправки файла
	char buf[128];
	//буфер для отправки available
	char availableBuf[10];

	int available = 0;
	do {
		//получим available(количество байт которые считали из файла)
		available = fread(buf, sizeof(char), 128, file);
		//перевод перевод числа в массив
		sprintf(availableBuf, "%d", available);
		//отправим available
		sendBuffer(currentSocket, availableBuf, 10);
		//отправим байты
		sendBuffer(currentSocket, buf, available);
	} while (!feof(file));
	//отправим оповещение о конце файла
	available = -1;
	sprintf(availableBuf, "%d", available);
	sendBuffer(currentSocket, availableBuf, 10);
	//освободим ресурсы
	fclose(file);
	return SUCCESS;
}
int FTPServer::receiveFile(SOCKET& currentSocket,const char* fileName)
{
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
		//запишем байты в новый файл
		fwrite(buf, sizeof(char), available, file);
	} while (true);
	//высвобождаем ресурсы
	fclose(file);

	return SUCCESS;
}
int FTPServer::AmountOfFiles(const char * path)
{
	int amount = 0;
	_finddata_t fd;
	char pathname[MAX_PATH];
	strcpy_s(pathname, MAX_PATH, path);
	strcat_s(pathname, "\\*");
	int OK = _findfirst(pathname, &fd);
	int result = OK;
	while (result != -1)
	{
		amount++;
		result = _findnext(OK, &fd);
	}
	_findclose(OK); // Функция _findclose закрывает поиск
	return amount;
}
long FTPServer::fileSize(const char *fileName) 
{
	fstream file(fileName);
	long size = 0;
	file.seekg(0, std::ios::end);
	size = file.tellg();
	return size;
}