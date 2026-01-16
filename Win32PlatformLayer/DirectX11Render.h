#include "d3d11.h"
#include "D3DCompiler.h"

#ifdef _DEBUG
#include "dxgidebug.h"
#else
#include "dxgi.h"
#endif

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Buffers                                                                                                                                                     |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct directX_render_target {
    render_group_target_description Description;
    ID3D11Texture2D* Texture;
    ID3D11RenderTargetView* View;
    ID3D11ShaderResourceView* ShaderTexture;
    ID3D11Texture2D* AttachmentTexture;
    ID3D11DepthStencilView* Attachment;
    ID3D11UnorderedAccessView* UnorderedAccessView;
};

struct directX_vertex_buffer {
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
};

typedef directX_vertex_buffer directX_mesh_buffer;
typedef directX_vertex_buffer directX_heightmap_buffer;

struct directX_font_buffer {
    ID3D11Buffer* VertexBuffer;
    ID3D11Buffer* IndexBuffer;
    ID3D11Buffer* InstanceBuffer;
};

ENUM(instanced_layout_id,
    instanced_layout_text_id,
    instanced_layout_test_id
);

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Shaders                                                                                                                                                          |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

ENUM(directX_Vertex_Shader_ID,
    Vertex_Shader_Passthrough_ID,
    Vertex_Shader_Screen_ID,
    Vertex_Shader_Screen_Texture_ID,
    Vertex_Shader_Barycentric_ID,
    Vertex_Shader_Perspective_ID,
    Vertex_Shader_Perspective_Texture_ID,
    Vertex_Shader_Mesh_ID,
    Vertex_Shader_Bones_ID,
    Vertex_Shader_Heightmap_ID,
    Vertex_Shader_Sky_ID,
    Vertex_Shader_Test_ID
);

ENUM(directX_Hull_Shader_ID,
    Hull_Shader_Heightmap_ID
);

ENUM(directX_Domain_Shader_ID,
    Domain_Shader_Heightmap_ID,
    Domain_Shader_Water_ID
);

ENUM(directX_Geometry_Shader_ID,
    Geometry_Shader_Normal_ID
);

ENUM(directX_Pixel_Shader_ID,
    Pixel_Shader_Antialiasing_ID,
    Pixel_Shader_Single_Color_ID,
    Pixel_Shader_Attrib_Color_ID,
    Pixel_Shader_Texture_ID,
    Pixel_Shader_Mesh_ID,
    Pixel_Shader_Bezier_Exterior_ID,
    Pixel_Shader_Bezier_Interior_ID,
    Pixel_Shader_Heightmap_ID,
    Pixel_Shader_Sky_ID,
    Pixel_Shader_Water_ID
);

ENUM(directX_Compute_Shader_ID,
    Compute_Shader_Outline_Init_ID,
    Compute_Shader_Jump_Flood_ID,
    Compute_Shader_Outline_ID
);

#define ShaderType(Type, ...) struct directX_##Type##_Shader { \
    directX_##Type##_Shader_ID ID; \
    read_file_result File; \
    ID3D11##Type##Shader* Shader; \
    ID3DBlob* Blob; \
    __VA_ARGS__; }

ShaderType(Vertex);
ShaderType(Hull);
ShaderType(Domain, ID3D11SamplerState* Sampler);
ShaderType(Geometry);
ShaderType(Pixel, ID3D11SamplerState* Sampler);
ShaderType(Compute);

ENUM(directX_constant_buffer_id,
    global_buffer_id,
    light_buffer_id,
    color_buffer_id,
    transform_buffer_id,
    bone_buffer_id,
    text_outline_buffer_id,
    outline_buffer_id
);

struct alignas(16) global_buffer {
    matrix4 Projection;
    matrix4 View;
    v2 Resolution;
    v2 Mouse;
    v2 LastMouse;
    float Time;
};

struct alignas(16) color_buffer {
    color Color;
};

struct alignas(16) transform_buffer {
    matrix4 Model;
    matrix4 Normal;
};

struct alignas(16) bone_buffer {
    matrix4 BoneTransforms[MAX_ARMATURE_BONES];
    matrix4 BoneNormalTransforms[MAX_ARMATURE_BONES];
    alignas(16) int nBones;
};

struct alignas(16) text_outline_buffer {
    v2 Pen;
    float Size;
};

struct alignas(16) tessellation_buffer {
    float Amount;
};

struct alignas(16) light_buffer {
	alignas(16) v3 Direction;
	alignas(16) v3 Color;
	alignas(16) v3 CameraPosition;
	float Ambient;
	float Diffuse;
};

struct alignas(16) outline_buffer {
    float Width;
    int Level;
};

// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+
// | Initialization                                                                                                                                                   |
// +------------------------------------------------------------------------------------------------------------------------------------------------------------------+

struct directX {
    game_assets* Assets;
    IDXGISwapChain* SwapChain;
    ID3D11Device* Device;
    ID3D11DeviceContext* DeviceContext;
    ID3D11Texture2D* StagingTexture;
    directX_render_target Target[render_group_target_count];
    ID3D11BlendState* CombineAlpha;
    ID3D11BlendState* OverwriteAlpha;
    ID3D11BlendState* TargetBlend;
    ID3D11ShaderResourceView* Texture[game_bitmap_id_count];
    ID3D11ShaderResourceView* Heightmap[game_heightmap_id_count];
    ID3D11DepthStencilState* DepthStencilEnabled;
    ID3D11DepthStencilState* DepthStencilDisabled;
    ID3D11RasterizerState* RasterizerState;
    D3D11_VIEWPORT Viewport;
    ID3D11InputLayout* VertexLayout[vertex_layout_id_count];
    ID3D11InputLayout* InstancedLayout[instanced_layout_id_count];
    ID3D11Buffer* VertexBuffer[vertex_layout_id_count];
    ID3D11Buffer* IndexBuffer;
    directX_Vertex_Shader VertexShader[directX_Vertex_Shader_ID_count];
    directX_Hull_Shader HullShader[directX_Hull_Shader_ID_count];
    directX_Domain_Shader DomainShader[directX_Domain_Shader_ID_count];
    directX_Geometry_Shader GeometryShader[directX_Geometry_Shader_ID_count];
    directX_Pixel_Shader PixelShader[directX_Pixel_Shader_ID_count];
    directX_Compute_Shader ComputeShader[directX_Compute_Shader_ID_count];
    ID3D11Buffer* ConstantBuffer[directX_constant_buffer_id_count];
    directX_mesh_buffer MeshBuffer[game_mesh_id_count];
    directX_font_buffer FontBuffer[game_font_id_count];
    directX_heightmap_buffer HeightmapBuffer;
    uint32 MSAASamples;
    // float DPI;
    bool Initialized;
    // bool VSync;
};

directX DirectX;

void CreateTarget(uint32 Width, uint32 Height, render_group_target_description TargetDescription) {
    directX_render_target* Target = &DirectX.Target[TargetDescription.Target];
    Target->Description = TargetDescription;
    
    D3D11_TEXTURE2D_DESC TextureDescription = {};
    TextureDescription.Width = Width;
    TextureDescription.Height = Height;
    TextureDescription.MipLevels = 1;
    TextureDescription.ArraySize = 1;
    TextureDescription.Usage = D3D11_USAGE_DEFAULT;
    TextureDescription.CPUAccessFlags = 0;
    TextureDescription.MiscFlags = 0;
    TextureDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    if (!TargetDescription.Multisample) TextureDescription.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    
    TextureDescription.SampleDesc.Quality = 0;
    TextureDescription.SampleDesc.Count = TargetDescription.Multisample ? DirectX.MSAASamples : 1;
        
    switch(TargetDescription.Format) {
        case Color_Format_R:    { TextureDescription.Format = DXGI_FORMAT_R32_FLOAT; } break;
        case Color_Format_RG:   { TextureDescription.Format = DXGI_FORMAT_R32G32_FLOAT; } break;
        case Color_Format_RGB:  { TextureDescription.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; } break;
        case Color_Format_RGBA: { TextureDescription.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; } break;
        default: Raise("DirectX: Invalid color format creating render target.");
    }

    HRESULT Result = DirectX.Device->CreateTexture2D(&TextureDescription, NULL, &Target->Texture);
    if (FAILED(Result)) Raise("DirectX: Render target texture creation failed.");

    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDescription = {};
    ShaderResourceViewDescription.Format = TextureDescription.Format;
    ShaderResourceViewDescription.ViewDimension = 
        TargetDescription.Multisample ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceViewDescription.Texture2D.MostDetailedMip = 0;
    ShaderResourceViewDescription.Texture2D.MipLevels = -1;
    Result = DirectX.Device->CreateShaderResourceView(Target->Texture, &ShaderResourceViewDescription, &Target->ShaderTexture); 
    if (FAILED(Result)) Raise("DirectX: Shader resource view for target texture failed.");

    if (!TargetDescription.Multisample) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDescription;
        UAVDescription.Format = TextureDescription.Format;
        UAVDescription.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        UAVDescription.Texture2D.MipSlice = 0;
        Result = DirectX.Device->CreateUnorderedAccessView(Target->Texture, &UAVDescription, &Target->UnorderedAccessView);
        if (FAILED(Result)) Raise("DirectX: Unordered access view creation for target failed.");
    }

    Result = DirectX.Device->CreateRenderTargetView(Target->Texture, NULL, &Target->View);
    if (FAILED(Result)) Raise("DirectX: Target creation failed.");

    if (TargetDescription.Depth || TargetDescription.Stencil) {
        TextureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        TextureDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

        Result = DirectX.Device->CreateTexture2D(&TextureDescription, NULL, &Target->AttachmentTexture);
        if (FAILED(Result)) Raise("DirectX: Render target attachment texture creation failed.");

        D3D11_DEPTH_STENCIL_VIEW_DESC RenderTargetDepthStencilViewDescription = {};
        RenderTargetDepthStencilViewDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        RenderTargetDepthStencilViewDescription.ViewDimension = 
            TargetDescription.Multisample ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
        RenderTargetDepthStencilViewDescription.Texture2D.MipSlice = 0;
        Result = DirectX.Device->CreateDepthStencilView(Target->AttachmentTexture, &RenderTargetDepthStencilViewDescription, &Target->Attachment);
        if (FAILED(Result)) Raise("DirectX: Depth/stencil view creation failed.");
    }
}

