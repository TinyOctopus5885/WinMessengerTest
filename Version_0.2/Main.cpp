#include <process.h>
#include <locale.h>
#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <ws2tcpip.h>
#include <Windows.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT Wms, WPARAM wParam, LPARAM lParam);

SOCKET ClientSockets[10] = { 0, };
wchar_t ClientName[10][64] = { 0, };

HWND g_hWnd;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpszCmdParam, _In_ int nCmdShow)
{
	setlocale(LC_ALL, "");
	WNDCLASSW WndClass{}; // 윈도우 클래스 생성
	// 윈도우클래스 설정 ---------------------------------------------------------------------------------------------------
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.hbrBackground = (HBRUSH)(BLACK_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.lpszClassName = L"Program";
	WndClass.lpszMenuName = NULL;
	//----------------------------------------------------------------------------------------------------------------------

	RegisterClassW(&WndClass); // 메모리에 윈도우 클래스 등록

	// 윈도우 생성 ---------------------------------------------------------------------------------------------------------
	HWND hWnd = CreateWindowExW
	(
		NULL,
		L"Program", L"Program", // 클래스 이름, 프로그램 이름
		WS_VISIBLE | WS_SYSMENU | WS_SIZEBOX, // 제목표시줄 형태
		(int)((GetSystemMetrics(SM_CXSCREEN) - 520) * 0.5),
		(int)((GetSystemMetrics(SM_CYSCREEN) - 350) * 0.5),
		520, // 프로그램 가로길이
		350, // 프로그램 세로길이
		NULL,
		NULL,
		hInstance,
		NULL
	);
	//----------------------------------------------------------------------------------------------------------------------

	g_hWnd = hWnd;

	// 메세지 처리 ---------------------------------------------------------------------------------------------------------
	MSG Message{}; // 메세지
	while (GetMessageW(&Message, NULL, NULL, NULL))
	{
		TranslateMessage(&Message);
		DispatchMessageW(&Message);
	}
	//----------------------------------------------------------------------------------------------------------------------

	DestroyWindow(hWnd);
	return (int)Message.wParam; // 프로그램 종료
}

void EraseFirstCharW(wchar_t* Wchar, int MaxCount)
{
	for (int i = 0; i < MaxCount - 1; i++)
	{
		Wchar[i] = Wchar[i + 1];
	}
	Wchar[MaxCount - 1] = 0;
	return;
}

HWND CreateChatBox(HWND hWnd, LPCWSTR text, int x, int y, int width, int height)
{
	HWND TempHwnd = CreateWindowExW
	(
		NULL,
		L"edit",
		text,
		WS_CHILD |ES_LEFT | ES_MULTILINE| ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
		x,
		y,
		width, height,
		hWnd,
		(HMENU)-1,
		(HINSTANCE)GetWindowLongPtrW(hWnd, -6), // 부모 윈도우 핸들로부터 인스턴스 얻기
		NULL
	);
	SetWindowTextW(TempHwnd, L"");
	return TempHwnd;
}

HWND CreateEditBox(HWND hWnd, LPCWSTR text, int x, int y, int width, int height)
{
	HWND TempHwnd = CreateWindowExW
	(
		NULL,
		L"edit",
		text,
		WS_CHILD | ES_LEFT,
		x,
		y,
		width, height,
		hWnd,
		(HMENU)-1,
		(HINSTANCE)GetWindowLongPtrW(hWnd, -6), // 부모 윈도우 핸들로부터 인스턴스 얻기
		NULL
	);
	SetWindowTextW (TempHwnd, L"");
	return TempHwnd;
}

HWND CreateButton(HWND hWnd, LPCWSTR text ,int x, int y, int width, int height)
{
	return CreateWindowExW
	(
		NULL,
		L"button",
		text,
		WS_CHILD | BS_OWNERDRAW,
		x,
		y,
		width, height,
		hWnd,
		(HMENU)-1,
		(HINSTANCE)GetWindowLongPtrW(hWnd, -6), // 부모 윈도우 핸들로부터 인스턴스 얻기
		NULL
	);
}

HWND FindEditBox(HWND hWnd, LPCWSTR name) { return FindWindowExW(hWnd, NULL, L"edit", name); }

HWND FindButton(HWND hWnd, LPCWSTR name) { return FindWindowExW(hWnd, NULL, L"button", name); }

void SendW(wchar_t* msg, int msg_num)
{
	wchar_t temp_buf[1024] = { 0, };
	wchar_t temp_msg[1024] = { 0, };
	memcpy(temp_msg, msg, 1024);
	swprintf_s(temp_buf, 1024, L"%d%s" ,msg_num, temp_msg);

	char send_buf[1024] = { 0, };
	wcstombs_s(NULL, send_buf, sizeof(send_buf), temp_buf, 1024);

	for (int i = 0; i < 10; i++)
	{
		if (ClientSockets[i] != 0)
		{
			send(ClientSockets[i], send_buf, 1024, 0);
		}
	}

	return;
}

DWORD WINAPI RecvThread(LPVOID lpParameter)
{
	SOCKET ClientSocket = (SOCKET)lpParameter;

	int ClientNum = 1;

	for (int i = 0; i < 10; i++)
	{
		if (ClientSockets[i] == 0)
		{
			ClientSockets[i] = ClientSocket;
			ClientNum = i + 1;
			break;
		}
	}

	char recv_buf[1024] = { 0, };
	wchar_t recv_msg[1024] = { 0, };
	char send_buf[1024] = { 0, };

	int iResult = 1;

	iResult = recv(ClientSocket, recv_buf, sizeof(recv_buf), 0);
	mbstowcs_s(NULL, recv_msg, sizeof(recv_msg) / 2, recv_buf, sizeof(recv_msg) / 2); // wchar_t는 글자당 2바이트이기 때문에 2로 나눈다
	memcpy(ClientName[ClientNum - 1], recv_msg, sizeof(ClientName[ClientNum - 1]));

	SendW(recv_msg, 1);

	wchar_t text[1024] = { 0, };

	for (int i = 0; i < 10; i++)
	{
		if (lstrlenW(ClientName[i]) != 0)
		{
			lstrcatW(text, ClientName[i]);
			lstrcatW(text, L"\r\n");
		}
	}

	SendW(text, 2);

	do
	{
		iResult = recv(ClientSocket, recv_buf, sizeof(recv_buf), 0);
		memcpy(send_buf, recv_buf, sizeof(send_buf));
		if (iResult > 0)
		{
			for (int i = 0; i < 10; i++)
			{
				if (ClientSockets[i] != 0)
				{
					send(ClientSockets[i], send_buf, sizeof(send_buf), 0);
				}
			}
		}
		else if(iResult == 0)
		{ break; }

	} while (iResult > 0);

	memset(text, 0, 1024);

	memcpy(text, ClientName[ClientNum - 1], 64);

	SendW(text, 3);

	ClientSockets[ClientNum - 1] = 0;
	memset(ClientName[ClientNum - 1], 0, 64);

	memset(text, 0, 1024);

	for (int i = 0; i < 10; i++)
	{
		if (lstrlenW(ClientName[i]) != 0)
		{
			lstrcatW(text, ClientName[i]);
			lstrcatW(text, L"\r\n");
		}
	}

	SendW(text, 2);

	closesocket(ClientSocket);
	return 0;
}

DWORD WINAPI ServerThread(LPVOID lpParameter)
{
	HWND hWnd = (HWND)lpParameter;
	wchar_t ProgramName[32] = { 0, };
	GetWindowTextW(hWnd, ProgramName, 32);

	WSADATA wsaData{};
	int iResult;
	SOCKET ListenSocket{};
	SOCKET ClientSocket{};
	addrinfo hints{};
	addrinfo* result{};

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // 라이브러리 초기화
	if (iResult != 0) { return 1; }

	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; //TCP
	hints.ai_flags = AI_PASSIVE;

	// 생성할 서버 주소와 포트를 확인 --------------------------------------------------------------------------------------
	iResult = getaddrinfo("192.168.0.18", "6543", &hints, &result);
	if (iResult != 0)
	{
		WSACleanup();
		return 1;
	}
	//----------------------------------------------------------------------------------------------------------------------

	// 소켓 생성 -----------------------------------------------------------------------------------------------------------
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	//----------------------------------------------------------------------------------------------------------------------

	// 리스닝 소켓에 포트바인딩 --------------------------------------------------------------------------------------------
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);
	//----------------------------------------------------------------------------------------------------------------------

	// 연결수락 가능 상태로 만들기 -----------------------------------------------------------------------------------------
	iResult = listen(ListenSocket, 255);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//----------------------------------------------------------------------------------------------------------------------

	//unsigned short MaxCount = 10;

	// 클라이언트 연결 수락 ------------------------------------------------------------------------------------------------
	while (true)
	{
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{ break; }
		else
		{ _beginthreadex(NULL, 0, (_beginthreadex_proc_type)RecvThread, (void*)ClientSocket, 0, NULL); }
	}
	//----------------------------------------------------------------------------------------------------------------------

	// 종료
	closesocket(ListenSocket);
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}

