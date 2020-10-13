#pragma once
#pragma comment(lib, "Ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <string.h>
#define _WIN32_WINNT 0x501
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sstream>
#include <stdint.h>
#include <fstream>
#include <time.h>

using namespace std;
// This is the port that must be stisfied for the external engine creation in netapp.
#define PORT 6789
#define HEADER_LEN 6
#define READ_TIMEOUT 2
#define WRITE_TIMEOUT 2