void BindTarget(render_group_target Target) {
    directX_render_target* RenderTarget = &DirectX.Target[Target];
    DirectX.DeviceContext->OMSetRenderTargets(1, &RenderTarget->View, RenderTarget->Attachment);
}

ID3D11ShaderResourceView* CreateTexture(uint32 Width, uint32 Height, void* Data = NULL) {
    ID3D11Texture2D* Texture = NULL;

    D3D11_TEXTURE2D_DESC Description = {};
    Description.Width = Width;
    Description.Height = Height;
    Description.MipLevels = 1;
    Description.ArraySize = 1;
    Description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Description.SampleDesc.Count = 1;
    Description.SampleDesc.Quality = 0;
    Description.Usage = D3D11_USAGE_DEFAULT;
    Description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    Description.CPUAccessFlags = 0;
    Description.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA ResourceData = {};
    if (Data) {
        ResourceData.pSysMem = Data;
        ResourceData.SysMemPitch = 4 * Width;
        ResourceData.SysMemSlicePitch = 4 * Width * Height;
    }

    HRESULT hResult = DirectX.Device->CreateTexture2D(&Description, Data ? &ResourceData : NULL, &Texture);
    if (FAILED(hResult)) Raise("DirectX: Texture creation failed.");

    ID3D11ShaderResourceView* Result = NULL;
    D3D11_SHADER_RESOURCE_VIEW_DESC ResourceDescription = {};
    ResourceDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ResourceDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ResourceDescription.Texture2D.MostDetailedMip = 0;
    ResourceDescription.Texture2D.MipLevels = -1;
    DirectX.Device->CreateShaderResourceView(Texture, &ResourceDescription, &Result);

    return Result;
}

void CreateBuffer(
    ID3D11Buffer** Buffer, 
    uint32 Size, 
    uint32 BindFlags, 
    bool Dynamic = false,
    void* Data = NULL
) {
    D3D11_BUFFER_DESC BufferDescription;
    BufferDescription.Usage = Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    BufferDescription.ByteWidth = Size;
    BufferDescription.BindFlags = BindFlags;
    BufferDescription.CPUAccessFlags = Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
    BufferDescription.MiscFlags = 0;
    BufferDescription.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA ResourceData = {};
    if (Data) {
        ResourceData.pSysMem = Data;
        ResourceData.SysMemPitch = 0;
        ResourceData.SysMemSlicePitch = 0;
    }

    HRESULT Result = DirectX.Device->CreateBuffer(&BufferDescription, Data ? &ResourceData : NULL, Buffer);
    if (FAILED(Result)) {
        Raise("DirectX: Buffer creation failed.");
    }
}

#define CreateConstantBuffer(Type) CreateBuffer(&DirectX.ConstantBuffer[Type##_id], sizeof(Type), D3D11_BIND_CONSTANT_BUFFER, true)

void* GetMappedBuffer(ID3D11Buffer* DirectXBuffer) {
    D3D11_MAPPED_SUBRESOURCE Resource;
    HRESULT Result = DirectX.DeviceContext->Map(DirectXBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Buffer couldn't be mapped correctly.");
        return NULL;
    }
    else return Resource.pData;
}

void SetGlobalBuffer(int32 Width, int32 Height, camera* Camera, game_input* Input, float Time) {
    ID3D11Buffer* GlobalBuffer = DirectX.ConstantBuffer[global_buffer_id];
    void* MappedBuffer = GetMappedBuffer(GlobalBuffer);
    if (MappedBuffer) {
        global_buffer* Buffer = (global_buffer*)MappedBuffer;
        Buffer->Projection = GetWorldProjectionMatrix(Width, Height);
        Buffer->View = GetViewMatrix(Camera);
        Buffer->Resolution = V2(Width, Height);
        Buffer->Mouse = Input->Mouse.Cursor;
        Buffer->LastMouse = Input->Mouse.LastCursor;
        Buffer->Time = Time;
        DirectX.DeviceContext->Unmap(GlobalBuffer, 0);
        DirectX.DeviceContext->VSSetConstantBuffers(0, 1, &GlobalBuffer);
        DirectX.DeviceContext->HSSetConstantBuffers(0, 1, &GlobalBuffer);
        DirectX.DeviceContext->DSSetConstantBuffers(0, 1, &GlobalBuffer);
        DirectX.DeviceContext->GSSetConstantBuffers(0, 1, &GlobalBuffer);
        DirectX.DeviceContext->PSSetConstantBuffers(0, 1, &GlobalBuffer);
        DirectX.DeviceContext->CSSetConstantBuffers(0, 1, &GlobalBuffer);
    }
}

void SetLightBuffer(light Light, v3 CameraPosition) {
    ID3D11Buffer* TextBuffer = DirectX.ConstantBuffer[light_buffer_id];
    void* MappedBuffer = GetMappedBuffer(TextBuffer);
    if (MappedBuffer) {
        light_buffer* Buffer = (light_buffer*)MappedBuffer;
        Buffer->Ambient = Light.Ambient;
        Buffer->Diffuse = Light.Diffuse;
        Buffer->Direction = Light.Direction;
        Buffer->Color = V3(Light.Color.R, Light.Color.G, Light.Color.B);
        Buffer->CameraPosition = CameraPosition;
        DirectX.DeviceContext->Unmap(TextBuffer, 0);
        DirectX.DeviceContext->PSSetConstantBuffers(1, 1, &TextBuffer);
    }
}

void SetColorBuffer(color Color) {
    ID3D11Buffer* ColorBuffer = DirectX.ConstantBuffer[color_buffer_id];
    void* MappedBuffer = GetMappedBuffer(ColorBuffer);
    if (MappedBuffer) {
        color_buffer* Buffer = (color_buffer*)MappedBuffer;
        Buffer->Color = Color;
        DirectX.DeviceContext->Unmap(ColorBuffer, 0);
        DirectX.DeviceContext->PSSetConstantBuffers(2, 1, &ColorBuffer);
        DirectX.DeviceContext->CSSetConstantBuffers(2, 1, &ColorBuffer);
    }
}

void SetTransformBuffer(transform T = IdentityTransform) {
    matrix4 Model = Matrix(T);
    ID3D11Buffer* TransformBuffer = DirectX.ConstantBuffer[transform_buffer_id];
    void* MappedBuffer = GetMappedBuffer(TransformBuffer);
    if (MappedBuffer) {
        transform_buffer* Buffer = (transform_buffer*)MappedBuffer;
        Buffer->Model = Model;
        Buffer->Normal = Matrix4(inverse(Matrix3(Model)));
        DirectX.DeviceContext->Unmap(TransformBuffer, 0);
        DirectX.DeviceContext->VSSetConstantBuffers(3, 1, &TransformBuffer);
        DirectX.DeviceContext->DSSetConstantBuffers(3, 1, &TransformBuffer);
    }
}

void ClearTransformBuffer() {
    SetTransformBuffer();
}

void SetBoneBuffer(armature* Armature) {
    ID3D11Buffer* BoneBuffer = DirectX.ConstantBuffer[bone_buffer_id];
    void* MappedBuffer = GetMappedBuffer(BoneBuffer);
    if (MappedBuffer) {
        bone_buffer* Buffer = (bone_buffer*)MappedBuffer;
        Buffer->nBones = Armature->nBones;
        for (int i = 0; i < Armature->nBones; i++) {
            matrix4 BoneMatrix = Matrix(Armature->Bones[i].Transform);
            Buffer->BoneTransforms[i] = BoneMatrix;
            Buffer->BoneNormalTransforms[i] = Matrix4(inverse(Matrix3(BoneMatrix)));
        }
        DirectX.DeviceContext->Unmap(BoneBuffer, 0);
        DirectX.DeviceContext->VSSetConstantBuffers(4, 1, &BoneBuffer);
    }
}

void ClearBoneBuffer() {
    armature Armature = {};
    SetBoneBuffer(&Armature);
}

void SetTextBuffer(v2 Pen, float Size) {
    ID3D11Buffer* TextBuffer = DirectX.ConstantBuffer[text_outline_buffer_id];
    void* MappedBuffer = GetMappedBuffer(TextBuffer);
    if (MappedBuffer) {
        text_outline_buffer* Buffer = (text_outline_buffer*)MappedBuffer;
        Buffer->Pen = Pen;
        Buffer->Size = Size;
        DirectX.DeviceContext->Unmap(TextBuffer, 0);
        DirectX.DeviceContext->VSSetConstantBuffers(7, 1, &TextBuffer);
    }
}

void SetOutlineBuffer(float Width, int Level) {
    ID3D11Buffer* OutlineBuffer = DirectX.ConstantBuffer[outline_buffer_id];
    void* MappedBuffer = GetMappedBuffer(OutlineBuffer);
    if (MappedBuffer) {
        outline_buffer* Buffer = (outline_buffer*)MappedBuffer;
        Buffer->Width = Width;
        Buffer->Level = Level;
        DirectX.DeviceContext->Unmap(OutlineBuffer, 0);
        DirectX.DeviceContext->PSSetConstantBuffers(5, 1, &OutlineBuffer);
        DirectX.DeviceContext->CSSetConstantBuffers(5, 1, &OutlineBuffer);
    }
}

