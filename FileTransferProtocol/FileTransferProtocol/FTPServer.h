#pragma once
#include "additionalLibs.h"
struct ClientInfo
{
	sockaddr_in addr;
	SOCKET socket;
	char* ipName;
	char* userName;
};
struct InfoForFile 
{
	char* userName;
	int type;
};
class FTPServer 
{
private:
	WSADATA wsd;
	SOCKET mainServerSocket;
	sockaddr_in mainServerSocketAddr;
public:
	static char pathToUsersDirectories[MAX_PATH];
	static void MessageAboutError(DWORD dwError);
	static int sendBuffer(SOCKET& currentSocket,char* buffer, unsigned int length);
	static int receiveBuffer(SOCKET& currentSocket, char* buffer, int length);
	static int sendFile(SOCKET& currentSocket, const char* fileName);
	static int receiveFile(SOCKET& currentSocket, const char* fileName);
	static void getUserInfoAndCheck(ClientInfo* clientInfo);
	static bool checkUserInFile(const char* user);
	static bool addNewUser(SOCKET& usersSocket);
	static bool writeToFileNewUser(const char* user);
	static bool sendAllFilesInDirectory(SOCKET& usersSocket, const  char* userName);
	static int AmountOfFiles(const char * path);
	static long fileSize(const char *fileName);
	FTPServer();
	int initAndListen();
};