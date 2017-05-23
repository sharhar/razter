#include <razter/razter.h>
#include <d3d11.h>
#include <d3dX11.h>
#include <d3dx10.h>
#include <stdio.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>

typedef struct DXCTX {
	IDXGISwapChain* swapChain;
	ID3D11Device* dev;
	ID3D11DeviceContext* devCon;
	ID3D11RenderTargetView* targetView;
} DXCTX;

void rzdxInit(RZRenderContext* ctx) {
	ctx->ctx = (DXCTX*)malloc(sizeof(DXCTX));

	memset(ctx->ctx, 0, sizeof(DXCTX));
}

GLFWwindow* rzdxCreateWindow(RZRenderContext* ctx, int width, int height, const char* title) {
	DXCTX* dxCTX = (DXCTX*)ctx->ctx;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	
	HWND hWnd = glfwGetWin32Window(window);
	HINSTANCE hInstance = (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE);

	printf("%p\n", hWnd);

	HRESULT hr;

	DXGI_MODE_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));
	bufferDesc.Width = width;
	bufferDesc.Height = height;
	bufferDesc.RefreshRate.Numerator = 60;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapChainDesc.BufferDesc = bufferDesc;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	hr = D3D11CreateDeviceAndSwapChain(NULL, 
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 
		NULL, 
		NULL, 
		NULL,
		D3D11_SDK_VERSION, 
		&swapChainDesc, 
		&dxCTX->swapChain, 
		&dxCTX->dev, 
		NULL, 
		&dxCTX->devCon);

	IDXGISwapChain* swapChain = dxCTX->swapChain;

	ID3D11Texture2D* backBuffer;
	hr = dxCTX->swapChain->lpVtbl->GetBuffer(dxCTX->swapChain, 0, sizeof(ID3D11Texture2D), (void**)&backBuffer);
	dxCTX->dev->lpVtbl->CreateRenderTargetView(dxCTX->dev, backBuffer, NULL, &dxCTX->targetView);
	backBuffer->lpVtbl->Release(backBuffer);

	dxCTX->devCon->lpVtbl->OMSetRenderTargets(dxCTX->devCon, 1, &dxCTX->targetView, NULL);

	return window;
}

void rzdxClear(RZRenderContext* ctx) {

}

void rzdxSetClearColor(RZRenderContext* ctx, float r, float g, float b, float a) {

}

void rzdxSwap(RZRenderContext* ctx) {

}

void rzdxLoadPFN(RZRenderContext* ctx) {
	ctx->init = rzdxInit;
	ctx->createWindow = rzdxCreateWindow;
	ctx->clear = rzdxClear;
	ctx->setClearColor = rzdxSetClearColor;
	ctx->swap = rzdxSwap;
}