void CreateInputLayout(vertex_layout Layout, directX_Vertex_Shader_ID ShaderID) {
    D3D11_INPUT_ELEMENT_DESC LayoutDescription[MAX_VERTEX_ATTRIBUTES] = {};
    uint32 Offset = 0;
    for (int j = 0; j < Layout.nAttributes; j++) {
        vertex_type Type = Layout.Attributes[j].Type;
        const char* SemanticName = "";
        if      (j == 0) SemanticName = "POSITION"; 
        else if (j == 1) {
            if (Type == vertex_type_v4) 
                SemanticName = "COLOR";
            else SemanticName = "TEXCOORD";
        }
        else if (j == 2) SemanticName = "NORMAL";
        else if (j == 3) SemanticName = "BONEIDS";
        else if (j == 4) SemanticName = "BONEWEIGHTS";

        DXGI_FORMAT Format;
        int Size = GetVertexTypeSize(Type);
        if (IsFloatType(Type)) {
            switch (Size) {
                case 1: { Format = DXGI_FORMAT_R32_FLOAT; } break;
                case 2: { Format = DXGI_FORMAT_R32G32_FLOAT; } break;
                case 3: { Format = DXGI_FORMAT_R32G32B32_FLOAT; } break;
                case 4: { Format = DXGI_FORMAT_R32G32B32A32_FLOAT; } break;
                default: {
                    if (Size > 4) Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                    else Raise("DirectX: Invalid vertex type.");
                }
            }
        }
        else if (IsIntegerType(Type)) {
            switch (Size) {
                case 1: { Format = DXGI_FORMAT_R32_SINT; } break;
                case 2: { Format = DXGI_FORMAT_R32G32_SINT; } break;
                case 3: { Format = DXGI_FORMAT_R32G32B32_SINT; } break;
                case 4: { Format = DXGI_FORMAT_R32G32B32A32_SINT; } break;
                default: {
                    if (Size > 4) Format = DXGI_FORMAT_R32G32B32A32_SINT;
                    else Raise("DirectX: Invalid vertex type.");
                }
            }
        }
        else Raise("DirectX: Invalid vertex type.");

        LayoutDescription[j].SemanticName = SemanticName;
        LayoutDescription[j].SemanticIndex = 0;
        LayoutDescription[j].Format = Format;
        LayoutDescription[j].InputSlot = 0;
        LayoutDescription[j].AlignedByteOffset = Offset;
        LayoutDescription[j].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        LayoutDescription[j].InstanceDataStepRate = 0;

        Offset += GetVertexTypeSizeInBytes(Type);
    }

    HRESULT Result = DirectX.Device->CreateInputLayout(
        LayoutDescription,
        Layout.nAttributes,
        DirectX.VertexShader[ShaderID].Blob->GetBufferPointer(),
        DirectX.VertexShader[ShaderID].Blob->GetBufferSize(),
        &DirectX.VertexLayout[Layout.ID]
    );
}

void CreateInputLayout(instanced_layout_id LayoutID) {
    directX_Vertex_Shader_ID VertexShaderID;
    D3D11_INPUT_ELEMENT_DESC LayoutDescription[MAX_VERTEX_ATTRIBUTES];
    switch (LayoutID) {
        case instanced_layout_text_id: {
            VertexShaderID = Vertex_Shader_Barycentric_ID;

            // Per vertex
            LayoutDescription[0] = {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0};
            LayoutDescription[1] = {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 2*sizeof(float), D3D11_INPUT_PER_VERTEX_DATA, 0};

            // Per instance
            LayoutDescription[2] = {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1};
            LayoutDescription[3] = {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 3*sizeof(float), D3D11_INPUT_PER_INSTANCE_DATA, 1};
        } break;
        case instanced_layout_test_id: {
            VertexShaderID = Vertex_Shader_Test_ID;
        } break;
    }

    HRESULT Result = DirectX.Device->CreateInputLayout(
        LayoutDescription,
        4,
        DirectX.VertexShader[VertexShaderID].Blob->GetBufferPointer(),
        DirectX.VertexShader[VertexShaderID].Blob->GetBufferSize(),
        &DirectX.InstancedLayout[LayoutID]
    );

    if (FAILED(Result)) {
        Log(Error, "DirectX: Instanced vertex layout creation failed.");
    }
}

ID3DBlob* CompileShader(read_file_result* File, const char* Path) {
    read_file_result NewFile = Platform.ReadEntireFile(Path);

    if (NewFile.ContentSize == 0) return nullptr;

    *File = NewFile;

    const char* Extension = GetFileExtension(Path);
    const char* Target = nullptr;
    if      (strcmp(Extension, "vsh") == 0)     Target = "vs_5_0";
    else if (strcmp(Extension, "hsh") == 0)     Target = "hs_5_0";
    else if (strcmp(Extension, "dsh") == 0)     Target = "ds_5_0";
    else if (strcmp(Extension, "gsh") == 0)     Target = "gs_5_0";
    else if (strcmp(Extension, "psh") == 0)     Target = "ps_5_0";
    else if (strcmp(Extension, "compute") == 0) Target = "cs_5_0";
    else    Raise("DirectX: Invalid shader extension.");

    ID3DBlob* Errors = nullptr;
    ID3DBlob* Blob = nullptr;
    UINT Flags = D3D10_SHADER_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_DEBUG;
    HRESULT Result = D3DCompile(
        File->Content, 
        File->ContentSize, 
        File->Path, 
        NULL, 
        NULL, 
        "main",
        Target,
        Flags,
        0,
        &Blob,
        &Errors
    );

    if (FAILED(Result)) {
        char ErrorBuffer[4096];
        sprintf_s(ErrorBuffer, "DirectX: Failure trying to compile shader: %s", (char*)Errors->GetBufferPointer());
        Log(Error, ErrorBuffer);
        return nullptr;
    }

    return Blob;
}

void ParseVertexLayout(directX_Vertex_Shader* Shader) {
    vertex_layout ShaderLayout = {};
    token SemanticNames[MAX_VERTEX_ATTRIBUTES] = {};

    tokenizer Tokenizer = InitTokenizer(Shader->File.Content);
    token Token = GetToken(Tokenizer);
    while (Token.Type != Token_End) {
        if (Token == "struct") {
            Token = RequireToken(Tokenizer, Token_Identifier);
            if (Token == "VS_IN") {
                RequireToken(Tokenizer, Token_OpenBrace);
                
                Token = RequireToken(Tokenizer, Token_Identifier);
                while (Token.Type != Token_CloseBrace) {
                    vertex_type Type = vertex_type_empty;
                    if      (Token == "float")    Type = vertex_type_float;
                    else if (Token == "float2")   Type = vertex_type_v2;
                    else if (Token == "float3")   Type = vertex_type_v3;
                    else if (Token == "float4")   Type = vertex_type_v4;
                    else if (Token == "float2x2") Type = vertex_type_mat2;
                    else if (Token == "float3x3") Type = vertex_type_mat3;
                    else if (Token == "float4x4") Type = vertex_type_mat4;
                    else if (Token == "int")      Type = vertex_type_int;
                    else if (Token == "int2")     Type = vertex_type_iv2;
                    else if (Token == "int3")     Type = vertex_type_iv3;
                    else if (Token == "int4")     Type = vertex_type_iv4;
                    else Raise("DirectX: Unknown vertex attribute type.");
                    
                    RequireToken(Tokenizer, Token_Identifier);
                    RequireToken(Tokenizer, Token_Colon);
                    SemanticNames[ShaderLayout.nAttributes] = RequireToken(Tokenizer, Token_Identifier);
                    AddAttribute(&ShaderLayout, Type);
                    RequireToken(Tokenizer, Token_Semicolon);
                    Token = GetToken(Tokenizer);
                }

                vertex_layout_id LayoutID;
                bool Found = FindCompatibleVertexLayout(ShaderLayout, &LayoutID);
                if (Found && DirectX.VertexLayout[LayoutID] == NULL) {
                    CreateInputLayout(VertexLayouts[LayoutID], Shader->ID);
                }
                if (!Found) {
                    char ErrorBuffer[128];
                    sprintf_s(ErrorBuffer, "DirectX: Could not find compatible vertex layout for shader %s.", Shader->File.Path);
                    Log(Warn, ErrorBuffer);
                }
            }
        }

        Token = GetToken(Tokenizer);
    }
}

void LoadShader(directX_Vertex_Shader_ID ID, const char* Path) {
    directX_Vertex_Shader* Shader = &DirectX.VertexShader[ID];
    Shader->ID = ID;
    ID3DBlob* Blob = CompileShader(&Shader->File, Path);
    if (Blob) {
        Shader->Blob = Blob;
        HRESULT Result = DirectX.Device->CreateVertexShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &Shader->Shader
        );

        char TextBuffer[2048];
        if (FAILED(Result)) {
            sprintf_s(TextBuffer, "DirectX: There was an error creating vertex shader %s", Path);
            Log(Error, TextBuffer);
        }
        else {
            ParseVertexLayout(Shader);
        }
    }
}

void ParseSamplers(ID3D11SamplerState** Sampler, char* Code) {
    tokenizer Tokenizer = InitTokenizer(Code);
    token Token = GetToken(Tokenizer);
    while (Token.Type != Token_End) {
        if (Token == "SamplerState") {
            D3D11_SAMPLER_DESC SamplerDescription = {};
            SamplerDescription.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            SamplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            SamplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            SamplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            SamplerDescription.MipLODBias = 0.0f;
            SamplerDescription.MaxAnisotropy = 1;
            SamplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            SamplerDescription.BorderColor[0] = 0;
            SamplerDescription.BorderColor[1] = 0;
            SamplerDescription.BorderColor[2] = 0;
            SamplerDescription.BorderColor[3] = 0;
            SamplerDescription.MinLOD = 0;
            SamplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

            DirectX.Device->CreateSamplerState(&SamplerDescription, Sampler);
        }

        Token = GetToken(Tokenizer);
    }
}

void LoadShader(directX_Pixel_Shader_ID ID, const char* Path) {
    directX_Pixel_Shader* Shader = &DirectX.PixelShader[ID];
    Shader->ID = ID;
    ID3DBlob* Blob = CompileShader(&Shader->File, Path);
    if (Blob) {
        Shader->Blob = Blob;
        HRESULT Result = DirectX.Device->CreatePixelShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &Shader->Shader
        );

        char TextBuffer[2048];
        if (FAILED(Result)) {
            sprintf_s(TextBuffer, "DirectX: There was an error creating pixel shader %s", Path);
            Log(Error, TextBuffer);
        }
        else {
            ParseSamplers(&Shader->Sampler, (char*)Shader->File.Content);
        }
    }
}

void LoadShader(directX_Hull_Shader_ID ID, const char* Path) {
    directX_Hull_Shader* Shader = &DirectX.HullShader[ID];
    Shader->ID = ID;
    ID3DBlob* Blob = CompileShader(&Shader->File, Path);
    if (Blob) {
        Shader->Blob = Blob;
        HRESULT Result = DirectX.Device->CreateHullShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &Shader->Shader
        );

        char TextBuffer[2048];
        if (FAILED(Result)) {
            sprintf_s(TextBuffer, "DirectX: There was an error creating hull shader %s", Path);
            Log(Error, TextBuffer);
        }
    }
}

