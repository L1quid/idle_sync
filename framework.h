// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
#include <Ws2tcpip.h> // needed for ip_mreq definition for multicast
#include <Wsrm.h>
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

#include "sync_net.h"
#include "sync_msg.h"