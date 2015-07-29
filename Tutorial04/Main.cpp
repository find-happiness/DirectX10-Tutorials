#include <Windows.h>
#include <d3d10.h>
#include "resource.h"
#include <D3DX10.h>

struct SimpleVexter
{
	D3DXVECTOR3 pos;
	D3DXVECTOR4 color;
};

ID3D10Device* gpDevice;
HWND hWnd;
IDXGISwapChain* gpSwapChain;
ID3D10RenderTargetView* gpRenderTargetView;
ID3D10Effect* gpEffect;
ID3D10EffectTechnique* g_pEffectTechnique;

ID3D10EffectMatrixVariable* g_pMatrixVariableWorld;
ID3D10EffectMatrixVariable* g_pMatrixVariableView;
ID3D10EffectMatrixVariable* g_pMatrixVariablePrjection;

ID3D10InputLayout* g_pInputLayout;
ID3D10Buffer* g_pBufferVertex;
ID3D10Buffer* g_pBufferIndex;

D3DXMATRIX worldMatrix;
D3DXMATRIX viewMatrix;
D3DXMATRIX prejectionMatrix;

D3D10_DRIVER_TYPE driverType;

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

	UINT height = rc.bottom - rc.top;
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

	for (int i = 0; i < numType; i++)
	{
		driverType = driverTypes[i];
		hr = D3D10CreateDeviceAndSwapChain(NULL, driverType, NULL, createFlag, D3D10_SDK_VERSION, &swapChainDesc, &gpSwapChain, &gpDevice);

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

	DWORD effectFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if _DEBUG
	effectFlag |= D3D10_SHADER_DEBUG;
#endif

	hr = D3DX10CreateEffectFromFile(L"tutorial04.fx", NULL, NULL, "fx_4_0", effectFlag, 0,
		gpDevice, NULL, NULL, &gpEffect, NULL, NULL);

	if (FAILED(hr))
	{
		MessageBox(NULL, L"effect file create failed!", L"error", MB_OK);
		return hr;
	}

	g_pEffectTechnique = gpEffect->GetTechniqueByName("Render");
	if (!g_pEffectTechnique)
		return E_FAIL;

	g_pMatrixVariableWorld = gpEffect->GetVariableByName("World")->AsMatrix();
	g_pMatrixVariableView = gpEffect->GetVariableByName("View")->AsMatrix();
	g_pMatrixVariablePrjection = gpEffect->GetVariableByName("Projection")->AsMatrix();

	D3D10_INPUT_ELEMENT_DESC layout[] = 
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D10_INPUT_PER_VERTEX_DATA,0},
		{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,12,D3D10_INPUT_PER_VERTEX_DATA,0}
	};

	UINT numLayout = sizeof(layout) / sizeof(layout[0]);

	D3D10_PASS_DESC passDesc;

	hr = g_pEffectTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	if (FAILED(hr))
		return hr;

	hr = gpDevice->CreateInputLayout(layout, numLayout, passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&g_pInputLayout);

	if (FAILED(hr))
		return hr;

	gpDevice->IASetInputLayout(g_pInputLayout);

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

	D3D10_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D10_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(SimpleVexter) * 8 ;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D10_SUBRESOURCE_DATA subresourceData;
	subresourceData.pSysMem = vertices;

	hr = gpDevice->CreateBuffer(&vertexBufferDesc, &subresourceData, &g_pBufferVertex);

	if (FAILED(hr))
		return hr;

	UINT stride = sizeof(SimpleVexter);
	UINT offset = 0;
	gpDevice->IASetVertexBuffers(0, 1, &g_pBufferVertex,&stride,&offset);


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

	vertexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(DWORD) * 36;
	vertexBufferDesc.Usage = D3D10_USAGE_DEFAULT;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	subresourceData.pSysMem = indices;

	hr = gpDevice->CreateBuffer(&vertexBufferDesc, &subresourceData, &g_pBufferIndex);
	if (FAILED(hr))
		return hr;

	gpDevice->IASetIndexBuffer(g_pBufferIndex,DXGI_FORMAT_R32_UINT,0);

	gpDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMatrixIdentity(&worldMatrix);

	D3DXVECTOR3 eye(0.0f, 1.0f, -5.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 at(0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&viewMatrix, &eye, &at, &up);

	D3DXMatrixPerspectiveFovLH(&prejectionMatrix, (float)D3DX_PI *0.5f, width / (FLOAT)height, 0.1f, 100.0f);

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
	if (gpEffect)
		gpEffect->Release();
	if (g_pBufferVertex)
		g_pBufferVertex->Release();
	if (g_pBufferIndex)
		g_pBufferIndex->Release();
	if (gpDevice)
		gpDevice->Release();
	
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

	gpDevice->ClearRenderTargetView(gpRenderTargetView, clearColor);


	g_pMatrixVariablePrjection->SetMatrix((float*)&prejectionMatrix);
	g_pMatrixVariableView->SetMatrix((float*)&viewMatrix);
	g_pMatrixVariableWorld->SetMatrix((float*)worldMatrix);

	D3D10_TECHNIQUE_DESC td;
	g_pEffectTechnique->GetDesc(&td);

	for (UINT i = 0; i < td.Passes; i++)
	{
		g_pEffectTechnique->GetPassByIndex(i)->Apply(0);
		gpDevice->DrawIndexed(36, 0, 0);
	}

	gpSwapChain->Present(0, 0);
}