void LoadShader(directX_Domain_Shader_ID ID, const char* Path) {
    directX_Domain_Shader* Shader = &DirectX.DomainShader[ID];
    Shader->ID = ID;
    ID3DBlob* Blob = CompileShader(&Shader->File, Path);
    if (Blob) {
        Shader->Blob = Blob;
        HRESULT Result = DirectX.Device->CreateDomainShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &Shader->Shader
        );

        char TextBuffer[2048];
        if (FAILED(Result)) {
            sprintf_s(TextBuffer, "DirectX: There was an error creating domain shader %s", Path);
            Log(Error, TextBuffer);
        }
        else {
            ParseSamplers(&Shader->Sampler, (char*)Shader->File.Content);
        }
    }
}

void LoadShader(directX_Geometry_Shader_ID ID, const char* Path) {
    directX_Geometry_Shader* Shader = &DirectX.GeometryShader[ID];
    Shader->ID = ID;
    ID3DBlob* Blob = CompileShader(&Shader->File, Path);
    if (Blob) {
        Shader->Blob = Blob;
        HRESULT Result = DirectX.Device->CreateGeometryShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &Shader->Shader
        );

        char TextBuffer[2048];
        if (FAILED(Result)) {
            sprintf_s(TextBuffer, "DirectX: There was an error creating geometry shader %s", Path);
            Log(Error, TextBuffer);
        }
    }
}

void LoadShader(directX_Compute_Shader_ID ID, const char* Path) {
    directX_Compute_Shader* Shader = &DirectX.ComputeShader[ID];
    Shader->ID = ID;
    ID3DBlob* Blob = CompileShader(&Shader->File, Path);
    if (Blob) {
        Shader->Blob = Blob;
        HRESULT Result = DirectX.Device->CreateComputeShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &Shader->Shader
        );

        char TextBuffer[2048];
        if (FAILED(Result)) {
            sprintf_s(TextBuffer, "DirectX: There was an error creating compute shader %s", Path);
            Log(Error, TextBuffer);
        }
    }
}

void ReloadShader(directX_Vertex_Shader* Shader) {
    ID3DBlob* Blob = CompileShader(&Shader->File, Shader->File.Path);
    if (Blob) {
        ID3D11VertexShader* NewShader = nullptr;
        HRESULT Result = DirectX.Device->CreateVertexShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &NewShader
        );

        if (SUCCEEDED(Result)) {
            if (Shader->Shader) Shader->Shader->Release();
            if (Shader->Blob) Shader->Blob->Release();
            Shader->Blob = Blob;
            Shader->Shader = NewShader;

            ParseVertexLayout(Shader);

            char Text[512];
            sprintf_s(Text, "DirectX: Shader %s was successfully updated.", Shader->File.Path);
            Log(Info, Text);
        }
    }
}

void ReloadShader(directX_Hull_Shader* Shader) {
    ID3DBlob* Blob = CompileShader(&Shader->File, Shader->File.Path);
    if (Blob) {
        ID3D11HullShader* NewShader = nullptr;
        HRESULT Result = DirectX.Device->CreateHullShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &NewShader
        );

        if (SUCCEEDED(Result)) {
            if (Shader->Shader) Shader->Shader->Release();
            if (Shader->Blob) Shader->Blob->Release();
            Shader->Blob = Blob;
            Shader->Shader = NewShader;

            char Text[512];
            sprintf_s(Text, "DirectX: Shader %s was successfully updated.", Shader->File.Path);
            Log(Info, Text);
        }
    }
}

void ReloadShader(directX_Domain_Shader* Shader) {
    ID3DBlob* Blob = CompileShader(&Shader->File, Shader->File.Path);
    if (Blob) {
        ID3D11DomainShader* NewShader = nullptr;
        HRESULT Result = DirectX.Device->CreateDomainShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &NewShader
        );

        if (SUCCEEDED(Result)) {
            if (Shader->Shader) Shader->Shader->Release();
            if (Shader->Blob) Shader->Blob->Release();
            Shader->Blob = Blob;
            Shader->Shader = NewShader;

            char Text[512];
            sprintf_s(Text, "DirectX: Shader %s was successfully updated.", Shader->File.Path);
            Log(Info, Text);
        }
    }
}

void ReloadShader(directX_Geometry_Shader* Shader) {
    ID3DBlob* Blob = CompileShader(&Shader->File, Shader->File.Path);
    if (Blob) {
        ID3D11GeometryShader* NewShader = nullptr;
        HRESULT Result = DirectX.Device->CreateGeometryShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &NewShader
        );

        if (SUCCEEDED(Result)) {
            if (Shader->Shader) Shader->Shader->Release();
            if (Shader->Blob) Shader->Blob->Release();
            Shader->Blob = Blob;
            Shader->Shader = NewShader;

            char Text[512];
            sprintf_s(Text, "DirectX: Shader %s was successfully updated.", Shader->File.Path);
            Log(Info, Text);
        }
    }
}

void ReloadShader(directX_Pixel_Shader* Shader) {
    ID3DBlob* Blob = CompileShader(&Shader->File, Shader->File.Path);
    if (Blob) {
        ID3D11PixelShader* NewShader = nullptr;
        HRESULT Result = DirectX.Device->CreatePixelShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &NewShader
        );

        if (SUCCEEDED(Result)) {
            if (Shader->Shader) Shader->Shader->Release();
            if (Shader->Blob) Shader->Blob->Release();
            Shader->Blob = Blob;
            Shader->Shader = NewShader;

            ParseSamplers(&Shader->Sampler, (char*)Shader->File.Content);

            char Text[512];
            sprintf_s(Text, "DirectX: Shader %s was successfully updated.", Shader->File.Path);
            Log(Info, Text);
        }
    }
}

void ReloadShader(directX_Compute_Shader* Shader) {
    ID3DBlob* Blob = CompileShader(&Shader->File, Shader->File.Path);
    if (Blob) {
        ID3D11ComputeShader* NewShader = nullptr;
        HRESULT Result = DirectX.Device->CreateComputeShader(
            Blob->GetBufferPointer(),
            Blob->GetBufferSize(),
            NULL,
            &NewShader
        );

        if (SUCCEEDED(Result)) {
            if (Shader->Shader) Shader->Shader->Release();
            if (Shader->Blob) Shader->Blob->Release();
            Shader->Blob = Blob;
            Shader->Shader = NewShader;

            char Text[512];
            sprintf_s(Text, "DirectX: Shader %s was successfully updated.", Shader->File.Path);
            Log(Info, Text);
        }
    }
}

void ReloadShaders() {
    for (int i = 0; i < directX_Vertex_Shader_ID_count; i++) {
        directX_Vertex_Shader* Shader = &DirectX.VertexShader[i];
        int64 LastWriteTime = Platform.GetLastWriteTime(Shader->File.Path);
        if (LastWriteTime > Shader->File.Timestamp) {
            ReloadShader(Shader);
        }
    }

    for (int i = 0; i < directX_Hull_Shader_ID_count; i++) {
        directX_Hull_Shader* Shader = &DirectX.HullShader[i];
        int64 LastWriteTime = Platform.GetLastWriteTime(Shader->File.Path);
        if (LastWriteTime > Shader->File.Timestamp) {
            ReloadShader(Shader);
        }
    }

    for (int i = 0; i < directX_Domain_Shader_ID_count; i++) {
        directX_Domain_Shader* Shader = &DirectX.DomainShader[i];
        int64 LastWriteTime = Platform.GetLastWriteTime(Shader->File.Path);
        if (LastWriteTime > Shader->File.Timestamp) {
            ReloadShader(Shader);
        }
    }

    for (int i = 0; i < directX_Geometry_Shader_ID_count; i++) {
        directX_Geometry_Shader* Shader = &DirectX.GeometryShader[i];
        int64 LastWriteTime = Platform.GetLastWriteTime(Shader->File.Path);
        if (LastWriteTime > Shader->File.Timestamp) {
            ReloadShader(Shader);
        }
    }

    for (int i = 0; i < directX_Pixel_Shader_ID_count; i++) {
        directX_Pixel_Shader* Shader = &DirectX.PixelShader[i];
        int64 LastWriteTime = Platform.GetLastWriteTime(Shader->File.Path);
        if (LastWriteTime > Shader->File.Timestamp) {
            ReloadShader(Shader);
        }
    }

    for (int i = 0; i < directX_Compute_Shader_ID_count; i++) {
        directX_Compute_Shader* Shader = &DirectX.ComputeShader[i];
        int64 LastWriteTime = Platform.GetLastWriteTime(Shader->File.Path);
        if (LastWriteTime > Shader->File.Timestamp) {
            ReloadShader(Shader);
        }
    }
}

