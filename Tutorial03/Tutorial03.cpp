/*********************************************************************************
** Auth : song
** Date : 2015/07/10
** Desc : 绘制立方体
** Ver : v1.0.0
**
*********************************************************************************/

#include <Windows.h>
#include <d3d10.h>
#include <D3DX10.h>
#include "resource.h"


struct SimpleVexter
{
	D3DXVECTOR3 pos;
	D3DXVECTOR4 color;
};

HWND hWnd = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D10Device* g_pD3dDevice = NULL;
ID3D10RenderTargetView* g_pRenderTargetView = NULL;
ID3D10Effect* g_pEffect;
ID3D10EffectTechnique* g_pTechnique = NULL;
ID3D10InputLayout* g_pInputLayout;
ID3D10Buffer* g_pIndexBuffer;
ID3D10Buffer* g_pVertexBuffer;

ID3D10EffectMatrixVariable* g_pEffectMatrixVariableWorld = NULL;
ID3D10EffectMatrixVariable* g_pEffectMatrixVariableView = NULL;
ID3D10EffectMatrixVariable* g_pEffectMatrixVariablePrjection = NULL;

D3DXMATRIX worldMatrix;
D3DXMATRIX viewMatrix;
D3DXMATRIX prejectionMatrix;

D3D10_DRIVER_TYPE driverType;

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT InitDevice();
void CleanupDevice();
void Render();
LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPWSTR lpCmdLine, int nCmdShow)
{

	UNREFERENCED_PARAMETER(hPreInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

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

HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
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
	wcex.lpszClassName = L"DirectX10Tutorials";
	wcex.lpszMenuName = NULL;
	wcex.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	hWnd = CreateWindow(L"DirectX10Tutorials", L"Tutorial03:draw cube", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return E_FAIL;

	ShowWindow(hWnd, nCmdShow);

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
	//1.swap china

	HRESULT hr = S_OK;

	RECT rc;
	GetWindowRect(hWnd, &rc);

	UINT height = rc.bottom - rc.top;
	UINT width = rc.right - rc.left;

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.Windowed = true;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Flags = 0;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDriverFlag;
#ifdef _DEBUG
	createDriverFlag |= D3D10_CREATE_DEVICE_DEBUG;
#endif


	D3D10_DRIVER_TYPE driveTypes[] =
	{
		D3D10_DRIVER_TYPE_HARDWARE,
		D3D10_DRIVER_TYPE_REFERENCE,
	};
	
	UINT numDriverType = sizeof(driveTypes) / sizeof(driveTypes[0]);
	for (int d = 0; d < numDriverType; d++)
	{
		driverType = driveTypes[d];
		hr = D3D10CreateDeviceAndSwapChain(NULL, driverType, NULL, createDriverFlag, D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pD3dDevice);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	//2.创建渲染目标

	ID3D10Texture2D* pBackBuffer;

	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)(&pBackBuffer));

	if (FAILED(hr))
		return hr;

	hr = g_pD3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
	pBackBuffer->Release();

	if (FAILED(hr))
		return hr;

	g_pD3dDevice->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

	D3D10_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	g_pD3dDevice->RSSetViewports(1, &vp);


	//3.编译effect文件
	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	hr = D3DX10CreateEffectFromFile(L"Tutorial03.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0, g_pD3dDevice, NULL, NULL, &g_pEffect, NULL, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"effect file create failed!", L"error", MB_OK);
		return hr;
	}

	//
	g_pTechnique = g_pEffect->GetTechniqueByName("Render");
	if (!g_pTechnique)
		return E_FAIL;

	g_pEffectMatrixVariableWorld = g_pEffect->GetVariableByName("World")->AsMatrix();
	g_pEffectMatrixVariableView = g_pEffect->GetVariableByName("View")->AsMatrix();
	g_pEffectMatrixVariablePrjection = g_pEffect->GetVariableByName("Projection")->AsMatrix();

	D3D10_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// Create the input layout
	D3D10_PASS_DESC PassDesc;
	g_pTechnique->GetPassByIndex(0)->GetDesc(&PassDesc);
	hr = g_pD3dDevice->CreateInputLayout(layout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &g_pInputLayout);
	if (FAILED(hr))
		return hr;

	// Set the input layout
	g_pD3dDevice->IASetInputLayout(g_pInputLayout);

	SimpleVexter vertices[] =
	{
		{ D3DXVECTOR3(-1.0f, 1.0f, -1.0f), D3DXVECTOR4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, 1.0f, -1.0f), D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, 1.0f, 1.0f), D3DXVECTOR4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ D3DXVECTOR3(-1.0f, 1.0f, 1.0f), D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ D3DXVECTOR3(-1.0f, -1.0f, -1.0f), D3DXVECTOR4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, -1.0f), D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ D3DXVECTOR3(1.0f, -1.0f, 1.0f), D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ D3DXVECTOR3(-1.0f, -1.0f, 1.0f), D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f) },
	};

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVexter) * 8;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = vertices;
	hr = g_pD3dDevice->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVexter);
	UINT offset = 0;
	g_pD3dDevice->IASetVertexBuffers(0, 1, &g_pVertexBuffer, &stride, &offset);

	DWORD indices[] =
	{
		3, 1, 0,
		2, 1, 3,

		0, 5, 4,
		1, 5, 0,

		3, 4, 7,
		0, 4, 3,

		1, 6, 5,
		2, 6, 1,

		2, 7, 6,
		3, 7, 2,

		6, 4, 5,
		7, 4, 6,

	};

	bd.Usage = D3D10_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DWORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	InitData.pSysMem = indices;
	hr = g_pD3dDevice->CreateBuffer(&bd, &InitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	g_pD3dDevice->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set primitive topology
	g_pD3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMatrixIdentity(&worldMatrix);

	D3DXVECTOR3 eye(0.0f, 1.0f, -5.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 at(0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);

	D3DXMatrixPerspectiveFovLH(&prejectionMatrix, (float)D3DX_PI *0.5f, width / (FLOAT)height, 0.1f, 100.0f);

	return S_OK;
}

void CleanupDevice()
{
	if (g_pD3dDevice)
		g_pD3dDevice->ClearState();
	if (g_pInputLayout)
		g_pInputLayout->Release();
	if (g_pEffect)
		g_pEffect->Release();
	if (g_pRenderTargetView)
		g_pRenderTargetView->Release();
	if (g_pSwapChain)
		g_pSwapChain->Release();
	if (g_pD3dDevice)
		g_pD3dDevice->Release();
	if (g_pIndexBuffer)
		g_pIndexBuffer->Release();

}

void Render()
{
	static float t = 0.0f;
	if (driverType == D3D10_DRIVER_TYPE_REFERENCE)
	{
		t += (float)D3DX_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimerCur = GetTickCount();
		if (dwTimeStart == 0)
		{
			dwTimeStart = dwTimerCur;
		}
		t = (dwTimerCur - dwTimeStart) / 1000.f;
	}

	D3DXMatrixRotationY(&worldMatrix, t);

	float clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };

	g_pD3dDevice->ClearRenderTargetView(g_pRenderTargetView, clearColor);


	g_pEffectMatrixVariablePrjection->SetMatrix((float*)&prejectionMatrix);
	g_pEffectMatrixVariableView->SetMatrix((float*)&viewMatrix);
	g_pEffectMatrixVariableWorld->SetMatrix((float*)worldMatrix);

	D3D10_TECHNIQUE_DESC td;
	g_pTechnique->GetDesc(&td);

	for (UINT i = 0; i < td.Passes; i++)
	{
		g_pTechnique->GetPassByIndex(i)->Apply(0);
		g_pD3dDevice->DrawIndexed(36, 0, 0);
	}

	g_pSwapChain->Present(0, 0);

}