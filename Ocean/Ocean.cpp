//--------------------------------------------------------------------------------------
// File: Tutorial07.cpp
//
// This application demonstrates texturing
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729724.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include <windows.h>
#include <wrl.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "ReadData.h"
#include "resource.h"

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
    XMFLOAT3 Pos;
	XMFLOAT3 Norm;
    XMFLOAT2 Tex;
};

struct TessellationVertex
{
	XMFLOAT3 Pos;
};

struct CBNeverChanges
{
    XMMATRIX mView;
};

struct CBChangeOnResize
{
    XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
};


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE                                         g_hInst = nullptr;
HWND                                              g_hWnd = nullptr;
D3D_DRIVER_TYPE                                   g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL                                 g_featureLevel = D3D_FEATURE_LEVEL_11_0;
Microsoft::WRL::ComPtr<ID3D11Device>              g_pd3dDevice = nullptr;
Microsoft::WRL::ComPtr<ID3D11DeviceContext>       g_pImmediateContext = nullptr;
Microsoft::WRL::ComPtr<IDXGISwapChain>            g_pSwapChain = nullptr;
Microsoft::WRL::ComPtr<ID3D11RenderTargetView>    g_pRenderTargetView = nullptr;
Microsoft::WRL::ComPtr<ID3D11Texture2D>           g_pDepthStencil = nullptr;
Microsoft::WRL::ComPtr<ID3D11DepthStencilView>    g_pDepthStencilView = nullptr;
Microsoft::WRL::ComPtr<ID3D11VertexShader>        g_pVertexShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11VertexShader>        g_pOceanVertexShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11HullShader>          g_pOceanHullShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11DomainShader>        g_pOceanDomainShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11PixelShader>         g_pOceanPixelShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11PixelShader>         g_pOceanNormalPixelShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11PixelShader>         g_pOceanWiredPixelShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11PixelShader>         g_pPixelShader = nullptr;
Microsoft::WRL::ComPtr<ID3D11InputLayout>         g_pVertexLayout = nullptr;
Microsoft::WRL::ComPtr<ID3D11InputLayout>         g_pTessVertexLayout = nullptr;
Microsoft::WRL::ComPtr<ID3D11Buffer>              g_pVertexBuffer = nullptr;
Microsoft::WRL::ComPtr<ID3D11Buffer>              g_pTessVertexBuffer = nullptr;
Microsoft::WRL::ComPtr<ID3D11Buffer>              g_pIndexBuffer = nullptr;
Microsoft::WRL::ComPtr<ID3D11Buffer>              g_pCBNeverChanges = nullptr;
Microsoft::WRL::ComPtr<ID3D11Buffer>              g_pCBChangeOnResize = nullptr;
Microsoft::WRL::ComPtr<ID3D11Buffer>              g_pCBChangesEveryFrame = nullptr;
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  g_pTextureRV = nullptr;
Microsoft::WRL::ComPtr<ID3D11SamplerState>        g_pSamplerLinear = nullptr;
XMMATRIX                                          g_World;
XMMATRIX                                          g_View;
XMMATRIX                                          g_Projection;
XMFLOAT4                                          g_vMeshColor( 0.7f, 0.7f, 0.7f, 1.0f );

const float SIZE_TERRAIN = 2000.0f;
const unsigned int SQRT_NUMBER_OF_PATCHS = 8;

//Height map and normal/height map textures
//const LPCWSTR TEXTURE_HEIGHT_NAME = L"media\\terrain1(0.5K).png";
//const LPCWSTR TEXTURE_NORMAL_NAME = L"media\\terrain1NormalHeight(0.5K).dds";
const LPCWSTR TEXTURE_HEIGHT_NAME = L"..\\media\\terrain1(1K).png";
const LPCWSTR TEXTURE_NORMAL_NAME = L"..\\media\\terrain1NormalHeight(1K).dds";
//const LPCWSTR TEXTURE_HEIGHT_NAME = L"media\\terrain1(2K).png";
//const LPCWSTR TEXTURE_NORMAL_NAME = L"media\\terrain1NormalHeight(2K).dds";
//const LPCWSTR TEXTURE_HEIGHT_NAME = L"media\\terrain1(4K).png";
//const LPCWSTR TEXTURE_NORMAL_NAME = L"media\\terrain1NormalHeight(4K).dds";

