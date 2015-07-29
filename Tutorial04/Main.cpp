#include <Windows.h>
#include <d3d10.h>
#include "resource.h"
#include <D3DX10.h>


ID3D10Device* gpDevice;
HWND hWnd;
IDXGISwapChain* gpSwapChain;
ID3D10RenderTargetView* gpRenderTargetView;

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

	hWnd = CreateWindow(L"DirectX10Tutorials:Texture", L"Texture", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top
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

	HRESULT hr = S_OK;

	RECT rc;

	GetWindowRect(hWnd, &rc);

	UINT height =  rc.bottom - rc.top;
	UINT width = rc.right - rc.left;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	swapChainDesc.Windowed = true;

	UINT createFlag;

#ifdef _DEBUG
	createFlag |= D3D10_CREATE_DEVICE_DEBUG;
#endif


	D3D10_DRIVER_TYPE driverTypes[] = {

		D3D10_DRIVER_TYPE_HARDWARE,
		D3D10_DRIVER_TYPE_REFERENCE,
	};

	UINT numType = sizeof(driverTypes) / sizeof(driverTypes[0]);

	for (int i = 0; i < numType;i++)
	{
		
		hr = D3D10CreateDeviceAndSwapChain(NULL, driverTypes[i], NULL, createFlag, D3D10_SDK_VERSION, &swapChainDesc, &gpSwapChain, &gpDevice);

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	ID3D10Texture2D* pBackTexture2D;

	gpSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackTexture2D);

	if (FAILED(hr))
		return hr;

	gpDevice->CreateRenderTargetView(pBackTexture2D, NULL, &gpRenderTargetView);
	pBackTexture2D->Release();

	gpDevice->OMSetRenderTargets(1, &gpRenderTargetView, NULL);

	D3D10_VIEWPORT viewport;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	gpDevice->RSSetViewports(1, &viewport);


	return hr;
}

void CleanDevice()
{
	
	if (gpDevice)
		gpDevice->ClearState();
	if (gpRenderTargetView)
		gpRenderTargetView->Release();
	if (gpSwapChain)
		gpSwapChain->Release();
	if (gpDevice)
		gpDevice->Release();

}

void Render()
{
	
}