#include "pch.h"
#include "d3d12_renderer.h"


const int MSAA_SAMPLE_COUNT = 8;

static Renderer::Context context;

void Renderer::Initialize(Asset_Manager& asset_manager, Render_Group& render_group) {
    HRESULT hr;

    context.window = GetActiveWindow();
    context.fence_event = CreateEventExA(nullptr, "GPU fence", 0, EVENT_ALL_ACCESS);

    UINT flags = 0;
#ifdef _DEBUG
    ComPtr<ID3D12Debug> debug_controller;
    hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller));
    if (FAILED(hr)) {
        Log::Fatal("Failed to activate D3D12 debug interface.");
    }
    debug_controller->EnableDebugLayer();
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    ComPtr<IDXGIFactory4> factory;
    hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        Log::Fatal("Failed to create DXGI factory.");
    }

    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) {
            adapter.Reset();
            continue;
        }

        hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&context.device));
        if (FAILED(hr)) {
            Log::Fatal("Failed to create D3D12 device.");
        }
        else {
            break;
        }

        adapter.Reset();
    }

    if (adapter == nullptr) {
        Log::Fatal("No D3D12 capable hardware adapter found.");
    }

    context.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&context.fence));
    D3D12_COMMAND_QUEUE_DESC queue_desc;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queue_desc.NodeMask = 0;
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Priority = 0;
    hr = context.device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&context.command_queue));
    if (FAILED(hr)) {
        Log::Fatal("Failed to create D3D12 command queue.");
    }

    for (int i = 0; i < BACKBUFFER_COUNT; ++i) {
        hr = context.device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, 
            IID_PPV_ARGS(&context.frame_context[i].command_allocator)
        );
        if (FAILED(hr)) {
            Log::Fatal("Failed to create D3D12 command allocator.");
        }
    }

    ID3D12CommandAllocator* texture_upload_alloc;
    hr = context.device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&texture_upload_alloc)
    );
    if (FAILED(hr)) {
        Log::Fatal("Failed to create D3D12 texture upload allocator.");
    }

    std::vector<D3D12_RESOURCE_BARRIER> resource_barriers;
    ID3D12Fence* texture_upload_fence;
    context.device->CreateFence(0, {}, IID_PPV_ARGS(&texture_upload_fence));
    HANDLE texture_upload_fence_event = CreateEventW(nullptr, false, false, nullptr);
    
    hr = context.device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        texture_upload_alloc,
        nullptr,
        IID_PPV_ARGS(&context.command_list)
    );
    if (FAILED(hr)) {
        Log::Fatal("Failed to create D3D12 command list.");
    }

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
        .Width = (UINT)render_group.width,
        .Height = (UINT)render_group.height,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = false,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = BACKBUFFER_COUNT,
        .Scaling = DXGI_SCALING_STRETCH,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
    };

    ComPtr<IDXGISwapChain1> swap_chain;
    hr = factory->CreateSwapChainForHwnd(
        context.command_queue,
        context.window,
        &swap_chain_desc,
        nullptr,
        nullptr,
        &swap_chain
    );
    if (hr < 0) {
        Log::Fatal("Failed to create DXGI swap chain.");
    }

    hr = swap_chain.As(&context.swap_chain);
    if (hr < 0) {
        Log::Fatal("Failed to cast IDXGISwapChain1 into IDXGISwapChain4.");
    }

    IDXGIOutput* output;
    context.swap_chain->GetContainingOutput(&output);

    DXGI_OUTPUT_DESC output_desc;
    output->GetDesc(&output_desc);

    context.viewport = {
        .TopLeftX = 0.0,
        .TopLeftY = 0.0,
        .Width = (FLOAT)render_group.width,
        .Height = (FLOAT)render_group.height,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f,
    };

    context.scissor_rect = {
        .left = 0,
        .top = 0,
        .right = render_group.width,
        .bottom = render_group.height,
    };

    // Render target views
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {
        .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        .NumDescriptors = BACKBUFFER_COUNT + 1,
        .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };
    hr = context.device->CreateDescriptorHeap(
        &rtv_heap_desc, 
        IID_PPV_ARGS(&context.rtv_heap.heap)
    );
    if (FAILED(hr < 0)) {
        Log::Fatal("Failed to create RTV descriptor heap.");
    }
    context.rtv_heap.size = context.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Depth-stencil views
    D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc = {
        .Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
        .NumDescriptors = 2,
        .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };

    hr = context.device->CreateDescriptorHeap(
        &dsv_heap_desc,
        IID_PPV_ARGS(&context.dsv_heap.heap)
    );
    if (FAILED(hr)) {
        Log::Fatal("Failed to create DSV descriptor heap.");
    }
    context.dsv_heap.size = context.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_CLEAR_VALUE depth_clear = {
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .DepthStencil = {
            .Depth = 1.0f,
            .Stencil = 0,
        },
    };

    D3D12_RESOURCE_DESC resource_desc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        .Alignment = 0,
        .Width = (UINT64)render_group.width,
        .Height = (UINT64)render_group.height,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .SampleDesc = {
            .Count = 1,
            .Quality = 0,
        },
        .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
    };

    D3D12_HEAP_PROPERTIES heap_props = { .Type = D3D12_HEAP_TYPE_DEFAULT };
    hr = context.device->CreateCommittedResource(
        &heap_props,
        D3D12_HEAP_FLAG_NONE,
        &resource_desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depth_clear,
        IID_PPV_ARGS(&context.depth_stencil)
    );
    if (FAILED(hr)) {
        Log::Fatal("Failed to create depth stencil buffer.");
    }

    // MSAA
    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS ms_quality_levels = {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleCount = MSAA_SAMPLE_COUNT,
    };
    context.device->CheckFeatureSupport(
        D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &ms_quality_levels,
        sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS)
    );
    assert(ms_quality_levels.NumQualityLevels > 0);

    D3D12_RESOURCE_DESC msaa_desc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        .Alignment = 0,
        .Width = (UINT64)render_group.width,
        .Height = render_group.height,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = {
            .Count = MSAA_SAMPLE_COUNT,
            .Quality = 0,
        },
        .Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
    };

    D3D12_CLEAR_VALUE msaa_target_clear = {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Color = {0, 0, 0, 0},
    };

    D3D12_HEAP_PROPERTIES msaa_heap_props = {
        .Type = D3D12_HEAP_TYPE_DEFAULT,
    };
    context.device->CreateCommittedResource(
        &msaa_heap_props, D3D12_HEAP_FLAG_NONE,
        &msaa_desc, D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
        &msaa_target_clear,
        IID_PPV_ARGS(&context.msaa_target)
    );

    D3D12_RESOURCE_DESC msaa_depth_desc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        .Width = render_group.width,
        .Height = render_group.height,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
        .SampleDesc = {
            .Count = MSAA_SAMPLE_COUNT,
            .Quality = 0,
        },
        .Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL,
    };
    context.device->CreateCommittedResource(
        &msaa_heap_props, D3D12_HEAP_FLAG_NONE,
        &msaa_depth_desc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depth_clear,
        IID_PPV_ARGS(&context.msaa_depth_stencil)
    );

    // Render target views

    
    Log::Info("DirectX12 renderer initialized.");
}