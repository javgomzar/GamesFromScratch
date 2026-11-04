#include "d3d12_renderer.h"


void Renderer::Initialize(HWND active_window, Asset_Manager& asset_manager, Render_Group& render_group) {
    HRESULT hr;
    
    window = active_window;
    fence_event = CreateEventExA(nullptr, "GPU fence", 0, EVENT_ALL_ACCESS);

    UINT flags = 0;

#ifdef _DEBUG
    ComPtr<ID3D12Debug> debug_controller;
    hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
    if (FAILED(hr)) {
        Log(Log_Level::Fatal, "Failed to activate D3D12 debug interface.");
    }
    debug_controller->EnableDebugLayer();
    debug_controller->Release();
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    ComPtr<IDXGIFactory4> factory;
    hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        Log(Log_Level::Fatal, "Failed to create DXGI factory.");
    }

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) {
            adapter->Release();
            continue;
        }

        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
        if (FAILED(hr)) {
            Log(Log_Level::Fatal, "Failed to create D3D12 device.");
        }
        else {
            break;
        }

        adapter->Release();
        adapter = nullptr;
    }

    if (adapter == nullptr) {
        Log(Log_Level::Fatal, "No D3D12 capable hardware adapter found.");
    }

    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
    D3D12_COMMAND_QUEUE_DESC queue_desc;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.NodeMask = 0;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Priority = 0;
    hr = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue));
    if (FAILED(hr)) {
        Log(Log_Level::Fatal, "Failed to create D3D12 command queue.");
    }

    for (int i = 0; i < BACKBUFFER_COUNT; ++i) {
        hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&frame_context[i].command_allocator));
        if (FAILED(hr)) {
            Log(Log_Level::Fatal, "Failed to create D3D12 command allocator.");
        }
    }

    
}