#include <d3d10.h>
#include <D3DX10.h>
#include <Windows.h>
#include "resource.h"

HRESULT InitWindows(HINSTANCE hInstance,int nCmdShow);
HRESULT InitDevice();
void CleanDevice();
void Render();

LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hinstance,HINSTANCE hPreInstance,LPWSTR lpCmdLine,int nCmdShow)
{
	
	UNREFERENCED_PARAMETER(hPreInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindows(hinstance,nCmdShow)))
	{
		return 0;
	}

	if (FAILED(InitDevice()))
	{
		CleanDevice();
		return 0;
	}

	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{

		if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	CleanDevice();
	return (int)msg.wParam;
}

HRESULT InitWindows(HINSTANCE hInstance,int nCmdShow)
{
	WNDCLASSEX wcex;
	wcex.hInstance = hInstance;
	wcex.cbClsExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbWndExtra = 0;
	wcex.hCursor = LoadCursor(hInstance, (LPCWSTR)IDC_ARROW);
	wcex.hIcon = LoadIcon(hInstance, (LPCWSTR)IDI_APP_ICON);
	wcex.hIconSm = LoadIcon(hInstance, (LPCWSTR)IDI_APP_ICON);
	wcex.lpfnWndProc = WinProc;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"DirectX10Tutorials:Texture";
	wcex.lpszMenuName = NULL;
	wcex.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	RECT rect = { 0, 0, 640, 480 };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindow(L"DirectX10Tutorials:Texture", L"Texture", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top
		, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return E_FAIL;

	ShowWindow(hWnd, nCmdShow);

	return S_OK;
}

LRESULT CALLBACK WinProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT ps;

	switch (message)
	{
	case WM_PAINT:
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;

}

HRESULT InitDevice()
{
	return S_OK;
}

void CleanDevice()
{
	
}

void Render()
{
	
}