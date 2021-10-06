#include <Windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <iostream>
#include <vector>

#include "resource.h"

#define IDM_ABOUT 1
#define IDM_BUILDER 2
#define IDM_EXPORT 3
#define IDM_SETTINGS 4
#define IDM_SSL 5
#define IDM_CONSOLE 6
#define IDM_ADSI 7

struct _WINDOW_HANDLES
{
	HWND MainWindow;
	HWND ListViewWindow;
	HWND SplitterWindow;
	HWND TabMainWindow;
	HWND ConsoleOutputPage;
	HWND ConsoleInputPage;
	HWND ConsoleLogsPage;
	HWND ClientLogsPage;
} wh;

HINSTANCE ghInst;

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SplitterProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TabMainProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleInputProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static WNDPROC SplitterWndProc;
static WNDPROC TabMainWndProc;
static WNDPROC ConsoleInputWndProc;

static std::vector<unsigned __int16> vect; //This would be used in conjunction with sockets. The vector would store the sockets you have selected.

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow)
{
	ghInst = hInst;

	HIMAGELIST MainWindowImageList;
	MainWindowImageList = ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, 1, 0);
	HICON hIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	ImageList_AddIcon(MainWindowImageList, hIcon);
	DestroyIcon(hIcon);

	WNDCLASSW wc = {};
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.hIcon = ImageList_GetIcon(MainWindowImageList, 0, ILD_NORMAL);
	wc.hInstance = hInst;
	wc.lpszClassName = L"WindowClass";
	wc.lpfnWndProc = WindowProcedure;

	if (!RegisterClassW(&wc))
	{
		return -1;
	}

	wh.MainWindow = CreateWindowExW(NULL, L"WindowClass", L"Default RAT - Connected: 2 - [Total selected: 0]", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1300, 700, NULL, NULL, hInst, NULL);
	ShowWindow(wh.MainWindow, ncmdshow);

	MSG msg;

	while (GetMessageW((&msg), NULL, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return 0;
}

void Quote(void)
{
	SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"\"If you believe the lie that becomes the truth, and if you believe the truth, you never need to lie.\" - Christian L.\r\n");
}

void GetListViewCount(void)
{
	int ItemCount = ListView_GetItemCount(wh.ListViewWindow);
	wchar_t items[sizeof(ItemCount)];
	wsprintfW(items, L"%d", ItemCount);
	wcscat_s(items, L"\r\n");
	SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)items);
	SendMessageW(wh.ConsoleLogsPage, EM_REPLACESEL, 0, (LPARAM)items);
}

void GetSelectedIDs(void)
{
	if (vect.size() == 0)
	{
		SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"No clients are selected!\r\n");
	}
	else
	{
		SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"Currently selected ID(s): ");

		for (int i = 0; i < vect.size(); i++)
		{
			wchar_t ID[sizeof(vect)];
			wsprintfW(ID, L"%d ", vect[i]);
			SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)ID);
		}
		SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"\r\n");
	}
}

void ClearSelectedIDs(void)
{
	if (vect.size() == 0)
	{
		SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"No clients were selected!\r\n");
	}
	else
	{
		wchar_t title[50];
		vect.clear();
		wsprintfW(title, L"Default RAT - Connected: 2 - [Total selected: %d]", 0);
		SetWindowTextW(wh.MainWindow, title);
		SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"All clients unselected!\r\n");
	}
}

