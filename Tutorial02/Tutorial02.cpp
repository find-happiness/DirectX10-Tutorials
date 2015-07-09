#include <Windows.h>
#include "resource.h"
#include <D3D10.h>
#include <D3DX10.h>

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	D3DXVECTOR3 Pos;
};


HWND g_hWnd = NULL;
HINSTANCE g_hInstance = NULL;

ID3D10Device* g_pDevice = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
D3D10_DRIVER_TYPE g_pDriverType;
ID3D10RenderTargetView* g_pTargetView = NULL;
ID3D10Effect* g_pEffect = NULL;
ID3D10EffectTechnique* g_pTechnique = NULL;
ID3D10InputLayout* g_pInputLayout = NULL;
ID3D10Buffer* g_pVectorBuffer = NULL;

HRESULT InitWindows(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
void Render();
LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

///-------------------------------------------------------------------------------------------------
/// <summary>	Window main. </summary>
///
/// <remarks>	Song, 2015/7/9. </remarks>
///
/// <param name="hInstance">   	The instance. </param>
/// <param name="hPreInstance">	The pre instance. </param>
/// <param name="lpCmdLine">   	The command line. </param>
/// <param name="nCmdShow">	   	The command show. </param>
///
/// <returns>	A WINAPI. </returns>
///-------------------------------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPWSTR lpCmdLine, int nCmdShow)
{

	UNREFERENCED_PARAMETER(hPreInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindows(hInstance, nCmdShow)))
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
	return 0;
}

///-------------------------------------------------------------------------------------------------
/// <summary>	Initialises the windows. </summary>
///
/// <remarks>	Song, 2015/7/9. </remarks>
///
/// <param name="hInstance">	The instance. </param>
/// <param name="nCmdLine"> 	The command line. </param>
///
/// <returns>	A hResult. </returns>
///-------------------------------------------------------------------------------------------------
HRESULT InitWindows(HINSTANCE hInstance, int nCmdLine)
{

	WNDCLASSEX wcex;
	wcex.lpfnWndProc = WinProc;
	wcex.cbClsExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbWndExtra = 0;
	wcex.hCursor = LoadCursor(hInstance, (LPCWSTR)IDC_ARROW);
	wcex.hIcon = LoadIcon(hInstance, (LPCWSTR)IDI_WND_ICON);
	wcex.hIconSm = LoadIcon(hInstance, (LPCWSTR)IDI_WND_ICON);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"DirectX10Tutorials";
	wcex.lpszMenuName = NULL;
	wcex.hInstance = hInstance;
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wcex))
	{
		return	E_FAIL;
	}

	g_hInstance = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	g_hWnd = CreateWindow(L"DirectX10Tutorials", L"Tutorial03:draw Triangle", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!g_hWnd)
	{
		return E_FAIL;
	}

	ShowWindow(g_hWnd, nCmdLine);

	return S_OK;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
	if (!GetWindowRect(g_hWnd, &rc))
	{
		return E_FAIL;
	}

	UINT height = rc.bottom - rc.top;
	UINT width = rc.right - rc.left;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferCount = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.Height = rc.bottom - rc.top;
	sd.BufferDesc.Width = rc.right - rc.left;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.Windowed = true;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	UINT createDeviceFlag;
#ifdef _DEBUG
	createDeviceFlag |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	D3D10_DRIVER_TYPE driverTypes[]
	{
		D3D10_DRIVER_TYPE_HARDWARE,
			D3D10_DRIVER_TYPE_REFERENCE
	};

	UINT typesLenght = sizeof(driverTypes) / sizeof(driverTypes[0]);

	for (int typeIndex = 0; typeIndex < typesLenght; typeIndex++)
	{
		g_pDriverType = driverTypes[typeIndex];
		hr = D3D10CreateDeviceAndSwapChain(NULL, g_pDriverType, NULL, createDeviceFlag, D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pDevice);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	ID3D10Texture2D* pBackBuffer;

	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	g_pDevice->CreateRenderTargetView(pBackBuffer, 0, &g_pTargetView);

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
	depthStencilDesc.MiscFlags = 0;
	depthStencilDesc.Usage = D3D10_USAGE_DEFAULT;

	ID3D10Texture2D* pDepthStencilBuffer;

	hr = g_pDevice->CreateTexture2D(&depthStencilDesc, 0, &pDepthStencilBuffer);
	if (FAILED(hr))
		return hr;

	ID3D10DepthStencilView* pdepthStencilView;
	hr = g_pDevice->CreateDepthStencilView(pDepthStencilBuffer, 0, &pdepthStencilView);

	if (FAILED(hr))
		return hr;

	g_pDevice->OMSetRenderTargets(1, &g_pTargetView, pdepthStencilView);

	D3D10_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	g_pDevice->RSSetViewports(1, &vp);

	// Create the effect

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	hr = D3DX10CreateEffectFromFile(L"Tutorial02.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0,
		g_pDevice, NULL, NULL, &g_pEffect, NULL, NULL);

	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be located.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	g_pTechnique = g_pEffect->GetTechniqueByName("Render");

	D3D10_INPUT_ELEMENT_DESC layout[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	D3D10_PASS_DESC passDesc;

	g_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	hr = g_pDevice->CreateInputLayout(layout, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &g_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}

	g_pDevice->IASetInputLayout(g_pInputLayout);

	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		D3DXVECTOR3(0.0f, 0.5f, 0.5f),
		D3DXVECTOR3(0.5f, -0.5f, 0.5f),
		D3DXVECTOR3(-0.5f, -0.5f, 0.5f),
	};

	D3D10_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D10_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(SimpleVertex) * 3;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;

	hr = g_pDevice->CreateBuffer(&bufferDesc, &InitData, &g_pVectorBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	g_pDevice->IASetVertexBuffers(0, 1, &g_pVectorBuffer, &stride, &offset);

	g_pDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return S_OK;
}

void CleanupDevice()
{
	if (g_pDevice)
		g_pDevice->ClearState();

	if (g_pVectorBuffer) g_pVectorBuffer->Release();
	if (g_pInputLayout) g_pInputLayout->Release();
	if (g_pEffect) g_pEffect->Release();
	if (g_pTargetView)
		g_pTargetView->Release();
	if (g_pSwapChain)
		g_pSwapChain->Release();
	if (g_pDevice)
		g_pDevice->Release();
}

void Render()
{

	float color[4] = { 0.0f, 0.125f, 0.3f, 1.0f };

	g_pDevice->ClearRenderTargetView(g_pTargetView, color);

	D3D10_TECHNIQUE_DESC techDesc;
	g_pTechnique->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; p++)
	{
		g_pTechnique->GetPassByIndex(p)->Apply(0);
		g_pDevice->Draw(3, 0);
	}

	g_pSwapChain->Present(0, 0);
}