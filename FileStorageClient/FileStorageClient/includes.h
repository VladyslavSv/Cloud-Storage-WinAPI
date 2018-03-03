#pragma once

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include "Winsock2.h"
#include "Ws2tcpip.h"
#include <windows.h>
#include <windowsX.h>
#include <tchar.h>
#include "resource.h"
#include <shlobj.h>
#include <sstream>
#include <string>
#include <fstream>
#include <regex>
#include <Commctrl.h>
#include <Strsafe.h>
#include <vector>
#include <cmath>
#include <mutex>

using namespace std;
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib, "comctl32.lib")
void messageAboutError(DWORD dwError);
