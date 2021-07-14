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
	WNDCLASSW WndClass{}; // ������ Ŭ���� ����
	// ������Ŭ���� ���� ---------------------------------------------------------------------------------------------------
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

	RegisterClassW(&WndClass); // �޸𸮿� ������ Ŭ���� ���

	// ������ ���� ---------------------------------------------------------------------------------------------------------
	HWND hWnd = CreateWindowExW
	(
		NULL,
		L"Program", L"Program", // Ŭ���� �̸�, ���α׷� �̸�
		WS_VISIBLE | WS_SYSMENU | WS_SIZEBOX, // ����ǥ���� ����
		(int)((GetSystemMetrics(SM_CXSCREEN) - 520) * 0.5),
		(int)((GetSystemMetrics(SM_CYSCREEN) - 350) * 0.5),
		520, // ���α׷� ���α���
		350, // ���α׷� ���α���
		NULL,
		NULL,
		hInstance,
		NULL
	);
	//----------------------------------------------------------------------------------------------------------------------

	g_hWnd = hWnd;

	// �޼��� ó�� ---------------------------------------------------------------------------------------------------------
	MSG Message{}; // �޼���
	while (GetMessageW(&Message, NULL, NULL, NULL))
	{
		TranslateMessage(&Message);
		DispatchMessageW(&Message);
	}
	//----------------------------------------------------------------------------------------------------------------------

	DestroyWindow(hWnd);
	return (int)Message.wParam; // ���α׷� ����
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
		(HINSTANCE)GetWindowLongPtrW(hWnd, -6), // �θ� ������ �ڵ�κ��� �ν��Ͻ� ���
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
		(HINSTANCE)GetWindowLongPtrW(hWnd, -6), // �θ� ������ �ڵ�κ��� �ν��Ͻ� ���
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
		(HINSTANCE)GetWindowLongPtrW(hWnd, -6), // �θ� ������ �ڵ�κ��� �ν��Ͻ� ���
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
	mbstowcs_s(NULL, recv_msg, sizeof(recv_msg) / 2, recv_buf, sizeof(recv_msg) / 2); // wchar_t�� ���ڴ� 2����Ʈ�̱� ������ 2�� ������
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

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); // ���̺귯�� �ʱ�ȭ
	if (iResult != 0) { return 1; }

	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; //TCP
	hints.ai_flags = AI_PASSIVE;

	// ������ ���� �ּҿ� ��Ʈ�� Ȯ�� --------------------------------------------------------------------------------------
	iResult = getaddrinfo("192.168.0.18", "6543", &hints, &result);
	if (iResult != 0)
	{
		WSACleanup();
		return 1;
	}
	//----------------------------------------------------------------------------------------------------------------------

	// ���� ���� -----------------------------------------------------------------------------------------------------------
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	//----------------------------------------------------------------------------------------------------------------------

	// ������ ���Ͽ� ��Ʈ���ε� --------------------------------------------------------------------------------------------
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

	// ������� ���� ���·� ����� -----------------------------------------------------------------------------------------
	iResult = listen(ListenSocket, 255);
	if (iResult == SOCKET_ERROR)
	{
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	//----------------------------------------------------------------------------------------------------------------------

	//unsigned short MaxCount = 10;

	// Ŭ���̾�Ʈ ���� ���� ------------------------------------------------------------------------------------------------
	while (true)
	{
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{ break; }
		else
		{ _beginthreadex(NULL, 0, (_beginthreadex_proc_type)RecvThread, (void*)ClientSocket, 0, NULL); }
	}
	//----------------------------------------------------------------------------------------------------------------------

	// ����
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

	// Winsock �ʱ�ȭ
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

	// ���ӵɶ����� ����õ�
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
		// ������ �����
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
	GetWindowTextW(FindEditBox(hWnd, L"������̸�"), name, 64);
	char send_buf[64] = { 0, };

	wcstombs_s(NULL, send_buf, sizeof(send_buf), name, sizeof(send_buf));

	send(ServerSocket, send_buf, sizeof(send_buf), 0);

	do
	{
		memset(recv_msg, 0, 1024);
		// Receive until the peer closes the connection
		iResult = recv(ServerSocket, recv_buf, sizeof(recv_buf), 0);
		mbstowcs_s(NULL, recv_msg, sizeof(recv_msg) / 2, recv_buf, sizeof(recv_msg) / 2); // wchar_t�� ���ڴ� 2����Ʈ�̱� ������ 2�� ������
		
		if (recv_msg[0] == L'0') // 0: �Ϲ� ä�ø޼���
		{
			EraseFirstCharW(recv_msg, 1024);
			GetWindowTextW(FindEditBox(hWnd, L"ä��ĭ"), chat, 1024);
			lstrcatW(chat, recv_msg);
			lstrcatW(chat, L"\r\n");
			SetWindowTextW(FindEditBox(hWnd, L"ä��ĭ"), chat);
		}
		if (recv_msg[0] == L'1') // 1: ���������
		{
			EraseFirstCharW(recv_msg, 1024);
			GetWindowTextW(FindEditBox(hWnd, L"ä��ĭ"), chat, 1024);
			lstrcatW(chat, recv_msg);
			lstrcatW(chat, L"���� �����ϼ̽��ϴ�.\r\n");
			SetWindowTextW(FindEditBox(hWnd, L"ä��ĭ"), chat);

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
			SetWindowTextW(FindEditBox(hWnd, L"����ڸ��"), recv_msg);
		}
		if (recv_msg[0] == L'2')
		{
			EraseFirstCharW(recv_msg, 1024);
			SetWindowTextW(FindEditBox(hWnd, L"����ڸ��"), recv_msg);
		}
		if (recv_msg[0] == L'3') // 1: ���������
		{
			EraseFirstCharW(recv_msg, 1024);
			GetWindowTextW(FindEditBox(hWnd, L"ä��ĭ"), chat, 1024);
			lstrcatW(chat, recv_msg);
			lstrcatW(chat, L"���� �����ϼ̽��ϴ�.\r\n");
			SetWindowTextW(FindEditBox(hWnd, L"ä��ĭ"), chat);

			for (int i = 0; i < 10; i++)
			{
				if (ClientName[i] == 0)
				{
					memcpy(ClientName[i], recv_msg, sizeof(ClientName[i]));
					break;
				}
			}
		}
		SendMessageW(FindEditBox(hWnd, L"ä��ĭ"), EM_SETSEL, 0, -1); //Select all. 
		SendMessageW(FindEditBox(hWnd, L"ä��ĭ"), EM_SCROLLCARET, 0, 0);//Unselect and stay at the end pos


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
			GetWindowTextW(FindEditBox(g_hWnd, L"������̸�"), name, 1024);
			lstrcatW(text, name);
			memcpy(name, text, 1024);
			GetWindowTextW(FindEditBox(g_hWnd, L"�޼����Է�ĭ"), text, 1024);
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
			SetWindowTextW(FindEditBox(g_hWnd, L"�޼����Է�ĭ"), L"");
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
		RECT rect{}; // ���α׷� ����
		GetClientRect(hWnd, &rect); // ���α׷� ���� ���

		CreateButton(hWnd, L"����", (rect.right - 130), (rect.bottom - 25), 60, 20);
		CreateButton(hWnd, L"����", (rect.right - 65), (rect.bottom - 25), 60, 20);
		CreateEditBox(hWnd, L"�޼����Է�ĭ", (rect.left + 5), (rect.bottom - 25), (rect.right - 10), 20);
		CreateEditBox(hWnd, L"������̸�", (rect.right - 105), (rect.top + 5), 100, 20);
		CreateChatBox(hWnd, L"����ڸ��", (rect.right - 145), (rect.top + 5), 140, (rect.bottom - 35));
		CreateChatBox(hWnd, L"ä��ĭ", (rect.left + 5), (rect.top + 5), (rect.right - 155), (rect.bottom - 35));
		old_proc = (WNDPROC)SetWindowLongPtrW(FindEditBox(hWnd, L"�޼����Է�ĭ"), GWLP_WNDPROC, (LONG_PTR)ChatBoxProc);

		ShowWindow(FindEditBox(hWnd, L"������̸�"), SW_SHOW);
		ShowWindow(FindButton(hWnd, L"����"), SW_SHOW);
		ShowWindow(FindButton(hWnd, L"����"), SW_SHOW);

		return 0;
	}
	case WM_COMMAND:
	{
		if ((HWND)lParam == FindButton(hWnd, L"����")) // ���� ����
		{
			ShowWindow(FindButton(hWnd, L"����"), SW_HIDE);
			ShowWindow(FindButton(hWnd, L"����"), SW_HIDE);
			ShowWindow(FindEditBox(hWnd, L"������̸�"), SW_HIDE);

			SetWindowTextW(hWnd, L"Server");
			handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ServerThread, hWnd, 0, NULL);
			return 0;
		}

		if ((HWND)lParam == FindButton(hWnd, L"����")) // Ŭ���̾�Ʈ ����
		{
			wchar_t name[64] = { 0, };
			GetWindowTextW(FindEditBox(hWnd, L"������̸�"), name, 64);

			if (lstrlen(name) != 0)
			{
				ShowWindow(FindButton(hWnd, L"����"), SW_HIDE);
				ShowWindow(FindButton(hWnd, L"����"), SW_HIDE);
				ShowWindow(FindEditBox(hWnd, L"������̸�"), SW_HIDE);
				ShowWindow(FindEditBox(hWnd, L"�޼����Է�ĭ"), SW_SHOW);
				ShowWindow(FindEditBox(hWnd, L"ä��ĭ"), SW_SHOW);
				ShowWindow(FindEditBox(hWnd, L"����ڸ��"), SW_SHOW);

				SetWindowTextW(hWnd, name);
				handle = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)ClientThread, hWnd, 0, NULL);
			}
			else
			{ MessageBoxW(hWnd, L"�̸��� �Է����ּ���", L"Client", MB_OK); }

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
		RECT rect{}; // ���α׷� ����

		GetClientRect(hWnd, &rect); // ���α׷� ���� ���

		MoveWindow(FindButton(hWnd, L"����"), (rect.right - 130), (rect.bottom - 25), 60, 20, FALSE);
		MoveWindow(FindButton(hWnd, L"����"), (rect.right - 65), (rect.bottom - 25), 60, 20, FALSE);
		MoveWindow(FindEditBox(hWnd, L"ä��ĭ"), (rect.left + 5), (rect.top + 5), (rect.right - 155), (rect.bottom - 35), FALSE);
		MoveWindow(FindEditBox(hWnd, L"����ڸ��"), (rect.right - 145), (rect.top + 5), 140, (rect.bottom - 35), FALSE);
		MoveWindow(FindEditBox(hWnd, L"�޼����Է�ĭ"), (rect.left + 5), (rect.bottom - 25), (rect.right - 10), 20, FALSE);
		MoveWindow(FindEditBox(hWnd, L"������̸�"), (rect.right - 105), (rect.top + 5), 100, 20, FALSE);
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
		PAINTSTRUCT PS{}; // �ٽñ׸��� ȭ���� ����
		BeginPaint(hWnd, &PS); // �׸��� ����

		HPEN hPen = (HPEN)GetStockObject(DC_PEN);
		HBRUSH hBrush = (HBRUSH)GetStockObject(DC_BRUSH);

		SetDCPenColor(PS.hdc, RGB(50, 50, 50));
		SetDCBrushColor(PS.hdc, RGB(50, 50, 50));

		SelectObject(PS.hdc, hPen);
		SelectObject(PS.hdc, hBrush);

		Rectangle(PS.hdc, PS.rcPaint.left, PS.rcPaint.top, PS.rcPaint.right, PS.rcPaint.bottom);

		EndPaint(hWnd, &PS); // �׸��� ��
		return 0;
	}
	case WM_CLOSE:
		if (handle != NULL)
		{ CloseHandle(handle); }
		DestroyWindow(hWnd);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0); // WM_QUIT �޼��� ȣ��
		return 0;
	default:
		break;
	}
	return DefWindowProcW(hWnd, Wms, wParam, lParam);
}