SOCKET ServerSocket = INVALID_SOCKET;

DWORD WINAPI ClientThread(LPVOID lpParameter)
{
	HWND hWnd = (HWND)lpParameter;
	wchar_t ProgramName[32] = { 0, };
	GetWindowTextW(hWnd, ProgramName, 32);

	WSADATA wsaData{};
	addrinfo* result = NULL;
	addrinfo* ptr = NULL;
	addrinfo hints{};
	int iResult;

	// Winsock 초기화
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		PostMessageW(hWnd, WM_CLOSE, NULL, NULL);
		return 1;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo((PCSTR)"211.211.66.148", "6543", &hints, &result);
	if (iResult != 0)
	{
		WSACleanup();
		PostMessageW(hWnd, WM_CLOSE, NULL, NULL);

		return 1;
	}

	// 접속될때까지 연결시도
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connecting to server
		ServerSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (ServerSocket == INVALID_SOCKET)
		{
			WSACleanup();
			PostMessageW(hWnd, WM_CLOSE, NULL, NULL);
			return 1;
		}
		// 서버에 연결됨
		iResult = connect(ServerSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			closesocket(ServerSocket); ServerSocket = INVALID_SOCKET; return 1;
		}
	}

	freeaddrinfo(result);

	if (ServerSocket == INVALID_SOCKET)
	{
		WSACleanup();
		PostMessageW(hWnd, WM_CLOSE, NULL, NULL);
		return 1;
	}

	wchar_t chat[1024] = { 0, };
	wchar_t recv_msg[1024] = { 0, };
	char recv_buf[1024] = { 0, };

	wchar_t name[64] = L"";
	GetWindowTextW(FindEditBox(hWnd, L"사용자이름"), name, 64);
	char send_buf[64] = { 0, };

	wcstombs_s(NULL, send_buf, sizeof(send_buf), name, sizeof(send_buf));

	send(ServerSocket, send_buf, sizeof(send_buf), 0);

	do
	{
		memset(recv_msg, 0, 1024);
		// Receive until the peer closes the connection
		iResult = recv(ServerSocket, recv_buf, sizeof(recv_buf), 0);
		mbstowcs_s(NULL, recv_msg, sizeof(recv_msg) / 2, recv_buf, sizeof(recv_msg) / 2); // wchar_t는 글자당 2바이트이기 때문에 2로 나눈다
		
		if (recv_msg[0] == L'0') // 0: 일반 채팅메세지
		{
			EraseFirstCharW(recv_msg, 1024);
			GetWindowTextW(FindEditBox(hWnd, L"채팅칸"), chat, 1024);
			lstrcatW(chat, recv_msg);
			lstrcatW(chat, L"\r\n");
			SetWindowTextW(FindEditBox(hWnd, L"채팅칸"), chat);
		}
		if (recv_msg[0] == L'1') // 1: 사용자입장
		{
			EraseFirstCharW(recv_msg, 1024);
			GetWindowTextW(FindEditBox(hWnd, L"채팅칸"), chat, 1024);
			lstrcatW(chat, recv_msg);
			lstrcatW(chat, L"님이 입장하셨습니다.\r\n");
			SetWindowTextW(FindEditBox(hWnd, L"채팅칸"), chat);

			for (int i = 0; i < 10; i++)
			{
				if (ClientName[i] == 0)
				{
					memcpy(ClientName[i], recv_msg, sizeof(ClientName[i]));
					break;
				}
			}

			for (int i = 0; i < 10; i++)
			{
				if (lstrlenW(ClientName[i]) != 0)
				{
					lstrcatW(recv_msg, ClientName[i]);
					lstrcatW(recv_msg, L"\r\n");
				}
			}
			SetWindowTextW(FindEditBox(hWnd, L"사용자목록"), recv_msg);
		}
		if (recv_msg[0] == L'2')
		{
			EraseFirstCharW(recv_msg, 1024);
			SetWindowTextW(FindEditBox(hWnd, L"사용자목록"), recv_msg);
		}
		if (recv_msg[0] == L'3') // 1: 사용자입장
		{
			EraseFirstCharW(recv_msg, 1024);
			GetWindowTextW(FindEditBox(hWnd, L"채팅칸"), chat, 1024);
			lstrcatW(chat, recv_msg);
			lstrcatW(chat, L"님이 퇴장하셨습니다.\r\n");
			SetWindowTextW(FindEditBox(hWnd, L"채팅칸"), chat);

			for (int i = 0; i < 10; i++)
			{
				if (ClientName[i] == 0)
				{
					memcpy(ClientName[i], recv_msg, sizeof(ClientName[i]));
					break;
				}
			}
		}
		SendMessageW(FindEditBox(hWnd, L"채팅칸"), EM_SETSEL, 0, -1); //Select all. 
		SendMessageW(FindEditBox(hWnd, L"채팅칸"), EM_SCROLLCARET, 0, 0);//Unselect and stay at the end pos


	} while (iResult > 0);

	// cleanup
	closesocket(ServerSocket);
	WSACleanup();

	PostMessageW(hWnd, WM_CLOSE, NULL, NULL);
	return 0;
}