void InitializeMainWindow(HWND hWnd)
{
	HMENU hMenu;
	RECT rcMain;
	GetClientRect(hWnd, &rcMain);

	hMenu = CreateMenu();
	AppendMenuW(hMenu, MF_STRING, IDM_ABOUT, L"About");
	AppendMenuW(hMenu, MF_STRING, IDM_BUILDER, L"Builder");
	AppendMenuW(hMenu, MF_STRING, IDM_EXPORT, L"Export");
	AppendMenuW(hMenu, MF_STRING, IDM_SETTINGS, L"Settings");
	AppendMenuW(hMenu, MF_STRING, IDM_SSL, L"SSL");
	SetMenu(hWnd, hMenu);

	wh.ListViewWindow = CreateWindowExW(WS_EX_CLIENTEDGE, WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT, 0, 0, rcMain.right, 272, hWnd, (HMENU)0, ghInst, NULL);
	ListView_SetExtendedListViewStyle(wh.ListViewWindow, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES); //If you don't want the grildlines, just remove LVS_EX_GRIDLINES

	SendMessageW(wh.ListViewWindow, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);

	HIMAGELIST hListViewImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 0);
	HICON hListViewIcon;
	hListViewIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON2));
	ImageList_AddIcon(hListViewImageList, hListViewIcon);
	DestroyIcon(hListViewIcon);

	ListView_SetImageList(wh.ListViewWindow, hListViewImageList, LVSIL_SMALL);

	LVCOLUMNW lvc = {};
	lvc.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = 0;
	lvc.cx = 55;
	lvc.pszText = (LPWSTR)L"ID"; //This represents a sockets (clients) position within a socket array.
	ListView_InsertColumn(wh.ListViewWindow, 0, &lvc);

	lvc.iSubItem = 1;
	lvc.cx = 118;
	lvc.pszText = (LPWSTR)L"Hostname";
	ListView_InsertColumn(wh.ListViewWindow, 1, &lvc);

	lvc.iSubItem = 2;
	lvc.cx = 95;
	lvc.pszText = (LPWSTR)L"IP";
	ListView_InsertColumn(wh.ListViewWindow, 2, &lvc);

	lvc.iSubItem = 3;
	lvc.cx = 256;
	lvc.pszText = (LPWSTR)L"Operating System";
	ListView_InsertColumn(wh.ListViewWindow, 3, &lvc);

	lvc.iSubItem = 4;
	lvc.cx = 150;
	lvc.pszText = (LPWSTR)L"Domain Name";
	ListView_InsertColumn(wh.ListViewWindow, 4, &lvc);

	lvc.iSubItem = 5;
	lvc.cx = 120;
	lvc.pszText = (LPWSTR)L"Group Membership";
	ListView_InsertColumn(wh.ListViewWindow, 5, &lvc);

	lvc.iSubItem = 6;
	lvc.cx = 120;
	lvc.pszText = (LPWSTR)L"Integrity Level";
	ListView_InsertColumn(wh.ListViewWindow, 6, &lvc);

	lvc.iSubItem = 7;
	lvc.pszText = (LPWSTR)L"Install Date";
	ListView_InsertColumn(wh.ListViewWindow, 7, &lvc);

	LVITEMW lvi = {};
	lvi.mask = LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.iImage = 0;
	lvi.pszText = (LPWSTR)L"1";
	ListView_InsertItem(wh.ListViewWindow, &lvi);

	ListView_SetItemText(wh.ListViewWindow, 0, 1, (LPWSTR)L"DESKTOP-IJ9073");
	ListView_SetItemText(wh.ListViewWindow, 0, 2, (LPWSTR)L"10.0.14.5");
	ListView_SetItemText(wh.ListViewWindow, 0, 3, (LPWSTR)L"Windows 10 2004 x64");
	ListView_SetItemText(wh.ListViewWindow, 0, 4, (LPWSTR)L"test.local");
	ListView_SetItemText(wh.ListViewWindow, 0, 5, (LPWSTR)L"Administrator");
	ListView_SetItemText(wh.ListViewWindow, 0, 6, (LPWSTR)L"Medium");
	ListView_SetItemText(wh.ListViewWindow, 0, 7, (LPWSTR)L"2021-08-13");

	lvi.iItem = 1;
	lvi.iImage = 0;
	lvi.iSubItem = 0;
	lvi.pszText = (LPWSTR)L"2";
	ListView_InsertItem(wh.ListViewWindow, &lvi);

	ListView_SetItemText(wh.ListViewWindow, 1, 1, (LPWSTR)L"DESKTOP-QZ2708");
	ListView_SetItemText(wh.ListViewWindow, 1, 2, (LPWSTR)L"10.0.28.19");
	ListView_SetItemText(wh.ListViewWindow, 1, 3, (LPWSTR)L"Windows 10 21H1 x64");
	ListView_SetItemText(wh.ListViewWindow, 1, 4, (LPWSTR)L"test.local");
	ListView_SetItemText(wh.ListViewWindow, 1, 5, (LPWSTR)L"User");
	ListView_SetItemText(wh.ListViewWindow, 1, 6, (LPWSTR)L"High");
	ListView_SetItemText(wh.ListViewWindow, 1, 7, (LPWSTR)L"2021-09-09");

	wh.SplitterWindow = CreateWindowExW(NULL, WC_STATIC, L"", WS_CHILD | WS_BORDER | WS_VISIBLE | SS_NOTIFY, 0, 272, rcMain.right, 10, hWnd, NULL, ghInst, NULL);
	SplitterWndProc = (WNDPROC)SetWindowLongPtr(wh.SplitterWindow, GWLP_WNDPROC, (LONG_PTR)SplitterProcedure);

	LOGFONT lf;
	GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lf);
	HFONT GeneralFont = CreateFont(
		19, 0,
		lf.lfEscapement, lf.lfOrientation, lf.lfWeight,
		lf.lfItalic, lf.lfUnderline, lf.lfStrikeOut,
		lf.lfCharSet, lf.lfOutPrecision, lf.lfClipPrecision,
		lf.lfQuality, lf.lfPitchAndFamily, NULL);

	wh.TabMainWindow = CreateWindowExW(WS_EX_CLIENTEDGE | WS_EX_COMPOSITED, WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE, rcMain.left, 283, rcMain.right, 378, hWnd, NULL, ghInst, NULL);
	TabMainWndProc = (WNDPROC)SetWindowLongPtrW(wh.TabMainWindow, GWLP_WNDPROC, (LONG_PTR)TabMainProcedure);
	SendMessageW(wh.TabMainWindow, WM_SETFONT, (WPARAM)GeneralFont, TRUE);

	HIMAGELIST hTabImageList;
	hTabImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 0);
	HICON hTabIcon;
	hTabIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON3));
	ImageList_AddIcon(hTabImageList, hTabIcon);
	hTabIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON4));
	ImageList_AddIcon(hTabImageList, hTabIcon);
	DestroyIcon(hTabIcon);

	TabCtrl_SetImageList(wh.TabMainWindow, hTabImageList);

	TCITEMW tie;
	tie.mask = TCIF_TEXT | TCIF_IMAGE;
	tie.iImage = 0;
	tie.pszText = (LPWSTR)L"Console";
	TabCtrl_InsertItem(wh.TabMainWindow, 0, &tie);

	wh.ConsoleOutputPage = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDIT, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_WANTRETURN, 0, 25, rcMain.right, 323, wh.TabMainWindow, NULL, ghInst, NULL);
	SendMessageW(wh.ConsoleOutputPage, WM_SETFONT, (WPARAM)GeneralFont, TRUE);

	wh.ConsoleInputPage = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDIT, L"", WS_CHILD | WS_VISIBLE, 0, 348, rcMain.right, 28, wh.TabMainWindow, NULL, ghInst, NULL);
	ConsoleInputWndProc = (WNDPROC)SetWindowLongPtr(wh.ConsoleInputPage, GWLP_WNDPROC, (LONG_PTR)ConsoleInputProcedure);
	SendMessageW(wh.ConsoleInputPage, WM_SETFONT, (WPARAM)GeneralFont, TRUE);

	tie.iImage = 1;
	tie.pszText = (LPWSTR)L"Console Logs";
	TabCtrl_InsertItem(wh.TabMainWindow, 1, &tie);

	wh.ConsoleLogsPage = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDIT, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_WANTRETURN, rcMain.left, 25, rcMain.right, 348, wh.TabMainWindow, NULL, ghInst, NULL);
	SendMessage(wh.ConsoleLogsPage, WM_SETFONT, (WPARAM)GeneralFont, TRUE);

	tie.iImage = 1;
	tie.pszText = (LPWSTR)L"Client Logs";
	TabCtrl_InsertItem(wh.TabMainWindow, 2, &tie);

	wh.ClientLogsPage = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDIT, L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_READONLY | ES_MULTILINE | ES_WANTRETURN, rcMain.left, 25, rcMain.right, 348, wh.TabMainWindow, NULL, ghInst, NULL);
	SendMessageW(wh.ClientLogsPage, WM_SETFONT, (WPARAM)GeneralFont, TRUE);

	LPCWSTR buff = L"<9/5/2021 - 12:35 PM> [+] Client Connected: ID 1\r\n";
	SendMessageW(wh.ClientLogsPage, EM_REPLACESEL, 0, (LPARAM)buff);

	LPCWSTR msg = L"\r\n<9/8/2021 - 11:27 AM> [-] Client Disconnected: ID 1\r\n";
	SendMessageW(wh.ClientLogsPage, EM_REPLACESEL, 0, (LPARAM)msg);

	ShowWindow(wh.ConsoleOutputPage, SW_HIDE);
	ShowWindow(wh.ConsoleInputPage, SW_HIDE);
	ShowWindow(wh.ConsoleLogsPage, SW_HIDE);
	ShowWindow(wh.ClientLogsPage, SW_HIDE);
}