RENDERER_INITIALIZE {
    HRESULT Result;

    DirectX = {};
    DirectX.Assets = Group->Assets;

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
    UINT CreateDeviceFlags = 0;
#ifdef _DEBUG
    CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    Result = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        CreateDeviceFlags,
        &FeatureLevel,
        1,
        D3D11_SDK_VERSION,
        &SwapChainDescription,
        &DirectX.SwapChain,
        &DirectX.Device,
        NULL,
        &DirectX.DeviceContext
    );
    if (FAILED(Result)) Raise("Couldn't create DirectX11 device.");

    // Multisample support
    UINT QualityLevels = 0;
    DirectX.MSAASamples = 8;
    Result = DirectX.Device->CheckMultisampleQualityLevels(PixelFormat, DirectX.MSAASamples, &QualityLevels);
    if (FAILED(Result)) Raise("DirectX: Couldn't query MSAA support.");
    Assert(QualityLevels > 0);

    // Attaching backbuffer to swap chain
    ID3D11Texture2D** Backbuffer = &DirectX.Target[Target_None].Texture;
    Result = DirectX.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)Backbuffer);
    if (FAILED(Result)) Raise("Couldn't get DirectX backbuffer.");

    Result = DirectX.Device->CreateRenderTargetView(DirectX.Target[Target_None].Texture, NULL, &DirectX.Target[Target_None].View);
    if (FAILED(Result)) Raise("Couldn't create DirectX target view.");

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

    Result = DirectX.Device->CreateTexture2D(&DepthBufferDescription, NULL, &DirectX.Target[Target_None].AttachmentTexture);
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
	DepthStencilDescription.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	DepthStencilDescription.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	DepthStencilDescription.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    Result = DirectX.Device->CreateDepthStencilState(&DepthStencilDescription, &DirectX.DepthStencilEnabled);
    DepthStencilDescription.DepthEnable = false;
    Result = DirectX.Device->CreateDepthStencilState(&DepthStencilDescription, &DirectX.DepthStencilDisabled);
    DirectX.DeviceContext->OMSetDepthStencilState(DirectX.DepthStencilDisabled, 1);

    D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDescription = {};
    DepthStencilViewDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    DepthStencilViewDescription.Texture2D.MipSlice = 0;

    Result = DirectX.Device->CreateDepthStencilView(DirectX.Target[Target_None].AttachmentTexture, &DepthStencilViewDescription, &DirectX.Target[Target_None].Attachment);
    if (FAILED(Result)) Raise("Couldn't create DirectX depth stencil view.");

    D3D11_RASTERIZER_DESC RasterizerDescription;
    RasterizerDescription.AntialiasedLineEnable = true;
	RasterizerDescription.CullMode = D3D11_CULL_NONE;
	RasterizerDescription.DepthBias = 0;
	RasterizerDescription.DepthBiasClamp = 0.0f;
	RasterizerDescription.DepthClipEnable = true;
	RasterizerDescription.FillMode = D3D11_FILL_SOLID;
	RasterizerDescription.FrontCounterClockwise = true;
	RasterizerDescription.MultisampleEnable = true;
	RasterizerDescription.ScissorEnable = false;
	RasterizerDescription.SlopeScaledDepthBias = 0.0f;

    Result = DirectX.Device->CreateRasterizerState(&RasterizerDescription, &DirectX.RasterizerState);
    if (FAILED(Result)) Raise("Couldn't create DirectX rasterizer state.");
    DirectX.DeviceContext->RSSetState(DirectX.RasterizerState);

    DirectX.Viewport.Width = Group->Width;
    DirectX.Viewport.Height = Group->Height;
    DirectX.Viewport.MinDepth = 0.0f;
    DirectX.Viewport.MaxDepth = 1.0f;
    DirectX.Viewport.TopLeftX = 0.0f;
    DirectX.Viewport.TopLeftY = 0.0f;
    DirectX.DeviceContext->RSSetViewports(1, &DirectX.Viewport);

    D3D11_RENDER_TARGET_BLEND_DESC RenderTargetBlendDescription = {};
    RenderTargetBlendDescription.BlendEnable = true;
    RenderTargetBlendDescription.BlendOp = D3D11_BLEND_OP_ADD;
    RenderTargetBlendDescription.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    RenderTargetBlendDescription.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    RenderTargetBlendDescription.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    RenderTargetBlendDescription.SrcBlendAlpha = D3D11_BLEND_ONE;
    RenderTargetBlendDescription.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    RenderTargetBlendDescription.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC BlendDescription = {};
    BlendDescription.AlphaToCoverageEnable = false;
    BlendDescription.IndependentBlendEnable = false;
    BlendDescription.RenderTarget[0] = RenderTargetBlendDescription;

    Result = DirectX.Device->CreateBlendState(&BlendDescription, &DirectX.CombineAlpha);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Blend state creation failed.");
    }

    RenderTargetBlendDescription.SrcBlend = D3D11_BLEND_ONE;
    RenderTargetBlendDescription.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    RenderTargetBlendDescription.SrcBlendAlpha = D3D11_BLEND_ONE;
    RenderTargetBlendDescription.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    BlendDescription.RenderTarget[0] = RenderTargetBlendDescription;

    Result = DirectX.Device->CreateBlendState(&BlendDescription, &DirectX.TargetBlend);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Blend state creation failed.");
    }

    RenderTargetBlendDescription.SrcBlend = D3D11_BLEND_ONE;
    RenderTargetBlendDescription.DestBlend = D3D11_BLEND_ZERO;
    RenderTargetBlendDescription.SrcBlendAlpha = D3D11_BLEND_ONE;
    RenderTargetBlendDescription.DestBlendAlpha = D3D11_BLEND_ZERO;
    BlendDescription.RenderTarget[0] = RenderTargetBlendDescription;

    Result = DirectX.Device->CreateBlendState(&BlendDescription, &DirectX.OverwriteAlpha);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Blend state creation failed.");
    }

    DirectX.Initialized = true;
    Log(Info, "Direct3D 11 was successfully initialized.");

// Render targets
    for (int i = 1; i < render_group_target_count; i++) {
        CreateTarget(Group->Width, Group->Height, Group->RenderTargets[i]);
    }

// Textures
    DirectX.StagingTexture = NULL;

    D3D11_TEXTURE2D_DESC StagingDescription;
    DirectX.Target[Target_None].Texture->GetDesc(&StagingDescription);

    StagingDescription.Usage = D3D11_USAGE_STAGING;
    StagingDescription.BindFlags = 0;
    StagingDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    StagingDescription.MiscFlags = 0;

    Result = DirectX.Device->CreateTexture2D(&StagingDescription, 0, &DirectX.StagingTexture);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Staging texture creation failed.");
    }

    for (int i = 0; i < game_bitmap_id_count; i++) {
        game_bitmap* Bitmap = &Group->Assets->Bitmap[i];
        DirectX.Texture[i] = CreateTexture(
            Bitmap->Header.Width, Bitmap->Header.Height,
            Bitmap->Content
        );
    }

    ID3D11Texture2D* FFTTexture = NULL;

    D3D11_TEXTURE2D_DESC Description = {};
    Description.Width = 1024;
    Description.Height = 1024;
    Description.MipLevels = 1;
    Description.ArraySize = 1;
    Description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    Description.SampleDesc.Count = 1;
    Description.SampleDesc.Quality = 0;
    Description.Usage = D3D11_USAGE_DEFAULT;
    Description.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    Description.CPUAccessFlags = 0;
    Description.MiscFlags = 0;

    Result = DirectX.Device->CreateTexture2D(&StagingDescription, 0, &DirectX.StagingTexture);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Staging texture creation failed.");
    }

// Heightmaps
    for (int i = 0; i < game_heightmap_id_count; i++) {
        game_bitmap* Bitmap = &Group->Assets->Heightmap[i].Bitmap;
        DirectX.Heightmap[i] = CreateTexture(
            Bitmap->Header.Width, Bitmap->Header.Height,
            Bitmap->Content
        );
    }

// Shaders
    // Vertex
    LoadShader(Vertex_Shader_Passthrough_ID,         "GameAssets\\Shaders\\HLSL\\Vertex\\Passthrough.vsh");
    LoadShader(Vertex_Shader_Screen_ID,              "GameAssets\\Shaders\\HLSL\\Vertex\\Screen.vsh");
    LoadShader(Vertex_Shader_Screen_Texture_ID,      "GameAssets\\Shaders\\HLSL\\Vertex\\ScreenTexture.vsh");
    LoadShader(Vertex_Shader_Perspective_ID,         "GameAssets\\Shaders\\HLSL\\Vertex\\Perspective.vsh");
    LoadShader(Vertex_Shader_Perspective_Texture_ID, "GameAssets\\Shaders\\HLSL\\Vertex\\PerspectiveTexture.vsh");
    LoadShader(Vertex_Shader_Mesh_ID,                "GameAssets\\Shaders\\HLSL\\Vertex\\Mesh.vsh");
    LoadShader(Vertex_Shader_Bones_ID,               "GameAssets\\Shaders\\HLSL\\Vertex\\Bones.vsh");
    LoadShader(Vertex_Shader_Barycentric_ID,         "GameAssets\\Shaders\\HLSL\\Vertex\\Barycentric.vsh");
    LoadShader(Vertex_Shader_Heightmap_ID,           "GameAssets\\Shaders\\HLSL\\Vertex\\Heightmap.vsh");
    LoadShader(Vertex_Shader_Sky_ID,                 "GameAssets\\Shaders\\HLSL\\Vertex\\Sky.vsh");
    LoadShader(Vertex_Shader_Test_ID,                "GameAssets\\Shaders\\HLSL\\Vertex\\Test.vsh");

    // Hull
    LoadShader(Hull_Shader_Heightmap_ID,             "GameAssets\\Shaders\\HLSL\\Hull\\Heightmap.hsh");

    // Domain
    LoadShader(Domain_Shader_Heightmap_ID,           "GameAssets\\Shaders\\HLSL\\Domain\\Heightmap.dsh");
    LoadShader(Domain_Shader_Water_ID,               "GameAssets\\Shaders\\HLSL\\Domain\\Water.dsh");

    // Geometry
    LoadShader(Geometry_Shader_Normal_ID,            "GameAssets\\Shaders\\HLSL\\Geometry\\Normal.gsh");
    
    // Pixel
    LoadShader(Pixel_Shader_Antialiasing_ID,         "GameAssets\\Shaders\\HLSL\\Pixel\\Antialiasing.psh");
    LoadShader(Pixel_Shader_Single_Color_ID,         "GameAssets\\Shaders\\HLSL\\Pixel\\SingleColor.psh");
    LoadShader(Pixel_Shader_Attrib_Color_ID,         "GameAssets\\Shaders\\HLSL\\Pixel\\AttribColor.psh");
    LoadShader(Pixel_Shader_Texture_ID,              "GameAssets\\Shaders\\HLSL\\Pixel\\Texture.psh");
    LoadShader(Pixel_Shader_Mesh_ID,                 "GameAssets\\Shaders\\HLSL\\Pixel\\Mesh.psh");
    LoadShader(Pixel_Shader_Bezier_Exterior_ID,      "GameAssets\\Shaders\\HLSL\\Pixel\\BezierExterior.psh");
    LoadShader(Pixel_Shader_Bezier_Interior_ID,      "GameAssets\\Shaders\\HLSL\\Pixel\\BezierInterior.psh");
    LoadShader(Pixel_Shader_Heightmap_ID,            "GameAssets\\Shaders\\HLSL\\Pixel\\Heightmap.psh");
    LoadShader(Pixel_Shader_Sky_ID,                  "GameAssets\\Shaders\\HLSL\\Pixel\\Sky.psh");
    LoadShader(Pixel_Shader_Water_ID,               "GameAssets\\Shaders\\HLSL\\Pixel\\Water.psh");

    // Compute
    LoadShader(Compute_Shader_Outline_Init_ID,       "GameAssets\\Shaders\\HLSL\\Compute\\OutlineInit.compute");
    LoadShader(Compute_Shader_Jump_Flood_ID,         "GameAssets\\Shaders\\HLSL\\Compute\\JumpFlood.compute");
    LoadShader(Compute_Shader_Outline_ID,            "GameAssets\\Shaders\\HLSL\\Compute\\Outline.compute");

