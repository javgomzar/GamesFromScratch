#include "d3d11.h"
#include "dxgi.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")

struct directX_framebuffer {
    render_group_target Target;
    int Samples;
    bool Multisampling;
    bool Depth;
    bool Stencil;
};

struct directX {
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;
    ID3D11RenderTargetView* TargetView;
    ID3D11Texture2D* DepthBuffer;
    ID3D11DepthStencilState* DepthStencilState;
    ID3D11DepthStencilView* DepthStencilView;
    ID3D11RasterizerState* RasterizerState;
    D3D11_VIEWPORT Viewport;
    float DPI;
    bool Initialized;
    bool VSync;
};

directX RendererContext;

RENDERER_INITIALIZE(directX) {
    HRESULT Result;

    // IDXGIFactory* Factory;
    // Result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory);
    // if (FAILED(Result)) Raise("Couldn't initialize DirectX11 factory.");
    
    // IDXGIAdapter* Adapter;
    // Result = Factory->EnumAdapters(0, &Adapter);
    // if (FAILED(Result)) Raise("Couldn't get DirectX11 adapter.");
    // DXGI_ADAPTER_DESC AdapterDescription;
    // Result = Adapter->GetDesc((&AdapterDescription));
    // if (FAILED(Result)) Raise("Couldn't get DirectX11 adapter description.");
    
    // IDXGIOutput* Output;
    // Result = Adapter->EnumOutputs(0, &Output);
    // if (FAILED(Result)) Raise("Couldn't get DirectX11 output.");

    // uint32 NumModes = 0;
    // Result = Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, NULL);
    // if (FAILED(Result)) Raise("Couldn't get DirectX11 display mode list size.");

    // DXGI_MODE_DESC* DisplayModes = new DXGI_MODE_DESC[NumModes];
    // Result = Output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &NumModes, DisplayModes);
    // if (FAILED(Result)) Raise("Couldn't get DirectX11 display mode list.");

    // int32 DisplayWidth = 1440;
    // int32 DisplayHeight = 1080;

    // uint32 Numerator = 0;
    // uint32 Denominator = 1;
    // for (int i = 0; i < NumModes; i++) {
    //     DXGI_MODE_DESC DisplayMode = DisplayModes[i];
    //     if (DisplayMode.Width == DisplayWidth && DisplayMode.Height == DisplayHeight) {
    //         Numerator = DisplayMode.RefreshRate.Numerator;
    //         Denominator = DisplayMode.RefreshRate.Denominator;
    //     }
    // }

    // delete [] DisplayModes;
    // DisplayModes = NULL;
    // Output->Release();
    // Output = NULL;
    // Adapter->Release();
    // Adapter = NULL;
    // Factory->Release();
    // Factory = NULL;

    DXGI_FORMAT PixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    DXGI_SWAP_CHAIN_DESC SwapChainDescription = {};
    SwapChainDescription.BufferCount = 2;
    SwapChainDescription.BufferDesc.Width = Group->Width;
    SwapChainDescription.BufferDesc.Height = Group->Height;
    SwapChainDescription.BufferDesc.Format = PixelFormat;

    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.OutputWindow = Window;
    SwapChainDescription.Windowed = TRUE;

    // MSAA
    SwapChainDescription.SampleDesc.Count = 1;
    SwapChainDescription.SampleDesc.Quality = 0;

    SwapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Discard the back buffer contents
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    
    // Advanced flags
    SwapChainDescription.Flags = 0;

    D3D_FEATURE_LEVEL FeatureLevel = D3D_FEATURE_LEVEL_11_0;
    Result = D3D11CreateDeviceAndSwapChain(
        NULL, 
        D3D_DRIVER_TYPE_HARDWARE, 
        NULL, 
        D3D11_CREATE_DEVICE_DEBUG, 
        &FeatureLevel, 
        1, 
        D3D11_SDK_VERSION, 
        &SwapChainDescription,
        &Renderer->SwapChain,
        &Renderer->Device,
        NULL,
        &Renderer->DeviceContext
    );
    if (FAILED(Result)) Raise("Couldn't create DirectX11 device.");

    // Multisample support
    UINT QualityLevels = 0;
    UINT MSAASamples = 8;
    Result = Renderer->Device->CheckMultisampleQualityLevels(PixelFormat, MSAASamples, &QualityLevels);
    if (FAILED(Result)) Raise("Couldn't query MSAA support.");
    Assert(QualityLevels > 0);

    // Attaching backbuffer to swap chain
    ID3D11Texture2D* Backbuffer;
    Result = Renderer->SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&Backbuffer);
    if (FAILED(Result)) Raise("Couldn't get DirectX backbuffer.");

    Result = Renderer->Device->CreateRenderTargetView(Backbuffer, NULL, &Renderer->TargetView);
    if (FAILED(Result)) Raise("Couldn't create DirectX target view.");

    Backbuffer->Release();
    Backbuffer = NULL;

    D3D11_TEXTURE2D_DESC DepthBufferDescription = {};
    DepthBufferDescription.Width = Group->Width;
    DepthBufferDescription.Height = Group->Height;
    DepthBufferDescription.MipLevels = 1;
    DepthBufferDescription.ArraySize = 1;
    DepthBufferDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthBufferDescription.SampleDesc.Count = 1;
    DepthBufferDescription.SampleDesc.Quality = 0;
    DepthBufferDescription.Usage = D3D11_USAGE_DEFAULT;
    DepthBufferDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    DepthBufferDescription.CPUAccessFlags = 0;
    DepthBufferDescription.MiscFlags = 0;

    Result = Renderer->Device->CreateTexture2D(&DepthBufferDescription, NULL, &Renderer->DepthBuffer);
    if (FAILED(Result)) Raise("Couldn't create the depth/stencil buffer.");

    D3D11_DEPTH_STENCIL_DESC DepthStencilDescription = {};
    DepthStencilDescription.DepthEnable = true;
	DepthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;

	DepthStencilDescription.StencilEnable = true;
	DepthStencilDescription.StencilReadMask = 0xFF;
	DepthStencilDescription.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	DepthStencilDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DepthStencilDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDescription.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	DepthStencilDescription.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDescription.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	DepthStencilDescription.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDescription.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    Result = Renderer->Device->CreateDepthStencilState(&DepthStencilDescription, &Renderer->DepthStencilState);
    Renderer->DeviceContext->OMSetDepthStencilState(Renderer->DepthStencilState, 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDescription = {};
    DepthStencilViewDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DepthStencilViewDescription.Texture2D.MipSlice = 0;

    Result = Renderer->Device->CreateDepthStencilView(Renderer->DepthBuffer, &DepthStencilViewDescription, &Renderer->DepthStencilView);
    if (FAILED(Result)) Raise("Couldn't create DirectX depth stencil view.");

    Renderer->DeviceContext->OMSetRenderTargets(1, &Renderer->TargetView, Renderer->DepthStencilView);

    D3D11_RASTERIZER_DESC RasterizerDescription;
    RasterizerDescription.AntialiasedLineEnable = false;
	RasterizerDescription.CullMode = D3D11_CULL_BACK;
	RasterizerDescription.DepthBias = 0;
	RasterizerDescription.DepthBiasClamp = 0.0f;
	RasterizerDescription.DepthClipEnable = true;
	RasterizerDescription.FillMode = D3D11_FILL_SOLID;
	RasterizerDescription.FrontCounterClockwise = false;
	RasterizerDescription.MultisampleEnable = false;
	RasterizerDescription.ScissorEnable = false;
	RasterizerDescription.SlopeScaledDepthBias = 0.0f;

    Result = Renderer->Device->CreateRasterizerState(&RasterizerDescription, &Renderer->RasterizerState);
    if (FAILED(Result)) Raise("Couldn't create DirectX rasterizer state.");
    Renderer->DeviceContext->RSSetState(Renderer->RasterizerState);

    Renderer->Viewport.Width = Group->Width;
    Renderer->Viewport.Height = Group->Height;
    Renderer->Viewport.MinDepth = 0.0f;
    Renderer->Viewport.MaxDepth = 1.0f;
    Renderer->Viewport.TopLeftX = 0.0f;
    Renderer->Viewport.TopLeftY = 0.0f;
    Renderer->DeviceContext->RSSetViewports(1, &Renderer->Viewport);

    Renderer->Initialized = true;
    
    Log(Info, "Direct3D 11 was successfully initialized.");
}

void ReloadShader(directX* DirectX, game_assets* Assets, game_shader* Shader) {

}

void ReloadShader(directX* DirectX, game_compute_shader* Shader) {

}

void ResizeWindow(directX* DirectX, int32 Width, int32 Height) {

}

void ScreenCapture(directX* DirectX, int32 Width, int32 Height) {

}

RENDERER_RENDER(directX) {
    TIMED_BLOCK;

    // Render entries
	for (int i = 0; i < Group->EntryCount; i++) {
		render_command Command = Group->Entries[i];

		switch(Command.Type) {
			case render_clear: {
				render_clear_command Clear = Group->Clears[Command.Index];

                float Color[4] = { Clear.Color.R, Clear.Color.G, Clear.Color.B, Clear.Color.Alpha };
                
                if (Command.Index == Target_Output) {
                    Renderer->DeviceContext->ClearRenderTargetView(Renderer->TargetView, Color);
                    Renderer->DeviceContext->ClearDepthStencilView(Renderer->DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
                }
                else {
                    // TODO: Clear arbitrary render targets
                }
			} break;
        }
    }

    Renderer->SwapChain->Present(1, 0);
}