LRESULT CALLBACK SplitterProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bSplitterMoving;

	RECT rcMain;
	GetClientRect(wh.MainWindow, &rcMain);

	switch (message)
	{
		case WM_SETCURSOR:
		{
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			SetWindowLongPtrW(hWnd, DWLP_MSGRESULT, TRUE);
			return TRUE;
		}

		case WM_LBUTTONDOWN:
		{
			bSplitterMoving = TRUE;
			SetCapture(wh.SplitterWindow);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			bSplitterMoving = FALSE;
			return 0;
		}

		case WM_MOUSEMOVE:
		{
			if (bSplitterMoving)
			{
				POINT  pt{ GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
				ClientToScreen(wh.SplitterWindow, &pt);
				ScreenToClient(wh.MainWindow, &pt);

				LONG newSplitterPos = pt.y;
				if (newSplitterPos < 105 || newSplitterPos > 450)
				{
					return 0;
				}

				else
				{
					HDWP hdwp = BeginDeferWindowPos(3);

					DeferWindowPos(hdwp, wh.ListViewWindow, NULL, rcMain.left, rcMain.left, rcMain.right, newSplitterPos, NULL);
					DeferWindowPos(hdwp, wh.SplitterWindow, NULL, rcMain.left, newSplitterPos, rcMain.right, 10, NULL);
					DeferWindowPos(hdwp, wh.TabMainWindow, NULL, rcMain.left, newSplitterPos + 11, rcMain.right, rcMain.bottom - (newSplitterPos + 11), NULL);
					EndDeferWindowPos(hdwp);

					MoveWindow(wh.ConsoleOutputPage, rcMain.left, 25, rcMain.right, rcMain.bottom - (newSplitterPos + 66), TRUE);
					MoveWindow(wh.ConsoleInputPage, rcMain.left, rcMain.bottom - (newSplitterPos + 41), rcMain.right, 28, TRUE);
					MoveWindow(wh.ConsoleLogsPage, rcMain.left, 25, rcMain.right, rcMain.bottom - (newSplitterPos + 41), TRUE);
					MoveWindow(wh.ClientLogsPage, rcMain.left, 25, rcMain.right, rcMain.bottom - (newSplitterPos + 41), TRUE);

					UpdateWindow(wh.ListViewWindow);
					UpdateWindow(wh.SplitterWindow);
					UpdateWindow(wh.TabMainWindow);
				}
				return 0;
			}
		}
	}
	return CallWindowProc(SplitterWndProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK TabMainProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CTLCOLORSTATIC:
		{
			HDC hdcStatic = (HDC)wParam;
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(0, 0, 0));
			return (LRESULT)GetStockObject(BLACK_BRUSH);
		}

		case WM_CTLCOLOREDIT:
		{
			HDC hdcStatic = (HDC)wParam;
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(0, 0, 0));
			return (LRESULT)GetStockObject(BLACK_BRUSH);
		}
	}

	return CallWindowProc(TabMainWndProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK ConsoleInputProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/*
	
	I am aware this is messy and defiently not the best way to do something like this. 

	*/

	switch (message)
	{
		case WM_CHAR:
		{
			switch (wParam)
			{
				case VK_RETURN:
				{
					int idx = GetWindowTextLengthW(wh.ConsoleOutputPage);
					SendMessageW(wh.ConsoleOutputPage, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);

					wchar_t buff[256]; //This will be improved in the future hopefully
					GetWindowTextW(wh.ConsoleInputPage, buff, 256); //This will be improved in the future hopefully
					wcscat_s(buff, L"\r\n");
					SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> "); //Greater-than sign indicates a command
					SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)buff);
					SetWindowTextW(wh.ConsoleInputPage, L"");
					SendMessageW(wh.ConsoleLogsPage, EM_REPLACESEL, 0, (LPARAM)L"> ");
					SendMessageW(wh.ConsoleLogsPage, EM_REPLACESEL, 0, (LPARAM)buff);

					if (_wcsicmp(buff, L"clear\r\n") == 0)
					{
						ClearSelectedIDs(); //Clears vector, thus, unselecting previosuly selected clients.
					}
					else if (_wcsicmp(buff, L"cls\r\n") == 0)
					{
						SetWindowText(wh.ConsoleOutputPage, L""); //Clears the console.
					}
					else if (_wcsicmp(buff, L"count\r\n") == 0)
					{
						GetListViewCount(); //Gets the numbers of items (clients) in the listview.
					}
					else if (_wcsicmp(buff, L"id\r\n") == 0)
					{
						GetSelectedIDs(); //Gets all the selected clients from the vector.
					}
					else if (_wcsicmp(buff, L"quote\r\n") == 0)
					{
						Quote(); //My quote. 
					}
					return 0;
				}
			}
		}
	}
	return CallWindowProc(ConsoleInputWndProc, hWnd, message, wParam, lParam);
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lp;

	RECT rcMain, rcListView;
	GetClientRect(hWnd, &rcMain);
	GetClientRect(wh.ListViewWindow, &rcListView);

	HMENU hMenu;

	switch (msg)
	{
		case WM_CLOSE:
		{
			if (MessageBoxW(hWnd, L"Are you sure you want to exit?", L"Quit Application", MB_YESNO) == IDYES)
			{
				DestroyWindow(hWnd);
			}
			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_CREATE:
		{
			InitializeMainWindow(hWnd);
			ShowWindow(wh.ConsoleOutputPage, SW_SHOW);
			ShowWindow(wh.ConsoleInputPage, SW_SHOW);
			SetFocus(wh.ConsoleInputPage);
			return 0;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wp))
			{
				/*
				
				Within each of these cases is where you would put your own function for what you would want to happen when choosing a menu item. 
				I put a comment next to each function for what I hope each case will do in the future. For now it just sends output to the console telling you which menu item you chose.

				*/

				case IDM_ABOUT:
				{
					SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> You chose About.\r\n"); //This will display about information describing the software. (developer, use case, liability, etc.)
					return 0;
				}

				case IDM_BUILDER:
				{
					SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> You chose Builder.\r\n"); //This will hopefully be used to build the client exe.
					return 0;
				}

				case IDM_EXPORT:
				{
					SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> You chose Export.\r\n"); //This will be used to export the logs.
					return 0;
				}

				case IDM_SETTINGS:
				{
					SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> You chose Settings.\r\n"); //This will be used to configure the server.
					return 0;
				}

				case IDM_SSL:
				{
					/*
					
					This will hopefully be used to create a CSR (Certifiacte Signing Request) through openssl. I am currently working on utilizing openssl's functions for both 
					client and server communications. If you want to see an SSL library being utilized take a look at CIA's Hive.

					*/

					SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> You chose SSL.\r\n");
					return 0;
				}

				case IDM_CONSOLE:
				{
					/*

					This will add the selected client(s) ID to the vector. When you send a command to a client(s), a for loop will iterate over the vector and send a command to each client.
					As stated previously the ID column in the listview would represent a sockets position within an array. Doing something like this should allow commands to be sent to multiple clients 
					all at once. I understand no network functionality is included within this project, I am just explaining how you could possibly implement something. 

					*/

					if (ListView_GetSelectedCount(wh.ListViewWindow) == 0)
					{
						SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> No item was selected! Please select an item and try again.\r\n");
						return 0;
					}
					else
					{	
						int count;
						int iPos = ListView_GetNextItem(wh.ListViewWindow, -1, LVNI_SELECTED);
						while (iPos != -1)
						{
							wchar_t pos[256] = {};
							ListView_GetItemText(wh.ListViewWindow, iPos, 0, pos, sizeof(pos));
							
							count = _wtoi(pos);

							if (std::find(vect.begin(), vect.end(), count) == vect.end())
							{
								vect.push_back(count);
							}
		
							iPos = ListView_GetNextItem(wh.ListViewWindow, iPos, LVNI_SELECTED);
						}
						
						wchar_t title[53] = {};
						wsprintfW(title, L"Default RAT - Connected: 2 - [Total selected: %I64u]", vect.size());
						SetWindowTextW(wh.MainWindow, title);
						return 0;
					}
				}

				case IDM_ADSI:
				{

				/*

				I was creating an ADSI GUI similar to Mark Russinovich's Active Directory Explorer. I did not include it because sending Active Directory information over a socket was more
				complex than I thought (you are probably better at this than me so it may not be hard for you, I'm still learning about the Win32 Active Directory\ADSI API).
				The goal for this is to be able to enumerate Active Directory within the network and send the results back.
				If this can be implemented, dropping binaries to disk like AdFind.exe can possbily be avoided (AdFind is a great tool!). If you are planning on implementing this yourself
				get ready to learn about COM and everything that it entails (Pointers, References, Interfaces, BSTR, VARIANT, SAFEARRAYS, etc).
				https://docs.microsoft.com/en-us/sysinternals/downloads/adexplorer

				*/

					if (ListView_GetSelectedCount(wh.ListViewWindow) == 0)
					{
						SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> No item was selected! Please select an item and try again.\r\n");
						return 0;
					}
					else
					{
						SendMessageW(wh.ConsoleOutputPage, EM_REPLACESEL, 0, (LPARAM)L"> You chose ADSI\r\n");

						int iPos = ListView_GetNextItem(wh.ListViewWindow, -1, LVNI_SELECTED);
						while (iPos != -1)
						{
							wchar_t hostname[256];
							ListView_GetItemText(wh.ListViewWindow, iPos, 1, hostname, 256);
							//ADSI(ghInst, hostname); This would have launched the ADSI window but I have taken it out because it was not fully finished.
							iPos = ListView_GetNextItem(wh.ListViewWindow, iPos, LVNI_SELECTED);
						}
						return 0;
					}
				}
			}
		}

		case WM_NOTIFY:
		{
			LPNMHDR tc = ((LPNMHDR)lp);
			switch (tc->code)
			{
				case NM_RCLICK:
				{
					if (tc->hwndFrom == wh.ListViewWindow)
					{
						POINT cursor;
						GetCursorPos(&cursor);

						hMenu = CreatePopupMenu();
						AppendMenuW(hMenu, MF_STRING, IDM_ADSI, (LPWSTR)L"ADSI");
						AppendMenuW(hMenu, MF_STRING, IDM_CONSOLE, (LPWSTR)L"Console");

						TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, cursor.x, cursor.y, 0, hWnd, NULL);
						DestroyMenu(hMenu);
						return TRUE;
					}
				}
				
				case TCN_SELCHANGE:
				{
					DWORD TabIndex = TabCtrl_GetCurSel(wh.TabMainWindow);
					ShowWindow(wh.ConsoleOutputPage, (TabIndex == 0) ? SW_SHOW : SW_HIDE);
					ShowWindow(wh.ConsoleInputPage, (TabIndex == 0) ? SW_SHOW : SW_HIDE);
					ShowWindow(wh.ConsoleLogsPage, (TabIndex == 1) ? SW_SHOW : SW_HIDE);
					ShowWindow(wh.ClientLogsPage, (TabIndex == 2) ? SW_SHOW : SW_HIDE);
					return 0;
				}
			}
		}

		case WM_GETMINMAXINFO:
		{
			lpMMI->ptMinTrackSize.x = 1300;
			lpMMI->ptMinTrackSize.y = 700;
			return 0;
		}

		case WM_SIZE:
		{
			POINT  pt{ GET_X_LPARAM(lp),GET_Y_LPARAM(lp) };
			ClientToScreen(wh.SplitterWindow, &pt);
			ScreenToClient(wh.MainWindow, &pt);
			LONG newSplitterPos = pt.y;

			HDWP hdwp = BeginDeferWindowPos(3);
			DeferWindowPos(hdwp, wh.ListViewWindow, NULL, rcMain.left, rcMain.left, rcMain.right, (newSplitterPos - rcMain.bottom) - 1, SWP_NOMOVE);
			DeferWindowPos(hdwp, wh.SplitterWindow, NULL, rcMain.left, (newSplitterPos - rcMain.bottom) - 1, rcMain.right, 10, SWP_NOMOVE);
			DeferWindowPos(hdwp, wh.TabMainWindow, NULL, rcMain.left, (newSplitterPos - rcMain.bottom) + 10, rcMain.right, rcMain.bottom - (rcListView.bottom + 15), SWP_NOMOVE);
			EndDeferWindowPos(hdwp);

			MoveWindow(wh.ConsoleOutputPage, rcMain.left, 25, rcMain.right, rcMain.bottom - (rcListView.bottom + 70), TRUE);
			MoveWindow(wh.ConsoleInputPage, rcMain.left, rcMain.bottom - (rcListView.bottom + 45), rcMain.right, 28, TRUE);
			MoveWindow(wh.ConsoleLogsPage, rcMain.left, 25, rcMain.right, rcMain.bottom - (rcListView.bottom + 45), TRUE);
			MoveWindow(wh.ClientLogsPage, rcMain.left, 25, rcMain.right, rcMain.bottom - (rcListView.bottom + 45), TRUE);

			ListView_SetColumnWidth(wh.ListViewWindow, 7, LVSCW_AUTOSIZE_USEHEADER);
			return 0;
		}
	}
	return DefWindowProcW(hWnd, msg, wp, lp);
}