// Vertex buffers
    // Layout buffers
    for (int i = 0; i < vertex_layout_id_count; i++) {
        CreateBuffer(&DirectX.VertexBuffer[i], VERTEX_BUFFER_SIZE, D3D11_BIND_VERTEX_BUFFER, true);
    }
    CreateBuffer(&DirectX.IndexBuffer, ELEMENT_BUFFER_SIZE, D3D11_BIND_INDEX_BUFFER, true);

    for (int i = 0; i < instanced_layout_id_count; i++) {
        CreateInputLayout((instanced_layout_id)i);
    }

    // Mesh buffers
    for (int i = 0; i < game_mesh_id_count; i++) {
        game_mesh* Mesh = &Group->Assets->Mesh[i];

        directX_mesh_buffer* Buffer = &DirectX.MeshBuffer[i];
        uint64 VerticesSize = GetMeshVerticesSize(Mesh->nVertices, Mesh->Armature.nBones > 0);
        uint64 ElementsSize = (3 * Mesh->nFaces + 2 * Mesh->nEdges) * sizeof(uint32);
        CreateBuffer(&Buffer->VertexBuffer, VerticesSize, D3D11_BIND_VERTEX_BUFFER, false, Mesh->Vertices);
        CreateBuffer(&Buffer->IndexBuffer, ElementsSize, D3D11_BIND_INDEX_BUFFER, false, Mesh->nEdges > 0 ? Mesh->Edges : Mesh->Faces);
    }

    // Font buffers
    for (int i = 0; i < game_font_id_count; i++) {
        game_font* Font = &Group->Assets->Font[i];

        directX_font_buffer* FontBuffer = &DirectX.FontBuffer[i];
        uint64 VerticesSize = 4 * sizeof(float) * 3 * Font->nOnCurve;
        uint64 ElementsSize = 3 * sizeof(uint32) * (Font->nPoints - Font->nOnCurve);
        for (int j = 0; j < FONT_CHARACTERS_COUNT; j++) {
            game_font_character* Character = &Font->Characters[j];
            ElementsSize += 3 * sizeof(uint32) * Character->nSolidTriangles;
        }
        CreateBuffer(&FontBuffer->VertexBuffer, VerticesSize, D3D11_BIND_VERTEX_BUFFER, false, Font->Vertices);
        CreateBuffer(&FontBuffer->IndexBuffer, ElementsSize, D3D11_BIND_INDEX_BUFFER, false, Font->Elements);
        CreateBuffer(&FontBuffer->InstanceBuffer, TEXT_BUFFER_SIZE, D3D11_BIND_VERTEX_BUFFER, true);
    }

    // Heightmap buffer
    const int nVertices = HEIGHTMAP_RESOLUTION*HEIGHTMAP_RESOLUTION;
    float Vertices[2*nVertices];
    GenerateHeightmapVertices(Vertices);
    CreateBuffer(&DirectX.HeightmapBuffer.VertexBuffer, 2*nVertices*sizeof(float), D3D11_BIND_VERTEX_BUFFER, false, Vertices);

    const int nElements = 4 * (HEIGHTMAP_RESOLUTION - 1) * (HEIGHTMAP_RESOLUTION - 1);
    uint32 Elements[nElements];
    GenerateHeightmapElements(Elements);
    CreateBuffer(&DirectX.HeightmapBuffer.IndexBuffer, nElements*sizeof(uint32), D3D11_BIND_INDEX_BUFFER, false, Elements);

    // Constant buffers
    CreateConstantBuffer(global_buffer);
    CreateConstantBuffer(color_buffer);
    CreateConstantBuffer(transform_buffer);
    CreateConstantBuffer(bone_buffer);
    CreateConstantBuffer(text_outline_buffer);
    CreateConstantBuffer(light_buffer);
    CreateConstantBuffer(outline_buffer);
}

D3D_PRIMITIVE_TOPOLOGY GetRenderPrimitive(render_primitive Primitive) {
    switch(Primitive) {
        case render_primitive_point:          return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        case render_primitive_line:           return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        case render_primitive_line_strip:     return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case render_primitive_triangle:       return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case render_primitive_triangle_strip: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case render_primitive_patches:        return D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
        default: Raise("DirectX: Invalid render primitive.");
    }
    return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
}

void ResizeWindow(int32 Width, int32 Height) {
    HRESULT Result;
    DirectX.DeviceContext->OMSetRenderTargets(0, NULL, NULL);

    // Resize staging texture for screen capture
    DirectX.StagingTexture->Release();
    D3D11_TEXTURE2D_DESC StagingDescription;
    DirectX.Target[Target_None].Texture->GetDesc(&StagingDescription);

    StagingDescription.Width = Width;
    StagingDescription.Height = Height;
    StagingDescription.Usage = D3D11_USAGE_STAGING;
    StagingDescription.BindFlags = 0;
    StagingDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    StagingDescription.MiscFlags = 0;

    Result = DirectX.Device->CreateTexture2D(&StagingDescription, 0, &DirectX.StagingTexture);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Staging texture creation failed.");
    }

    // Resize back buffer and corresponding depth/stencil buffer
    DirectX.Target[Target_None].View->Release();
    DirectX.Target[Target_None].Texture->Release();
    DirectX.Target[Target_None].Attachment->Release();
    DirectX.Target[Target_None].AttachmentTexture->Release();
    Result = DirectX.SwapChain->ResizeBuffers(2, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Swap chain buffers resizing failed.");
    }
    else {
        Result = DirectX.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&DirectX.Target[Target_None].Texture);
        if (FAILED(Result)) Log(Error, "DirectX: Swap chain texture fetching failed.");
        else 
            Result = DirectX.Device->CreateRenderTargetView(DirectX.Target[Target_None].Texture, NULL, &DirectX.Target[Target_None].View);
        
        D3D11_TEXTURE2D_DESC DepthBufferDescription = {};
        DepthBufferDescription.Width = Width;
        DepthBufferDescription.Height = Height;
        DepthBufferDescription.MipLevels = 1;
        DepthBufferDescription.ArraySize = 1;
        DepthBufferDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthBufferDescription.SampleDesc.Count = 1;
        DepthBufferDescription.SampleDesc.Quality = 0;
        DepthBufferDescription.Usage = D3D11_USAGE_DEFAULT;
        DepthBufferDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        DepthBufferDescription.CPUAccessFlags = 0;
        DepthBufferDescription.MiscFlags = 0;

        Result = DirectX.Device->CreateTexture2D(&DepthBufferDescription, NULL, &DirectX.Target[Target_None].AttachmentTexture);
        if (FAILED(Result)) Raise("DirectX: Depth/stencil buffer resize failed.");
        else {
            D3D11_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDescription = {};
            DepthStencilViewDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            DepthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            DepthStencilViewDescription.Texture2D.MipSlice = 0;

            Result = DirectX.Device->CreateDepthStencilView(
                DirectX.Target[Target_None].AttachmentTexture, 
                &DepthStencilViewDescription, 
                &DirectX.Target[Target_None].Attachment
            );
            if (FAILED(Result)) Raise("Couldn't create DirectX depth stencil view.");
        }
    }

    // Resize render targets
    for (int i = 1; i < render_group_target_count; i++) {
        directX_render_target* Target = &DirectX.Target[i];
        Target->View->Release();
        Target->Texture->Release();
        if (Target->Description.Depth || Target->Description.Stencil) {
            Target->Attachment->Release();
            Target->AttachmentTexture->Release();
        }
        Target->ShaderTexture->Release();
        CreateTarget(Width, Height, Target->Description);
    }

    // Resize viewport
    DirectX.Viewport.Width = Width;
    DirectX.Viewport.Height = Height;
    DirectX.DeviceContext->RSSetViewports(1, &DirectX.Viewport);
}

void ScreenCapture(int32 Width, int32 Height) {
    ID3D11Texture2D* Backbuffer = DirectX.Target[Target_None].Texture;
    DirectX.DeviceContext->CopyResource(DirectX.StagingTexture, Backbuffer);

    D3D11_TEXTURE2D_DESC Desc;
    DirectX.StagingTexture->GetDesc(&Desc);

    D3D11_MAPPED_SUBRESOURCE MapInfo;
    HRESULT Result = DirectX.DeviceContext->Map(DirectX.StagingTexture, 0, D3D11_MAP_READ, 0, &MapInfo);
    if (FAILED(Result)) {
        Log(Error, "DirectX: Screen capture failed");
        return;
    }

    // Bitmap header
    bitmap_header Header = {};
    MakeBitmapHeader(&Header, Width, Height);

    Header.RedMask = 0x000000ff;
    Header.GreenMask = 0x0000ff00;
    Header.BlueMask = 0x00ff0000;

    // File name
    time_t t = time(nullptr);
    struct tm tm;
    localtime_s(&tm, &t);
    char Filename[100];
    sprintf_s(Filename, "Captures/Screenshot %d-%02d-%02d_%02d-%02d-%02d.bmp",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday,
        tm.tm_hour,
        tm.tm_min,
        tm.tm_sec
    );

    HANDLE hFile = CreateFileA(Filename, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, NULL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        uint32 Size = Header.BitmapOffset + 4 * Width * Height;
        HANDLE hMapping = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, Size, NULL);
        if (!hMapping) {
            DWORD WinError = GetLastError();
            Log(Error, "Memory map for file returned invalid handle.");
            Assert(false);
        }

        uint8* Memory = (uint8*)MapViewOfFile(hMapping, FILE_MAP_WRITE, 0, 0, Size);
        memcpy(Memory, &Header, sizeof(bitmap_header));

        uint8* PixelDst = Memory + Header.BitmapOffset;
        uint8* Source = (uint8*)MapInfo.pData + (Height - 1)*MapInfo.RowPitch;
        for (int i = 0; i < Height; i++) {
            memcpy(PixelDst, Source, 4*Width);
            Source -= MapInfo.RowPitch;
            PixelDst += 4*Width;
        }
        
        FlushViewOfFile(Memory, Size);
        UnmapViewOfFile(Memory);
        CloseHandle(hMapping);
        CloseHandle(hFile);
    }
    else {
        // Debug
        DWORD WinError = GetLastError();
        if (WinError == ERROR_PATH_NOT_FOUND) {
            Log(Error, "Path not found.");
        }
        Assert(false);
    }

    DirectX.DeviceContext->Unmap(DirectX.StagingTexture, 0);
}

