#include "FTPServer.h"
void main() 
{
	FTPServer server;
	server.initAndListen();
	system("pause");
}