//Terrain Texture
const LPCWSTR textureTerrainName = L"..\\media\\rock.jpg";

//Texture size
//const UINT textureWidth = 512;
//const UINT textureHeight = 512;
const UINT textureWidth = 1024;
const UINT textureHeight = 1024;
//const UINT textureWidth = 2048;
//const UINT textureHeight = 2048;
//const UINT textureWidth = 4096;
//const UINT textureHeight = 4096;


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"OceanWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1024, 768 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"OceanWindowClass", L"Ocean rendering with hardware tessellation",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;


	//-------------------------------------------
	//  Obtain window width and height
	//-------------------------------------------
    RECT rc;
    GetClientRect( g_hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;


	//-------------------------------------------
	//  Create device and immediate context
	//-------------------------------------------
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE( driverTypes );

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1
    };
    UINT numFeatureLevels = ARRAYSIZE( featureLevels );

    for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice( nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                D3D11_SDK_VERSION, g_pd3dDevice.ReleaseAndGetAddressOf(), &g_featureLevel, g_pImmediateContext.ReleaseAndGetAddressOf() );
        if( SUCCEEDED( hr ) )
            break;
    }
    if( FAILED( hr ) )
        return hr;


	//-------------------------------------------
	//  Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	//-------------------------------------------
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgiFactory2;
	{
		Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
		hr = g_pd3dDevice.As(&dxgiDevice);
		if (FAILED(hr))
			return hr;

		Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
		hr = dxgiDevice->GetAdapter(adapter.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return hr;

		Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
		hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr))
			return hr;

		hr = dxgiFactory.As(&dxgiFactory2);
		if (FAILED(hr))
			return hr;
	}


	//-------------------------------------------
	//  Check DirectX 11.1 or later available
	//-------------------------------------------
	Microsoft::WRL::ComPtr<ID3D11Device1> g_pd3dDevice1 = nullptr;
    hr = g_pd3dDevice.As(&g_pd3dDevice1);
	if (FAILED(hr))
		return hr;

	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> g_pImmediateContext1 = nullptr;
	hr = g_pImmediateContext.As(&g_pImmediateContext1);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//  Create swap chain
	//-------------------------------------------
    DXGI_SWAP_CHAIN_DESC1 sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> g_pSwapChain1 = nullptr;
    hr = dxgiFactory2->CreateSwapChainForHwnd( g_pd3dDevice.Get(), g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1 );
	if (FAILED(hr))
		return hr;

    hr = g_pSwapChain1.As(&g_pSwapChain);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Block Alt+Enter shortcut
	//-------------------------------------------
    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    hr = dxgiFactory2->MakeWindowAssociation( g_hWnd, DXGI_MWA_NO_ALT_ENTER );
    if (FAILED(hr))
        return hr;


	//-------------------------------------------
	//   Create a render target view
	//-------------------------------------------
	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( pBackBuffer.ReleaseAndGetAddressOf() ) );
    if( FAILED( hr ) )
        return hr;

    hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer.Get(), nullptr, g_pRenderTargetView.GetAddressOf() );
    if( FAILED( hr ) )
        return hr;


	//-------------------------------------------
	//   Create depth stencil texture
	//-------------------------------------------
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory( &descDepth, sizeof(descDepth) );
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D( &descDepth, nullptr, g_pDepthStencil.ReleaseAndGetAddressOf() );
    if( FAILED( hr ) )
        return hr;


	//-------------------------------------------
	//   Create the depth stencil view
	//-------------------------------------------
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory( &descDSV, sizeof(descDSV) );
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil.Get(), &descDSV, g_pDepthStencilView.ReleaseAndGetAddressOf() );
    if( FAILED( hr ) )
        return hr;


    //-------------------------------------------
	//   Create vertex buffer for the cube
	//-------------------------------------------
	SimpleVertex vertices[] =
	{
		// Upper cover
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },

		// Lower cover								  			    
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f, -1.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },

		// Left cover
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(-1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },

		// Right cover
		{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f,  0.0f,  0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f,  0.0f,  0.0f), XMFLOAT2(1.0f, 0.0f) },

		// Back cover
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f,  0.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f,  0.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f,  0.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f,  0.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },

		// Front cover
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f,  0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f,  0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f,  0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f,  0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
	};

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * 24;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vBInitData;
	ZeroMemory(&vBInitData, sizeof(vBInitData));
	vBInitData.pSysMem = vertices;
	hr = g_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vBInitData, &g_pVertexBuffer);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create index buffer for the cube
	//-------------------------------------------
	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		6,4,5,
		7,4,6,

		11,9,8,
		10,9,11,

		14,12,13,
		15,12,14,

		19,17,16,
		18,17,19,

		22,20,21,
		23,20,22
	};

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(WORD) * 36;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA indexBufferInitData;
	ZeroMemory(&indexBufferInitData, sizeof(indexBufferInitData));
	indexBufferInitData.pSysMem = indices;
	hr = g_pd3dDevice->CreateBuffer(&indexBufferDesc, &indexBufferInitData, &g_pIndexBuffer);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create the transform vertex shader
	//-------------------------------------------
	auto blobTransformVS = DX::ReadData(L"Transform_VS.cso");
    hr = g_pd3dDevice->CreateVertexShader(blobTransformVS.data(), blobTransformVS.size(), nullptr, &g_pVertexShader );
    if( FAILED( hr ) )
        return hr;


	//-------------------------------------------
	//   Create the simple vertex buffer layout
	//-------------------------------------------
    D3D11_INPUT_ELEMENT_DESC simpleVertexLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0,    DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(simpleVertexLayout);
    hr = g_pd3dDevice->CreateInputLayout(simpleVertexLayout, numElements, blobTransformVS.data(), blobTransformVS.size(), &g_pVertexLayout );
    if( FAILED( hr ) )
        return hr;


	//-------------------------------------------
	//   Create the textured pixel shader
	//-------------------------------------------
	auto blobPS = DX::ReadData(L"Textured_PS.cso");
	hr = g_pd3dDevice->CreatePixelShader(blobPS.data(), blobPS.size(), nullptr, &g_pPixelShader);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create vertex buffer for tessallation
	//-------------------------------------------
	TessellationVertex tessallationVertexBuffer[SQRT_NUMBER_OF_PATCHS * SQRT_NUMBER_OF_PATCHS * 4];

	unsigned int index = 0;
	float bigInc = SIZE_TERRAIN / SQRT_NUMBER_OF_PATCHS;
	for (unsigned int i = 0; i < SQRT_NUMBER_OF_PATCHS; ++i)
	{
		for (unsigned int j = 0; j < SQRT_NUMBER_OF_PATCHS; ++j)
		{
			//Here we calculate 4 control points
			tessallationVertexBuffer[index + 0].Pos = XMFLOAT3(bigInc * i, 0.0f, bigInc * j);
			tessallationVertexBuffer[index + 1].Pos = XMFLOAT3(bigInc * i, 0.0f, bigInc * (j + 1));
			tessallationVertexBuffer[index + 2].Pos = XMFLOAT3(bigInc * (i + 1), 0.0f, bigInc * (j + 1));
			tessallationVertexBuffer[index + 3].Pos = XMFLOAT3(bigInc * (i + 1), 0.0f, bigInc * j);
			index += 4;
		}
	}

	D3D11_BUFFER_DESC tessVertexBufferDesc;
	ZeroMemory(&tessVertexBufferDesc, sizeof(tessVertexBufferDesc));
	tessVertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	tessVertexBufferDesc.ByteWidth = sizeof(TessellationVertex) * SQRT_NUMBER_OF_PATCHS * SQRT_NUMBER_OF_PATCHS * 4;
	tessVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	tessVertexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA tessVBInitData;
	ZeroMemory(&tessVBInitData, sizeof(tessVBInitData));
	tessVBInitData.pSysMem = tessallationVertexBuffer;
	hr = g_pd3dDevice->CreateBuffer(&tessVertexBufferDesc, &tessVBInitData, &g_pTessVertexBuffer);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create the ocean vertex shader
	//-------------------------------------------
	auto blobOceanVS = DX::ReadData(L"Ocean_VS.cso");
	hr = g_pd3dDevice->CreateVertexShader(blobOceanVS.data(), blobOceanVS.size(), nullptr, &g_pOceanVertexShader);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------------
	//   Create the tessellation vertex buffer layout
	//-------------------------------------------------
	D3D11_INPUT_ELEMENT_DESC tessVertexLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	numElements = ARRAYSIZE(tessVertexLayout);
	hr = g_pd3dDevice->CreateInputLayout(tessVertexLayout, numElements, blobOceanVS.data(), blobOceanVS.size(), &g_pTessVertexLayout);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create the ocean hull shader
	//-------------------------------------------
	auto blobOceanHS = DX::ReadData(L"Ocean_HS.cso");
	hr = g_pd3dDevice->CreateHullShader(blobOceanHS.data(), blobOceanHS.size(), nullptr, &g_pOceanHullShader);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create the ocean domain shader
	//-------------------------------------------
	auto blobOceanDS = DX::ReadData(L"Ocean_DS.cso");
	hr = g_pd3dDevice->CreateDomainShader(blobOceanDS.data(), blobOceanDS.size(), nullptr, &g_pOceanDomainShader);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create the ocean pixel shader
	//-------------------------------------------
	auto blobOceanPS = DX::ReadData(L"Ocean_PS.cso");
	hr = g_pd3dDevice->CreatePixelShader(blobOceanPS.data(), blobOceanPS.size(), nullptr, &g_pOceanPixelShader);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create the ocean normal pixel shader
	//-------------------------------------------
	auto blobOceanNormalPS = DX::ReadData(L"Ocean_Normal_PS.cso");
	hr = g_pd3dDevice->CreatePixelShader(blobOceanNormalPS.data(), blobOceanNormalPS.size(), nullptr, &g_pOceanNormalPixelShader);
	if (FAILED(hr))
		return hr;


	//-------------------------------------------
	//   Create the ocean wired pixel shader
	//-------------------------------------------
	auto blobOceanWiredPS = DX::ReadData(L"Ocean_Wired_PS.cso");
	hr = g_pd3dDevice->CreatePixelShader(blobOceanWiredPS.data(), blobOceanWiredPS.size(), nullptr, &g_pOceanWiredPixelShader);
	if (FAILED(hr))
		return hr;

	g_pImmediateContext->OMSetRenderTargets(1, g_pRenderTargetView.GetAddressOf(), g_pDepthStencilView.Get());

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pImmediateContext->RSSetViewports(1, &vp);

    // Set the input layout
    g_pImmediateContext->IASetInputLayout( g_pVertexLayout.Get() );

    // Set vertex buffer
    UINT stride = sizeof( SimpleVertex );
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers( 0, 1, g_pVertexBuffer.GetAddressOf(), &stride, &offset );

    // Set index buffer
    g_pImmediateContext->IASetIndexBuffer( g_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0 );

    // Set primitive topology
    g_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    // Create the constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBNeverChanges );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangeOnResize);
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBChangeOnResize );
    if( FAILED( hr ) )
        return hr;
    
    bd.ByteWidth = sizeof(CBChangesEveryFrame);
    hr = g_pd3dDevice->CreateBuffer( &bd, nullptr, &g_pCBChangesEveryFrame );
    if( FAILED( hr ) )
        return hr;

    // Load the Texture
    hr = CreateDDSTextureFromFile( g_pd3dDevice.Get(), L"seafloor.dds", nullptr, &g_pTextureRV );
    if( FAILED( hr ) )
        return hr;

    // Create the sample state
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory( &sampDesc, sizeof(sampDesc) );
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = g_pd3dDevice->CreateSamplerState( &sampDesc, &g_pSamplerLinear );
    if( FAILED( hr ) )
        return hr;

    // Initialize the world matrices
    g_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
    XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
    g_View = XMMatrixLookAtLH( Eye, At, Up );

    CBNeverChanges cbNeverChanges;
    cbNeverChanges.mView = XMMatrixTranspose( g_View );
    g_pImmediateContext->UpdateSubresource( g_pCBNeverChanges.Get(), 0, nullptr, &cbNeverChanges, 0, 0 );

    // Initialize the projection matrix
    g_Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 100.0f );
    
    CBChangeOnResize cbChangesOnResize;
    cbChangesOnResize.mProjection = XMMatrixTranspose( g_Projection );
    g_pImmediateContext->UpdateSubresource( g_pCBChangeOnResize.Get(), 0, nullptr, &cbChangesOnResize, 0, 0 );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Render a frame
