/*********************************************************************************
** Auth : song
** Date : $time$
** Desc : 初始化Direct10
**		  1. 填充一个DXGI_SWAP_CHAIN_DESC 结构体，该结构体描述了所要创建的交换链的特性。
**		2. 使用 D3D10CreateDeviceAndSwapChain 函数创建ID3D10Device 接口和IDXGISwapChain接口。
**		3. 为交换链的后台缓冲区创建一个渲染目标视图。
**		4. 创建深度/模板缓冲区以及相关的深度/模板视图。
**		5. 将渲染目标视图和深度/模板视图绑定到渲染管线的输出合并阶段，使它们可以被Direct3D使用。
**		6. 设置视口。
** Ver : v1.0.0
**
*********************************************************************************/

#include <Windows.h>
#include "resource.h"
#include <d3d10.h>
#include <D3DX10.h>


HINSTANCE g_hInstance;
HWND g_hWnd;
IDXGISwapChain* g_SwapChain;
ID3D10Device* g_D3d10Device;
ID3D10RenderTargetView* g_RenderTargetView;
D3D10_DRIVER_TYPE g_driverType;

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HRESULT InitWindows(HINSTANCE hInstance,int nCmdShow);

HRESULT InitDevice();

void CleanupDevice();

void Render();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	
	UNREFERENCED_PARAMETER(hPreInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindows(hInstance,nCmdShow)))
	{
		return 0;
	}

	if (FAILED(InitDevice()))
	{
		CleanupDevice();
		return 0;
	}

	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Render();
		}
	}

	CleanupDevice();

	return (int)msg.wParam;
}

HRESULT InitWindows(HINSTANCE hInstance,int nCmdShow)
{
	
	WNDCLASSEX wcex;

	wcex.hInstance = hInstance;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIconSm = LoadIcon(hInstance, (LPCWSTR)IDI_WNDICON);
	wcex.hbrBackground =(HBRUSH)( COLOR_WINDOW + 1);
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.lpszMenuName = NULL;
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WinProc;
	wcex.hIcon = LoadIcon(hInstance, (LPCWSTR)IDI_WNDICON);

	if (!RegisterClassEx(&wcex))
	{
		return E_FAIL;
	}

	RECT rc = { 0, 0, 640, 480 };

	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_hInstance = hInstance;

	g_hWnd = CreateWindow(L"TutorialWindowClass", L"DirectX10 turorial01:Init directX10", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!g_hWnd)
	{
		return E_FAIL;
	}

	ShowWindow(g_hWnd, nCmdShow);

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
	GetClientRect(g_hWnd, &rc);

	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// 1.填充交换链描述
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));

	sd.BufferCount = 1;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;
	//sd.Flags = 0;
	//sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	D3D10_DRIVER_TYPE driverTypes[]
	{
		D3D10_DRIVER_TYPE_HARDWARE,
		D3D10_DRIVER_TYPE_REFERENCE
	};

	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]);

	//2.创建设备和交换链
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];
		hr = D3D10CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags,
			D3D10_SDK_VERSION, &sd, &g_SwapChain, &g_D3d10Device);

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	ID3D10Texture2D* pBackBuffer;

	hr = g_SwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&pBackBuffer));

	if (FAILED(hr))
		return hr;
	//3.创建渲染目标
	hr = g_D3d10Device->CreateRenderTargetView(pBackBuffer, 0, &g_RenderTargetView);

	if (FAILED(hr))
		return hr;

	//4.填充深度模板描述
	D3D10_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.Height = height;
	depthStencilDesc.Width = width;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Usage = D3D10_USAGE_DEFAULT;
	depthStencilDesc.MiscFlags = 0;

	ID3D10Texture2D* pDepthStencilBuffer;
	ID3D10DepthStencilView* pDepthStencilView;

	hr = g_D3d10Device->CreateTexture2D(&depthStencilDesc, 0, &pDepthStencilBuffer);
	if (FAILED(hr))
		return hr;

	//5创建深度模板缓冲区视图
	hr = g_D3d10Device->CreateDepthStencilView(pDepthStencilBuffer, 0, &pDepthStencilView);
	if (FAILED(hr))
		return hr;

	//6.绑定深度模板视图到渲染目标
	g_D3d10Device->OMSetRenderTargets(1, &g_RenderTargetView, pDepthStencilView);

	D3D10_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	//7.设置视口
	g_D3d10Device->RSSetViewports(1, &vp);


	return S_OK;
}

void CleanupDevice()
{
	
	if (g_D3d10Device)
		g_D3d10Device->ClearState();

	if (g_RenderTargetView)
		g_RenderTargetView->Release();

	if (g_SwapChain)
		g_SwapChain->Release();

	if (g_D3d10Device)
		g_D3d10Device->Release();

}

void Render()
{
	float clearColor[4] = { 0.5f, 0.5f, 0.5f, 0.5f };
	g_D3d10Device->ClearRenderTargetView(g_RenderTargetView,clearColor);
	g_SwapChain->Present(0, 0);
}