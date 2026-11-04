#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include "GameRender.h"
#include "dxc_compiler.h"


const int BACKBUFFER_COUNT = 2;
const int MSAA_SAMPLE_COUNT = 8;

struct Frame_Context {
    ID3D12CommandAllocator* command_allocator;
    uint64 fence_value;
};

struct Descriptor_Heap {
    ID3D12DescriptorHeap* heap;
    uint32 size;
    uint32 count;
};

struct Dynamic_Buffer {
    ID3D12Resource* resource;
    void* memory;
};

class Renderer {
    uint64 frame;
    HWND window;
    ComPtr<ID3D12Device> device;
    ID3D12Fence* fence;
    HANDLE fence_event;
    IDXGISwapChain4* swap_chain;
    D3D12_VIEWPORT viewport;
    D3D12_RECT rect;
    ID3D12CommandQueue* command_queue;
    ID3D12GraphicsCommandList* command_list;
    Frame_Context frame_context[BACKBUFFER_COUNT];
    Descriptor_Heap rtv_heap;
    Descriptor_Heap dsv_heap;
    Descriptor_Heap srv_heap;
    Descriptor_Heap sampler_heap;
    ID3D12Resource* render_target[BACKBUFFER_COUNT];
    ID3D12Resource* depth_stencil;
    ID3D12Resource* msaa_target;
    ID3D12Resource* msaa_depth_stencil;
    struct {
        ID3D12Resource* position;
        ID3D12Resource* attribute;
        ID3D12Resource* joint;
        ID3D12Resource* index;
    } static_buffers;
    struct {
        Dynamic_Buffer position;
        Dynamic_Buffer attribute;
        Dynamic_Buffer index;
        Dynamic_Buffer text_instance;
        Dynamic_Buffer text_offset;
        Dynamic_Buffer poses;
    } dynamic_buffers[BACKBUFFER_COUNT];
    ID3D12RootSignature* root_signature;
    Hash_Table<Asset_ID, uint32> textures;
    DXC_Compiler compiler;

    void Initialize(HWND active_window, Asset_Manager& asset_manager, Render_Group& render_group);
    void Render();
};