//--------------------------------------------------------------------------------------
void Render()
{
    // Update our time
    static float t = 0.0f;
    if( g_driverType == D3D_DRIVER_TYPE_REFERENCE )
    {
        t += ( float )XM_PI * 0.0125f;
    }
    else
    {
        static ULONGLONG timeStart = 0;
        ULONGLONG timeCur = GetTickCount64();
        if( timeStart == 0 )
            timeStart = timeCur;
        t = ( timeCur - timeStart ) / 1000.0f;
    }

    // Rotate cube around the origin
    g_World = XMMatrixRotationY( t );

    // Modify the color
    g_vMeshColor.x = ( sinf( t * 1.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.y = ( cosf( t * 3.0f ) + 1.0f ) * 0.5f;
    g_vMeshColor.z = ( sinf( t * 5.0f ) + 1.0f ) * 0.5f;

    //
    // Clear the back buffer
    //
    g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView.Get(), Colors::MidnightBlue );

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    g_pImmediateContext->ClearDepthStencilView( g_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

    //
    // Update variables that change once per frame
    //
    CBChangesEveryFrame cb;
    cb.mWorld = XMMatrixTranspose( g_World );
    cb.vMeshColor = g_vMeshColor;
    g_pImmediateContext->UpdateSubresource( g_pCBChangesEveryFrame.Get(), 0, nullptr, &cb, 0, 0 );

    //
    // Render the cube
    //
    g_pImmediateContext->VSSetShader( g_pVertexShader.Get(), nullptr, 0 );
    g_pImmediateContext->VSSetConstantBuffers( 0, 1, g_pCBNeverChanges.GetAddressOf());
    g_pImmediateContext->VSSetConstantBuffers( 1, 1, g_pCBChangeOnResize.GetAddressOf());
    g_pImmediateContext->VSSetConstantBuffers( 2, 1, g_pCBChangesEveryFrame.GetAddressOf() );
    g_pImmediateContext->PSSetShader( g_pPixelShader.Get(), nullptr, 0 );
    g_pImmediateContext->PSSetConstantBuffers( 2, 1, g_pCBChangesEveryFrame.GetAddressOf() );
    g_pImmediateContext->PSSetShaderResources( 0, 1, g_pTextureRV.GetAddressOf() );
    g_pImmediateContext->PSSetSamplers( 0, 1, g_pSamplerLinear.GetAddressOf() );
    g_pImmediateContext->DrawIndexed( 36, 0, 0 );

    //
    // Present our back buffer to our front buffer
    //
    g_pSwapChain->Present( 0, 0 );
}
