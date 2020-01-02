// idle_sync.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "idle_sync.h"

#include <string>
#include <vector>

#define MAX_LOADSTRING 100
#define TICK_INTERVAL 1000
#define TICK_TIMER_ID 0x3CA
#define SYNC_MASTER 1
#define SYNC_AUX 2
#define UI_PADDING 16

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
char szHostname[MAX_LOADSTRING];
int g_mode = SYNC_MASTER;
SyncNet* g_net = NULL;
HWND g_hwnd = NULL;
std::string g_group;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL is_screensaver_running();
void master_tick();
void aux_tick();
void update_mode_menu_state(HWND hwnd);
void send_heartbeat();
void read_msgs();
void dmsg(const char* msg);
void process_sync_msg(SyncMsg* msg);
void start_screensaver();
void stop_screensaver();
BOOL CALLBACK kill_screensaver(HWND hwnd, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPWSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  g_group = "DG";

  memset(szHostname, 0, MAX_LOADSTRING);
  DWORD nb = MAX_LOADSTRING;
  GetComputerNameA(szHostname, (LPDWORD)&nb);


  WSADATA wsaData;
  if (WSAStartup(0x0101, &wsaData)) {
    dmsg((LPCSTR)"WSAStartup");
    return(1);
  }

  g_net = new SyncNet();

  if (!g_net->ready())
  {
    dmsg((LPCSTR)"SyncNet not ready");
    return(1);
  }

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_IDLESYNC, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow))
  {
    return FALSE;
  }

  HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IDLESYNC));

  MSG msg;

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0))
  {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IDLESYNC));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_IDLESYNC);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance; // Store instance handle in our global variable

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, 0, 500, 500, nullptr, nullptr, hInstance, nullptr);

  if (!hWnd)
  {
    return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static HWND hwnd_edit = NULL;
  g_hwnd = hWnd;
  static int calls = 0;

  switch (message)
  {
  case WM_CREATE:
    SetTimer(hWnd, TICK_TIMER_ID, TICK_INTERVAL, NULL);
    hwnd_edit = CreateWindowEx(0, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 0, 0, 0, 0, hWnd, (HMENU)IDC_LOG, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

    update_mode_menu_state(hWnd);
    return(0);
  case WM_TIMER:
    switch (wParam)
    {
    case TICK_TIMER_ID:
      switch (g_mode)
      {
      case SYNC_MASTER:
        switch (calls++)
        {
        case 5:
          //start_screensaver();
          break;
        case 10:
          //stop_screensaver();
          break;
        }

        master_tick();
        break;
      case SYNC_AUX:
        aux_tick();
        break;
      }

      return(0);
    }
    break;
  case WM_COMMAND:
  {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId)
    {
    case IDM_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    case IDM_MODE_MASTER:
      g_mode = SYNC_MASTER;
      dmsg("Setting mode to MASTER");
      update_mode_menu_state(hWnd);
      break;

    case IDM_MODE_AUX:
      g_mode = SYNC_AUX;
      dmsg("Setting mode to AUX");
      update_mode_menu_state(hWnd);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  }
  break;
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...
    EndPaint(hWnd, &ps);
  }
  break;
  case WM_SIZE:
    MoveWindow(hwnd_edit, UI_PADDING, UI_PADDING, LOWORD(lParam) - (UI_PADDING * 2), HIWORD(lParam) - (UI_PADDING * 2), TRUE);
    return(0);
  case WM_DESTROY:
    KillTimer(hWnd, TICK_TIMER_ID);
    WSACleanup();
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  UNREFERENCED_PARAMETER(lParam);
  switch (message)
  {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
    {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}

BOOL is_screensaver_running()
{
  BOOL ret = false;
  BOOL call_status = SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &ret, 0);

  //dmsg((LPCSTR)(ret ? "  Screensaver is ON" : "  Screensaver is OFF"));

  return(ret);
}

void master_tick()
{
  //dmsg((LPCSTR)"Master Tick");

  send_heartbeat();
  read_msgs();
}

void aux_tick()
{
  //dmsg((LPCSTR)"Aux Tick");

  send_heartbeat();
  read_msgs();

  // AUX
    // start listening for messages from master
    // broadcast aux heartbeat
    // listen for screensaver_on message; avoid screensaver if not received
}

void update_mode_menu_state(HWND hwnd)
{
  HMENU menu = GetMenu(hwnd);

  switch (g_mode)
  {
  case SYNC_MASTER:
    CheckMenuItem(menu, IDM_MODE_MASTER, MF_CHECKED);
    CheckMenuItem(menu, IDM_MODE_AUX, MF_UNCHECKED);
    break;
  case SYNC_AUX:
    CheckMenuItem(menu, IDM_MODE_MASTER, MF_UNCHECKED);
    CheckMenuItem(menu, IDM_MODE_AUX, MF_CHECKED);
    break;
  }
}

void send_heartbeat()
{
  // send master heartbeat
  BOOL screensaver_running = is_screensaver_running();
  std::string msg;

  char tmp[8];
  memset(tmp, 0, 8);
  _itoa_s(g_mode, tmp, 10);

  msg.append(g_group);
  msg.append(" ");
  msg.append(tmp);
  msg.append(" ");
  msg.append(szHostname);
  msg.append(" ");

  memset(tmp, 0, 8);
  _itoa_s(g_mode == SYNC_MASTER ? SYNC_MSG_MASTER_HEARTBEAT : SYNC_MSG_AUX_HEARTBEAT, tmp, 10);
  msg.append(tmp);

  if (g_mode == SYNC_MASTER)
  {
    msg.append(" ");

    if (screensaver_running)
      msg.append("1");
    else
      msg.append("0");
  }

  //dmsg("Sending heartbeat: ");
  //dmsg(msg.c_str());

  g_net->write(msg.c_str());
}

// does not handle incomplete messages yet
void read_msgs()
{
  while (true)
  {
    char* ctx = NULL;
    char buf[SYNC_NET_MSG_SIZE + 1];
    memset(buf, '\0', sizeof(buf));

    g_net->read(buf, SYNC_NET_MSG_SIZE);

    if (strlen(buf) == 0)
      break;

    SyncMsg msg;

    char* token = strtok_s(buf, " ", &ctx);
    int i = 0;

    while (token != NULL)
    {
      std::string val = token;

      switch (i)
      {
      case 0:
        msg.group_name = val;
        break;
      case 1:
        msg.sender_mode = std::stoi(token);
        break;
      case 2:
        msg.sender_hostname = val;
        break;
      case 3:
        msg.msg_type = std::stoi(token);
        break;
      default:
        msg.params.push_back(val);
        break;
      }

      i++;
      token = strtok_s(NULL, " ", &ctx);
    }

    process_sync_msg(&msg);
  }
}

void dmsg(const char* msg)
{
  OutputDebugStringA(msg);
  OutputDebugStringA("\r\n");

  int len = (int)strlen(msg);
  int wlen = MultiByteToWideChar(CP_ACP, 0, msg, len, NULL, 0);
  wchar_t* wtext = new wchar_t[wlen + 3];
  MultiByteToWideChar(CP_ACP, 0, msg, len, wtext, wlen);
  wtext[wlen] = '\r';
  wtext[wlen + 1] = '\n';
  wtext[wlen + 2] = 0;

  HWND hwnd = GetWindow(g_hwnd, GW_CHILD);
  len = GetWindowTextLengthA(hwnd);
  SendMessage(hwnd, EM_SETSEL, WPARAM(len), LPARAM(len));
  SendMessage(hwnd, EM_REPLACESEL, WPARAM(false), LPARAM(wtext));
  delete[] wtext;
}

void process_sync_msg(SyncMsg* msg)
{
  // only process *our* sync msgs
  if (msg->group_name != g_group)
  {
    //dmsg("Mismatched group names!");
    return;
  }

  // skip messages from self or siblings (in case of aux)
  if (msg->sender_mode == g_mode)
  {
    //dmsg("Skipping same-mode message");
    return;
  }

  //dmsg("Acting on message!");

  switch (msg->msg_type)
  {
  case SYNC_MSG_MASTER_HEARTBEAT:
    //dmsg("Received Master Heartbeat");
    if (msg->params[0] == std::string("0"))
      stop_screensaver();
    else
      start_screensaver();
    break;
  case SYNC_MSG_AUX_HEARTBEAT:
    //dmsg("Received Aux Heartbeat");
    break;
  default:
    dmsg("We don't know how to parse this!");
  }
}

void start_screensaver()
{
  dmsg("Start screensaver");
  PostMessage(g_hwnd, WM_SYSCOMMAND, SC_SCREENSAVE, 0);
}

void stop_screensaver()
{
  //dmsg("Prevent Screensaver");
  // reset input idle timer
  keybd_event((BYTE)VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);

  SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, TRUE, 0, SPIF_SENDWININICHANGE);
  POINT pt;
  GetCursorPos(&pt);
  SetCursorPos(pt.x + 5, pt.y + 5);
  SetCursorPos(pt.x, pt.y);

  if (!is_screensaver_running())
    return;

  dmsg("Kill screensaver");

  // kill any running screensaver
  // ********** THIS DOESNT WORK ***************
  HDESK hdesk = OpenDesktop(L"Screen-saver", 0, false, DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS);

  if (!hdesk)
  {
    kill_screensaver(GetForegroundWindow(), 0);
    return;
  }

  EnumDesktopWindows(hdesk, (WNDENUMPROC)kill_screensaver, 0);
  CloseDesktop(hdesk);
}

BOOL CALLBACK kill_screensaver(HWND hwnd, LPARAM lParam)
{
  char szclass[32];
  std::string tmp;
  GetClassNameA(hwnd, szclass, sizeof(szclass) - 1);
  tmp = szclass;

  if (IsWindowVisible(hwnd))// && tmp == std::string("WindowsScreenSaverClass"))
  {
    dmsg("Killing screensaver!");
    PostMessage(hwnd, WM_CLOSE, 0, 0);
  }

  return(TRUE);
}