RENDERER_RENDER {
    TIMED_BLOCK;

    // Vertex buffers
    for (int i = 0; i < vertex_layout_id_count; i++) {
        ID3D11Buffer* VertexBuffer = DirectX.VertexBuffer[i];
        void* MappedBuffer = GetMappedBuffer(VertexBuffer);
        memcpy(MappedBuffer, Group->VertexBuffer.Vertices[i].Base, Group->VertexBuffer.Vertices[i].Used);
        DirectX.DeviceContext->Unmap(VertexBuffer, 0);
    }
    ID3D11Buffer* IndexBuffer = DirectX.IndexBuffer;
    void* MappedBuffer = GetMappedBuffer(IndexBuffer);
    memcpy(MappedBuffer, Group->VertexBuffer.Elements.Base, Group->VertexBuffer.Elements.Used);
    DirectX.DeviceContext->Unmap(IndexBuffer, 0);

    // Constant buffers
    SetGlobalBuffer(Group->Width, Group->Height, Camera, Input, Time);
    SetLightBuffer(Group->Light, Camera->Position + Camera->Distance * Camera->Basis.Z);
    ClearTransformBuffer();
    ClearBoneBuffer();

    // Render entries
	for (int i = 0; i < Group->EntryCount; i++) {
		render_command Command = Group->Entries[i];

		switch(Command.Type) {
			case render_clear: {
				render_clear_command Clear = Group->ClearCommands[Command.Index];

                directX_render_target* Target = &DirectX.Target[Clear.Target];
                float Color[4] = { Clear.Color.R, Clear.Color.G, Clear.Color.B, Clear.Color.Alpha };
                BindTarget(Clear.Target);
                
                DirectX.DeviceContext->ClearRenderTargetView(Target->View, Color);
                if (Target->Attachment)
                    DirectX.DeviceContext->ClearDepthStencilView(Target->Attachment, D3D11_CLEAR_DEPTH, 1.0f, 0);
			} break;

            case render_draw_primitive: {
                render_primitive_command PrimitiveCommand = Group->PrimitiveCommands[Command.Index];
                render_primitive_options Options = PrimitiveCommand.Options;
                vertex_buffer_entry VertexEntry = PrimitiveCommand.VertexEntry;
                element_buffer_entry ElementEntry = PrimitiveCommand.ElementEntry;
                instance_buffer_entry InstanceEntry = PrimitiveCommand.InstanceEntry;

                float BlendFactors[4] = {};
                if (Options.Flags & OVERWRITE_ALPHA_FLAG) 
                    DirectX.DeviceContext->OMSetBlendState(DirectX.OverwriteAlpha, BlendFactors, 0xffffffff);
                else
                    DirectX.DeviceContext->OMSetBlendState(DirectX.CombineAlpha, BlendFactors, 0xffffffff);

                BindTarget(Target_World);

                vertex_layout_id LayoutID;
                ID3D11Buffer** VertexBuffer = NULL;
                ID3D11Buffer* IndexBuffer = NULL;
                ID3D11Buffer* InstanceBuffer = NULL;
                directX_Vertex_Shader_ID VertexShaderID = Vertex_Shader_Screen_ID;
                directX_Pixel_Shader_ID PixelShaderID = Pixel_Shader_Single_Color_ID;
                uint32 Offset = 0;
                if (Options.Heightmap) {
                    LayoutID = vertex_layout_v2_id;
                    VertexShaderID = Vertex_Shader_Heightmap_ID;
                    PixelShaderID = Pixel_Shader_Heightmap_ID;
                    VertexBuffer = &DirectX.HeightmapBuffer.VertexBuffer;
                    IndexBuffer = DirectX.HeightmapBuffer.IndexBuffer;
                    DirectX.DeviceContext->HSSetShader(DirectX.HullShader[Hull_Shader_Heightmap_ID].Shader, NULL, 0);
                    DirectX.DeviceContext->DSSetShader(DirectX.DomainShader[Domain_Shader_Heightmap_ID].Shader, NULL, 0);

                    SetTransformBuffer(Options.Transform);
                }
                else if (Options.Flags & WATER_FLAG) {
                    LayoutID = vertex_layout_v2_id;
                    VertexShaderID = Vertex_Shader_Heightmap_ID;
                    PixelShaderID = Pixel_Shader_Water_ID;
                    VertexBuffer = &DirectX.HeightmapBuffer.VertexBuffer;
                    IndexBuffer = DirectX.HeightmapBuffer.IndexBuffer;
                    DirectX.DeviceContext->HSSetShader(DirectX.HullShader[Hull_Shader_Heightmap_ID].Shader, NULL, 0);
                    DirectX.DeviceContext->DSSetShader(DirectX.DomainShader[Domain_Shader_Water_ID].Shader, NULL, 0);

                    SetTransformBuffer(Options.Transform);
                }
                else if (Options.Flags & DEBUG_BONES_FLAG) {
                    LayoutID = vertex_layout_v3_id;
                    VertexShaderID = Vertex_Shader_Perspective_ID;
                    PixelShaderID = Pixel_Shader_Single_Color_ID;
                    VertexBuffer = &DirectX.VertexBuffer[LayoutID];
                }
                else {
                    LayoutID = VertexEntry.LayoutID;
                    VertexBuffer = &DirectX.VertexBuffer[LayoutID];
                    Offset = VertexEntry.Offset;

                    if (ElementEntry.Count > 0) {
                        IndexBuffer = DirectX.IndexBuffer;
                        Offset = ElementEntry.Offset;
                    }

                    if (InstanceEntry.Count > 0) {
                        InstanceBuffer = DirectX.VertexBuffer[InstanceEntry.LayoutID];
                    }

                    VertexShaderID = Options.Flags & DEPTH_TEST_FLAG ? Vertex_Shader_Perspective_ID : Vertex_Shader_Screen_ID;
                    
                    if (Options.Texture) {
                        VertexShaderID = Vertex_Shader_Screen_Texture_ID;
                        PixelShaderID = Pixel_Shader_Texture_ID;
                    }
                    else if (Options.Flags & SKY_FLAG) {
                        VertexShaderID = Vertex_Shader_Sky_ID;
                        PixelShaderID = Pixel_Shader_Sky_ID;
                    }
                }

                SetColorBuffer(PrimitiveCommand.Color);

                DirectX.DeviceContext->IASetInputLayout(DirectX.VertexLayout[LayoutID]);
                DirectX.DeviceContext->VSSetShader(DirectX.VertexShader[VertexShaderID].Shader, NULL, 0);
                DirectX.DeviceContext->PSSetShader(DirectX.PixelShader[PixelShaderID].Shader, NULL, 0);
            
                uint32 Stride = VertexLayouts[LayoutID].Stride;
                uint32 VertexOffset = 0;
                DirectX.DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer, &Stride, &VertexOffset);
                DirectX.DeviceContext->IASetPrimitiveTopology(GetRenderPrimitive(PrimitiveCommand.Primitive));

                if (Options.Texture) {
                    DirectX.DeviceContext->PSSetShaderResources(0, 1, &DirectX.Texture[Options.Texture->ID]);
                    DirectX.DeviceContext->PSSetSamplers(0, 1, &DirectX.PixelShader[PixelShaderID].Sampler);
                }

                if (Options.Heightmap) {
                    DirectX.DeviceContext->DSSetShaderResources(0, 1, &DirectX.Heightmap[Options.Heightmap->ID]);
                    DirectX.DeviceContext->DSSetSamplers(0, 1, &DirectX.DomainShader[Domain_Shader_Heightmap_ID].Sampler);
                }

                if (Options.Flags & DEPTH_TEST_FLAG) {
                    DirectX.DeviceContext->OMSetDepthStencilState(DirectX.DepthStencilEnabled, 1);
                }
                
                if (ElementEntry.Count > 0) {
                    DirectX.DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
                    DirectX.DeviceContext->DrawIndexed(ElementEntry.Count, Offset, 0);
                }
                else {
                    DirectX.DeviceContext->Draw(VertexEntry.Count, Offset);
                }

                if (Options.Heightmap || Options.Flags & WATER_FLAG) {
                    DirectX.DeviceContext->HSSetShader(NULL, NULL, 0);
                    DirectX.DeviceContext->DSSetShader(NULL, NULL, 0);
                    ClearTransformBuffer();
                }

                DirectX.DeviceContext->OMSetDepthStencilState(DirectX.DepthStencilDisabled, 1);
            } break;

            case render_mesh: {
                render_mesh_command MeshCommand = Group->MeshCommands[Command.Index];
                render_mesh_options Options = MeshCommand.Options;

                game_mesh* Mesh = GetAsset(Group->Assets, MeshCommand.MeshID);

                ID3D11Buffer** VertexBuffer = &DirectX.MeshBuffer[MeshCommand.MeshID].VertexBuffer;
                ID3D11Buffer* IndexBuffer = DirectX.MeshBuffer[MeshCommand.MeshID].IndexBuffer;
                vertex_layout_id LayoutID = vertex_layout_v3_v2_v3_id;
                directX_Vertex_Shader_ID VertexShaderID = Vertex_Shader_Mesh_ID;
                directX_Pixel_Shader_ID PixelShaderID = Pixel_Shader_Mesh_ID;
                if (Mesh->Armature.nBones > 0) {
                    LayoutID = vertex_layout_bones_id;
                    VertexShaderID = Vertex_Shader_Bones_ID;

                    if (Options.Armature) {
                        SetBoneBuffer(Options.Armature);
                    }
                }

                if (Options.Outline) {
                    BindTarget(Target_Outline);
                    PixelShaderID = Pixel_Shader_Single_Color_ID;
                }
                else BindTarget(Target_World);
                
                SetTransformBuffer(Options.Transform);
                SetColorBuffer(Options.Color);

                DirectX.DeviceContext->OMSetDepthStencilState(DirectX.DepthStencilEnabled, 1);

                DirectX.DeviceContext->PSSetShaderResources(0, 1, &DirectX.Texture[Options.TextureID]);
                DirectX.DeviceContext->PSSetSamplers(0, 1, &DirectX.PixelShader[PixelShaderID].Sampler);

                DirectX.DeviceContext->IASetInputLayout(DirectX.VertexLayout[LayoutID]);
                DirectX.DeviceContext->VSSetShader(DirectX.VertexShader[VertexShaderID].Shader, NULL, 0);
                DirectX.DeviceContext->PSSetShader(DirectX.PixelShader[PixelShaderID].Shader, NULL, 0);
            
                uint32 Stride = VertexLayouts[LayoutID].Stride;

                uint32 VertexOffset = 0;
                if (Mesh->nFaces > 0) {
                    DirectX.DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer, &Stride, &VertexOffset);
                    DirectX.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    DirectX.DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
                    DirectX.DeviceContext->DrawIndexed(3*Mesh->nFaces, 0, 0);
                }

                if (Mesh->nEdges > 0) {
                    DirectX.DeviceContext->IASetVertexBuffers(0, 1, VertexBuffer, &Stride, &VertexOffset);
                    DirectX.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
                    DirectX.DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
                    DirectX.DeviceContext->DrawIndexed(2*Mesh->nEdges, 3*Mesh->nFaces, 0);
                }

                if (!Options.Outline && Group->Debug && Group->DebugNormals) {
                    DirectX.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
                    DirectX.DeviceContext->GSSetShader(DirectX.GeometryShader[Geometry_Shader_Normal_ID].Shader, NULL, 0);
                    DirectX.DeviceContext->PSSetShader(DirectX.PixelShader[Pixel_Shader_Single_Color_ID].Shader, NULL, 0);
                    SetColorBuffer(Yellow);
                    DirectX.DeviceContext->Draw(Mesh->nVertices, 0);
                    DirectX.DeviceContext->GSSetShader(NULL, NULL, 0);
                }

                if (Options.Armature) ClearBoneBuffer();
                ClearTransformBuffer();
            } break;

            case render_text: {
                BindTarget(Target_World);

                DirectX.DeviceContext->OMSetDepthStencilState(DirectX.DepthStencilDisabled, 1);
                DirectX.DeviceContext->VSSetShader(DirectX.VertexShader[Vertex_Shader_Barycentric_ID].Shader, NULL, 0);
                DirectX.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                DirectX.DeviceContext->IASetInputLayout(DirectX.InstancedLayout[instanced_layout_text_id]);

                for (int FontID = 0; FontID < game_font_id_count; FontID++) {
                    game_font* Font = GetAsset(Group->Assets, (game_font_id)FontID);
                    directX_font_buffer* FontBuffer = &DirectX.FontBuffer[FontID];
                    
                    uint32 Stride = 4*sizeof(float);
                    uint32 Offset = 0;

                    DirectX.DeviceContext->IASetVertexBuffers(0, 1, &FontBuffer->VertexBuffer, &Stride, &Offset);
                    DirectX.DeviceContext->IASetIndexBuffer(FontBuffer->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

                    Stride = 7*sizeof(float);

                    for (int c = 0; c < FONT_CHARACTERS_COUNT; c++) {
                        uint32 Count = Group->TextBuffer.Count[FontID][c];
                        if (Count > 0) {
                            game_font_character* pCharacter = &Font->Characters[c];
                            void* MappedBuffer = GetMappedBuffer(FontBuffer->InstanceBuffer);
                            if (MappedBuffer) {
                                memcpy(
                                    MappedBuffer, 
                                    Group->TextBuffer.Instances[FontID][c].Base, 
                                    Group->TextBuffer.Instances[FontID][c].Used
                                );
                                DirectX.DeviceContext->Unmap(FontBuffer->InstanceBuffer, 0);

                                DirectX.DeviceContext->IASetVertexBuffers(1, 1, &FontBuffer->InstanceBuffer, &Stride, &Offset);

                                // Solid triangles
                                if (pCharacter->nSolidTriangles > 0) {
                                    DirectX.DeviceContext->PSSetShader(DirectX.PixelShader[Pixel_Shader_Attrib_Color_ID].Shader, NULL, 0);
                                    DirectX.DeviceContext->DrawIndexedInstanced(
                                        3*pCharacter->nSolidTriangles,
                                        Count,
                                        pCharacter->SolidTrianglesOffset,
                                        0,
                                        0
                                    );
                                }

                                // Exterior curves
                                if (pCharacter->nExteriorCurves > 0) {
                                    DirectX.DeviceContext->PSSetShader(DirectX.PixelShader[Pixel_Shader_Bezier_Exterior_ID].Shader, NULL, 0);
                                    DirectX.DeviceContext->DrawIndexedInstanced(
                                        3*pCharacter->nExteriorCurves,
                                        Count,
                                        pCharacter->ExteriorCurvesOffset,
                                        0,
                                        0
                                    );
                                }

                                // Interior curves
                                if (pCharacter->nInteriorCurves > 0) {
                                    DirectX.DeviceContext->PSSetShader(DirectX.PixelShader[Pixel_Shader_Bezier_Interior_ID].Shader, NULL, 0);
                                    DirectX.DeviceContext->DrawIndexedInstanced(
                                        3*pCharacter->nInteriorCurves,
                                        Count,
                                        pCharacter->InteriorCurvesOffset,
                                        0,
                                        0
                                    );
                                }
                            }
                        }
                    }
                }
            } break;

            case render_shader_pass: {
                render_shader_pass_command ShaderCommand = Group->ShaderPassCommands[Command.Index];

                directX_render_target* Source = &DirectX.Target[ShaderCommand.Source];
                directX_render_target* Target = &DirectX.Target[ShaderCommand.Target];

                // Normal shaders
                BindTarget(ShaderCommand.Target);

                DirectX.DeviceContext->VSSetShader(DirectX.VertexShader[Vertex_Shader_Passthrough_ID].Shader, NULL, 0);

                directX_Pixel_Shader_ID PixelShaderID = Pixel_Shader_Single_Color_ID;
                switch (ShaderCommand.Type) {
                    default: Raise("DirectX: Invalid shader pass type.");
                }

                DirectX.DeviceContext->PSSetShader(DirectX.PixelShader[PixelShaderID].Shader, NULL, 0);
                DirectX.DeviceContext->PSSetShaderResources(0, 1, &Source->ShaderTexture);
                DirectX.DeviceContext->PSSetSamplers(0, 1, &DirectX.PixelShader[PixelShaderID].Sampler);

                SetColorBuffer(ShaderCommand.Color);
                SetOutlineBuffer(ShaderCommand.Width, ShaderCommand.Level);

                vertex_layout_id LayoutID = vertex_layout_v2_v2_id;
                uint32 Stride = VertexLayouts[LayoutID].Stride;
                uint32 VertexOffset = 0;
                DirectX.DeviceContext->IASetVertexBuffers(0, 1, &DirectX.VertexBuffer[LayoutID], &Stride, &VertexOffset);
                DirectX.DeviceContext->Draw(ShaderCommand.VertexEntry.Count, ShaderCommand.VertexEntry.Offset);
                
            } break;

            case render_compute: {
                render_compute_command ComputeCommand = Group->ComputeCommands[Command.Index];

                DirectX.DeviceContext->OMSetRenderTargets(0, NULL, NULL);
                directX_render_target* Target = &DirectX.Target[ComputeCommand.Target];

                directX_Compute_Shader_ID ShaderID;
                switch(ComputeCommand.Type) {
                    case compute_outline_init: { ShaderID = Compute_Shader_Outline_Init_ID; } break;
                    case compute_jump_flood: { 
                        ShaderID = Compute_Shader_Jump_Flood_ID;
                        SetOutlineBuffer(0.0f, ComputeCommand.Level);
                    } break;
                    case compute_outline:{
                        ShaderID = Compute_Shader_Outline_ID;
                        SetOutlineBuffer(ComputeCommand.Width, 0);
                        SetColorBuffer(ComputeCommand.Color);
                    } break;
                    default: Raise("DirectX: Invalid compute shader ID.");
                }

                DirectX.DeviceContext->CSSetShader(DirectX.ComputeShader[ShaderID].Shader, NULL, 0);
                DirectX.DeviceContext->CSSetUnorderedAccessViews(0, 1, &Target->UnorderedAccessView, NULL);
                DirectX.DeviceContext->Dispatch(ComputeCommand.nGroups.X, ComputeCommand.nGroups.Y, ComputeCommand.nGroups.Z);
                ID3D11UnorderedAccessView* NullUAV = 0;
                DirectX.DeviceContext->CSSetUnorderedAccessViews(0, 1, &NullUAV, NULL);
            } break;

            case render_target: {
                render_target_command TargetCommand = Group->TargetCommands[Command.Index];

                directX_render_target* Source = &DirectX.Target[TargetCommand.Source];
                directX_render_target* Target = &DirectX.Target[TargetCommand.Target];

                float BlendFactors[4] = {};
                DirectX.DeviceContext->OMSetBlendState(DirectX.TargetBlend, BlendFactors, 0xffffffff);

                BindTarget(TargetCommand.Target);

                directX_Vertex_Shader* VertexShader = &DirectX.VertexShader[Vertex_Shader_Passthrough_ID];
                directX_Pixel_Shader* PixelShader = &DirectX.PixelShader[
                    Source->Description.Multisample ? Pixel_Shader_Antialiasing_ID : Pixel_Shader_Texture_ID];
                
                DirectX.DeviceContext->IASetInputLayout(DirectX.VertexLayout[vertex_layout_v2_v2_id]);
                DirectX.DeviceContext->VSSetShader(VertexShader->Shader, NULL, 0);
                DirectX.DeviceContext->PSSetShader(PixelShader->Shader, NULL, 0);

                DirectX.DeviceContext->PSSetShaderResources(0, 1, &Source->ShaderTexture);
                DirectX.DeviceContext->PSSetSamplers(0, 1, &PixelShader->Sampler);

                vertex_layout Layout = VertexLayouts[vertex_layout_v2_v2_id];
                uint32 Offset = 0;
                DirectX.DeviceContext->IASetVertexBuffers(
                    0, 
                    1, 
                    &DirectX.VertexBuffer[vertex_layout_v2_v2_id], 
                    &Layout.Stride, 
                    &Offset
                );
                DirectX.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                DirectX.DeviceContext->Draw(TargetCommand.VertexEntry.Count, TargetCommand.VertexEntry.Offset);

                ID3D11ShaderResourceView* Empty = nullptr;
                DirectX.DeviceContext->PSSetShaderResources(0, 1, &Empty);
            } break;
        }
    }

    DirectX.SwapChain->Present(1, 0);
}