WNDPROC old_proc;

LRESULT CALLBACK ChatBoxProc(HWND hWnd, UINT Wms, WPARAM wParam, LPARAM lParam)
{
	wchar_t text[1024] = L"0";
	wchar_t name[1024] = { 0, };
	char send_buf[1024] = { 0, };
	switch (Wms)
	{
	case WM_CHAR:
		switch (wParam)
		{
		case VK_RETURN:
			GetWindowTextW(FindEditBox(g_hWnd, L"사용자이름"), name, 1024);
			lstrcatW(text, name);
			memcpy(name, text, 1024);
			GetWindowTextW(FindEditBox(g_hWnd, L"메세지입력칸"), text, 1024);
			if (lstrlen(text) == 0)
			{ return 0; }
			if (ServerSocket != INVALID_SOCKET)
			{
				lstrcatW(name, L": ");
				lstrcatW(name, text);
				memcpy(text, name, 1024);
				wcstombs_s(NULL, send_buf, sizeof(send_buf), text, sizeof(send_buf));
				send(ServerSocket, send_buf, sizeof(send_buf), 0);
			}
			SetWindowTextW(FindEditBox(g_hWnd, L"메세지입력칸"), L"");
			return 0;
		}
	}
	return CallWindowProcW(old_proc, hWnd, Wms, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT Wms, WPARAM wParam, LPARAM lParam)
{
	HANDLE handle = NULL;

	switch (Wms)
	{
	case WM_CREATE:
	{
		RECT rect{}; // 프로그램 정보
		GetClientRect(hWnd, &rect); // 프로그램 정보 얻기

		CreateButton(hWnd, L"생성", (rect.right - 130), (rect.bottom - 25), 60, 20);
		CreateButton(hWnd, L"입장", (rect.right - 65), (rect.bottom - 25), 60, 20);
		CreateEditBox(hWnd, L"메세지입력칸", (rect.left + 5), (rect.bottom - 25), (rect.right - 10), 20);
		CreateEditBox(hWnd, L"사용자이름", (rect.right - 105), (rect.top + 5), 100, 20);
		CreateChatBox(hWnd, L"사용자목록", (rect.right - 145), (rect.top + 5), 140, (rect.bottom - 35));
		CreateChatBox(hWnd, L"채팅칸", (rect.left + 5), (rect.top + 5), (rect.right - 155), (rect.bottom - 35));
		old_proc = (WNDPROC)SetWindowLongPtrW(FindEditBox(hWnd, L"메세지입력칸"), GWLP_WNDPROC, (LONG_PTR)ChatBoxProc);

		ShowWindow(FindEditBox(hWnd, L"사용자이름"), SW_SHOW);
		ShowWindow(FindButton(hWnd, L"생성"), SW_SHOW);
		ShowWindow(FindButton(hWnd, L"입장"), SW_SHOW);

		return 0;
	}
	case WM_COMMAND:
	{
		if ((HWND)lParam == FindButton(hWnd, L"생성")) // 서버 시작
		{
			ShowWindow(FindButton(hWnd, L"생성"), SW_HIDE);
			ShowWindow(FindButton(hWnd, L"입장"), SW_HIDE);
			ShowWindow(FindEditBox(hWnd, L"사용자이름"), SW_HIDE);

			SetWindowTextW(hWnd, L"Server");
			handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ServerThread, hWnd, 0, NULL);
			return 0;
		}

		if ((HWND)lParam == FindButton(hWnd, L"입장")) // 클라이언트 시작
		{
			wchar_t name[64] = { 0, };
			GetWindowTextW(FindEditBox(hWnd, L"사용자이름"), name, 64);

			if (lstrlen(name) != 0)
			{
				ShowWindow(FindButton(hWnd, L"생성"), SW_HIDE);
				ShowWindow(FindButton(hWnd, L"입장"), SW_HIDE);
				ShowWindow(FindEditBox(hWnd, L"사용자이름"), SW_HIDE);
				ShowWindow(FindEditBox(hWnd, L"메세지입력칸"), SW_SHOW);
				ShowWindow(FindEditBox(hWnd, L"채팅칸"), SW_SHOW);
				ShowWindow(FindEditBox(hWnd, L"사용자목록"), SW_SHOW);

				SetWindowTextW(hWnd, name);
				handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ClientThread, hWnd, 0, NULL);
			}
			else
			{ MessageBoxW(hWnd, L"이름을 입력해주세요", L"Client", MB_OK); }

			return 0;
		}
		return 0;
	}
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 520;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 350;
		return 0;
	case WM_SIZE:
	{
		RECT rect{}; // 프로그램 정보

		GetClientRect(hWnd, &rect); // 프로그램 정보 얻기

		MoveWindow(FindButton(hWnd, L"생성"), (rect.right - 130), (rect.bottom - 25), 60, 20, FALSE);
		MoveWindow(FindButton(hWnd, L"입장"), (rect.right - 65), (rect.bottom - 25), 60, 20, FALSE);
		MoveWindow(FindEditBox(hWnd, L"채팅칸"), (rect.left + 5), (rect.top + 5), (rect.right - 155), (rect.bottom - 35), FALSE);
		MoveWindow(FindEditBox(hWnd, L"사용자목록"), (rect.right - 145), (rect.top + 5), 140, (rect.bottom - 35), FALSE);
		MoveWindow(FindEditBox(hWnd, L"메세지입력칸"), (rect.left + 5), (rect.bottom - 25), (rect.right - 10), 20, FALSE);
		MoveWindow(FindEditBox(hWnd, L"사용자이름"), (rect.right - 105), (rect.top + 5), 100, 20, FALSE);
		PostMessageW((HWND)hWnd, WM_PAINT, NULL, NULL);
		return 0;
	}
	case WM_CTLCOLOREDIT:
	{
		HDC hdc = (HDC)wParam;
		HBRUSH hBrush = (HBRUSH)GetStockObject(DC_BRUSH);
		SetDCBrushColor(hdc, RGB(100, 100, 100));
		SetBkColor(hdc, RGB(100, 100, 100));
		SetTextColor(hdc, RGB(255, 255, 255));
		return (LRESULT)((HBRUSH)GetStockObject(DC_BRUSH));
	}
	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = (HDC)wParam;
		HBRUSH hBrush = (HBRUSH)GetStockObject(DC_BRUSH);
		SetDCBrushColor(hdc, RGB(100, 100, 100));
		SetBkColor(hdc, RGB(100, 100, 100));
		SetTextColor(hdc, RGB(255, 255, 255));
		return (LRESULT)((HBRUSH)GetStockObject(DC_BRUSH));
	}
	case WM_DRAWITEM:
	{
		WCHAR text[64];
		LPDRAWITEMSTRUCT DrawStruct = (LPDRAWITEMSTRUCT)(lParam);

		HPEN hPen = (HPEN)GetStockObject(DC_PEN);
		HBRUSH hBrush = (HBRUSH)GetStockObject(DC_BRUSH);

		SetDCPenColor(DrawStruct->hDC, RGB(50, 50, 50));
		SetDCBrushColor(DrawStruct->hDC, RGB(50, 50, 50));
		SelectObject(DrawStruct->hDC, hPen);
		SelectObject(DrawStruct->hDC, hBrush);

		Rectangle
		(
			DrawStruct->hDC,
			DrawStruct->rcItem.left,
			DrawStruct->rcItem.top,
			DrawStruct->rcItem.right,
			DrawStruct->rcItem.bottom
		);

		if (DrawStruct->itemState & ODS_SELECTED)
		{
			SetDCPenColor(DrawStruct->hDC, RGB(75, 75, 75));
			SetDCBrushColor(DrawStruct->hDC, RGB(75, 75, 75));
			SetTextColor(DrawStruct->hDC, RGB(200, 200, 200));
		}
		else
		{
			SetDCPenColor(DrawStruct->hDC, RGB(100, 100, 100));
			SetDCBrushColor(DrawStruct->hDC, RGB(100, 100, 100));
			SetTextColor(DrawStruct->hDC, RGB(255, 255, 255));
		}

		SelectObject(DrawStruct->hDC, hPen);
		SelectObject(DrawStruct->hDC, hBrush);

		RoundRect
		(
			DrawStruct->hDC,
			DrawStruct->rcItem.left,
			DrawStruct->rcItem.top,
			DrawStruct->rcItem.right,
			DrawStruct->rcItem.bottom,
			20, 20
		);

		SetBkMode(DrawStruct->hDC, TRANSPARENT);
		GetWindowTextW(DrawStruct->hwndItem, text, 64);
		DrawTextW(DrawStruct->hDC, text, (int)wcslen(text), &DrawStruct->rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}
	case WM_PAINT:
	{
		PAINTSTRUCT PS{}; // 다시그리는 화면의 정보
		BeginPaint(hWnd, &PS); // 그리기 시작

		HPEN hPen = (HPEN)GetStockObject(DC_PEN);
		HBRUSH hBrush = (HBRUSH)GetStockObject(DC_BRUSH);

		SetDCPenColor(PS.hdc, RGB(50, 50, 50));
		SetDCBrushColor(PS.hdc, RGB(50, 50, 50));

		SelectObject(PS.hdc, hPen);
		SelectObject(PS.hdc, hBrush);

		Rectangle(PS.hdc, PS.rcPaint.left, PS.rcPaint.top, PS.rcPaint.right, PS.rcPaint.bottom);

		EndPaint(hWnd, &PS); // 그리기 끝
		return 0;
	}
	case WM_CLOSE:
		if (handle != NULL)
		{ CloseHandle(handle); }
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0); // WM_QUIT 메세지 호출
		return 0;
	default:
		break;
	}
	return DefWindowProcW(hWnd, Wms, wParam, lParam);
}