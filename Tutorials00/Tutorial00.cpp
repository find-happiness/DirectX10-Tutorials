/*********************************************************************************
** Auth : song
** Date : 2015/7/8
** Desc : ³õÊ¼»¯´°¿Ú
** Ver  : v1.0.0
**
*********************************************************************************/

#include <Windows.h>
#include "resource.h"

HINSTANCE g_hinstance = NULL;

HWND g_hwnd = NULL;

LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
HRESULT InitWindows(HINSTANCE hinstance, int nCmdShow);

int WINAPI wWinMain(HINSTANCE hinstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindows(hinstance,nCmdShow)))
	{
		return 0;
	}

	MSG msg = { 0 };

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
	
}

HRESULT InitWindows(HINSTANCE hInstance,int nCmdShow)
{
	
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInstance,(LPWSTR) IDI_WNDICON);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hIconSm = LoadIcon(hInstance, (LPWSTR)IDI_WNDICON);
	wcex.lpfnWndProc = WndProc;
	wcex.lpszClassName = L"DirectX10Turorials";
	wcex.lpszMenuName = NULL;
	wcex.hInstance = hInstance;

	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	g_hinstance = hInstance;

	RECT rect = { 0, 0, 640, 480 };

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	g_hwnd = CreateWindow(L"DirectX10Turorials", L"DirectX10 Turorial 00", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);

	if (!g_hwnd)
		return E_FAIL;

	ShowWindow(g_hwnd, nCmdShow);

	return S_OK;

}

LRESULT CALLBACK WndProc(HWND hWnd,UINT message,WPARAM wparam,LPARAM lParam)
{
	
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default :
			return DefWindowProc(hWnd, message, wparam, lParam);
	}

	return 0;

}







