#include "GameMath.h"
#include "GameAssets.h"
#include "Win32Debug.h"

#include "vulkan/Vulkan.h"
#include "vulkan/vulkan_win32.h"
#include "shaderc/shaderc.hpp"

#pragma comment(lib, "shaderc_combined.lib")

#include <set>
#include <vector>
#include <string>
#include <string.h>

#define VK_USE_PLATFORM_WIN32_KHR

const int MAX_FRAMES_IN_FLIGHT = 2;

struct vulkan_swap_chain {
    VkExtent2D Extent;
    VkSwapchainKHR SwapChain;
    std::vector<VkImage> Images;
    std::vector<VkImageView> ImageViews;
    std::vector<VkFramebuffer> Framebuffers;
};

struct vulkan {
    VkInstance Instance;
    VkDebugUtilsMessengerEXT DebugMessenger;
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;
    VkSurfaceKHR Surface;
    VkRect2D Scissor;
    VkViewport Viewport;
    VkQueue GraphicsQueue;
    uint32 GraphicsFamily;
    VkQueue PresentationQueue;
    uint32 PresentationFamily;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;
    vulkan_swap_chain SwapChain;
    VkRenderPass RenderPass;
    VkPipelineLayout PipelineLayout;
    VkPipeline Pipelines[game_shader_pipeline_id_count];
    VkCommandPool ComandPool;
    VkCommandBuffer CommandBuffer[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore ImageAvailable[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore RenderFinished[MAX_FRAMES_IN_FLIGHT];
    VkFence InFlightFence[MAX_FRAMES_IN_FLIGHT];
    uint32 CurrentFrame;
    bool Initialized;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
    VkDebugUtilsMessageTypeFlagsEXT MessageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    log_level Level = Info;
    if (Severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) Level = Warn;
    else if (Severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) Level = Error;
    OutputDebugStringA(pCallbackData->pMessage);
    OutputDebugStringA("\n");

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, 
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, 
    VkDebugUtilsMessengerEXT* pDebugMessenger
) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void CreateSwapChain(vulkan* Vulkan, uint32 Width, uint32 Height) {
    VkSurfaceCapabilitiesKHR Capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Vulkan->PhysicalDevice, Vulkan->Surface, &Capabilities);
    
    char DebugBuffer[256];
    sprintf_s(
        DebugBuffer, 
        "\n\tWidth: %d, Height: %d\n\tCapabilities:\n\t\tMinWidth: %d, MinHeight: %d\n\t\tMaxWidth: %d, MaxHeight: %d", 
        Width, Height, 
        Capabilities.minImageExtent.width, Capabilities.minImageExtent.height,
        Capabilities.maxImageExtent.width, Capabilities.maxImageExtent.height
    );
    Log(Info, DebugBuffer);

    Vulkan->SwapChain.Extent = {
        Clamp(Width, Capabilities.minImageExtent.width, Capabilities.maxImageExtent.width),
        Clamp(Height, Capabilities.minImageExtent.height, Capabilities.maxImageExtent.height)
    };

    Vulkan->Scissor.offset = {0,0};
    Vulkan->Scissor.extent = Vulkan->SwapChain.Extent;

    VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};
    SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapChainCreateInfo.surface = Vulkan->Surface;
    uint32 ImageCount = Capabilities.minImageCount + 1;
    if (Capabilities.maxImageCount > 0 && ImageCount > Capabilities.maxImageCount) {
        SwapChainCreateInfo.minImageCount = Capabilities.maxImageCount;
    }
    SwapChainCreateInfo.minImageCount = ImageCount;
    SwapChainCreateInfo.imageFormat = Vulkan->SurfaceFormat.format;
    SwapChainCreateInfo.imageColorSpace = Vulkan->SurfaceFormat.colorSpace;
    SwapChainCreateInfo.imageExtent = Vulkan->SwapChain.Extent;
    SwapChainCreateInfo.imageArrayLayers = 1; // Make 2 for stereoscopic 3D
    SwapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    uint32 QueueFamilyIndices[] = { Vulkan->GraphicsFamily, Vulkan->PresentationFamily };
    if (Vulkan->GraphicsFamily != Vulkan->PresentationFamily) {
        SwapChainCreateInfo.queueFamilyIndexCount = 2;
        SwapChainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }
    else {
        SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    SwapChainCreateInfo.preTransform = Capabilities.currentTransform;
    SwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapChainCreateInfo.presentMode = Vulkan->PresentMode;
    SwapChainCreateInfo.clipped = VK_TRUE;
    SwapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult SwapChainCreationResult = vkCreateSwapchainKHR(Vulkan->LogicalDevice, &SwapChainCreateInfo, NULL, &Vulkan->SwapChain.SwapChain);
    if (SwapChainCreationResult != VK_SUCCESS) {
        Raise("Swapchain creation failed.");
    }

// Images
    vkGetSwapchainImagesKHR(Vulkan->LogicalDevice, Vulkan->SwapChain.SwapChain, &ImageCount, NULL);
    Vulkan->SwapChain.Images.resize(ImageCount);
    Vulkan->SwapChain.ImageViews.resize(ImageCount);
    vkGetSwapchainImagesKHR(Vulkan->LogicalDevice, Vulkan->SwapChain.SwapChain, &ImageCount, Vulkan->SwapChain.Images.data());

// Image views
    for (int i = 0; i < Vulkan->SwapChain.Images.size(); i++) {
        VkImageViewCreateInfo ImageViewCreateInfo = {};
        ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewCreateInfo.image = Vulkan->SwapChain.Images[i];
        ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ImageViewCreateInfo.format = Vulkan->SurfaceFormat.format;
        ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ImageViewCreateInfo.subresourceRange.levelCount = 1;
        ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ImageViewCreateInfo.subresourceRange.layerCount = 1;

        VkResult ImageViewCreationResult = vkCreateImageView(Vulkan->LogicalDevice, &ImageViewCreateInfo, NULL, &Vulkan->SwapChain.ImageViews[i]);
        if (ImageViewCreationResult != VK_SUCCESS) {
            Raise("Image view creation went wrong.");
        }
    }

// Framebuffers
    Vulkan->SwapChain.Framebuffers.resize(Vulkan->SwapChain.ImageViews.size());
    for (int i = 0; i < Vulkan->SwapChain.ImageViews.size(); i++) {
        VkImageView Attachments[] = { Vulkan->SwapChain.ImageViews[i] };
        VkFramebufferCreateInfo FramebufferInfo = {};
        FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        FramebufferInfo.renderPass = Vulkan->RenderPass;
        FramebufferInfo.attachmentCount = 1;
        FramebufferInfo.pAttachments = Attachments;
        FramebufferInfo.width = Vulkan->SwapChain.Extent.width;
        FramebufferInfo.height = Vulkan->SwapChain.Extent.height;
        FramebufferInfo.layers = 1;

        VkResult Result = vkCreateFramebuffer(Vulkan->LogicalDevice, &FramebufferInfo, NULL, &Vulkan->SwapChain.Framebuffers[i]);
        if (Result != VK_SUCCESS) {
            Raise("Framebuffer creation went wrong.");
        }
    }
}

shaderc_shader_kind GetShaderKind(game_shader_type Type) {
    switch(Type) {
        case Vertex_Shader:                  return shaderc_vertex_shader;
        case Geometry_Shader:                return shaderc_geometry_shader;
        case Tessellation_Control_Shader:    return shaderc_tess_control_shader;
        case Tessellation_Evaluation_Shader: return shaderc_tess_evaluation_shader;
        case Fragment_Shader:                return shaderc_fragment_shader;
        default: Raise("Invalid shader type.");
    }
    return shaderc_fragment_shader;
}

VkShaderStageFlagBits GetShaderStage(game_shader_type Type) {
    switch(Type) {
        case Vertex_Shader:                  return VK_SHADER_STAGE_VERTEX_BIT;
        case Geometry_Shader:                return VK_SHADER_STAGE_GEOMETRY_BIT;
        case Tessellation_Control_Shader:    return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case Tessellation_Evaluation_Shader: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case Fragment_Shader:                return VK_SHADER_STAGE_FRAGMENT_BIT;
        default: Raise("Invalid shader type.");
    }
    return VK_SHADER_STAGE_FRAGMENT_BIT;
}

VkFormat GetShaderAttributeFormat(shader_layout_type Type) {
    switch(Type) {
        case shader_layout_type_float: { return VK_FORMAT_R32_SFLOAT; } break;
        case shader_layout_type_vec2:  { return VK_FORMAT_R32G32_SFLOAT; } break;
        case shader_layout_type_vec3:  { return VK_FORMAT_R32G32B32_SFLOAT; } break;
        case shader_layout_type_vec4:  { return VK_FORMAT_R32G32B32A32_SFLOAT; } break;
        case shader_layout_type_int:   { return VK_FORMAT_R32_SINT; } break;
        case shader_layout_type_ivec2: { return VK_FORMAT_R32G32_SINT; } break;
        case shader_layout_type_ivec3: { return VK_FORMAT_R32G32B32_SINT; } break;
        case shader_layout_type_ivec4: { return VK_FORMAT_R32G32B32A32_SINT; } break;
        default: Raise("Invalid shader attribute type.");
    }
    return VK_FORMAT_R32_SFLOAT;
}

void CompileShader(shaderc::Compiler* Compiler, shaderc::CompileOptions* Options, game_shader* Shader) {
    // If SPIR-V binary doesn't exist or is old, recompile it
    char ResultPath[256];
    strcpy(ResultPath, Shader->File.Path);
    strcat_s(ResultPath, ".spv");
    read_file_result ResultFile = PlatformReadEntireFile(ResultPath);
    if (ResultFile.ContentSize == 0 || ResultFile.Timestamp < Shader->File.Timestamp) {
       std::string ShaderCode(Shader->Code);
        
        char* Filename = strrchr(Shader->File.Path, '\\') + 1;
        shaderc::SpvCompilationResult Result = Compiler->CompileGlslToSpv(Shader->Code, GetShaderKind(Shader->Type), Filename);
        if (Result.GetCompilationStatus() != shaderc_compilation_status_success) {
            Raise(Result.GetErrorMessage().data());
        }
        uint32 ResultSize = std::distance(Result.cbegin(), Result.cend()) << 2;
        PlatformWriteEntireFile(ResultPath, ResultSize, (void*)Result.cbegin());

        ResultFile = PlatformReadEntireFile(ResultPath);
    }
    Shader->BinarySize = ResultFile.ContentSize;
    Shader->Binary = (uint32*)ResultFile.Content;
}

void CreatePipelineLayout(vulkan* Vulkan, game_assets* Assets) {
    VkDescriptorSetLayout Sets[SHADER_SETS];

// UBOs
    for(int Set = 0; Set < 2; Set++) {
        VkDescriptorSetLayout* SetLayout = &Sets[Set];
        VkDescriptorSetLayoutBinding SetLayoutBindings[MAX_SHADER_SET_BINDINGS] = {};
        uint32 Bindings = Assets->nBindings[Set];
        for(int Binding = 0; Binding < Bindings; Binding++) {
            SetLayoutBindings[Binding].binding = Binding;
            SetLayoutBindings[Binding].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            SetLayoutBindings[Binding].descriptorCount = 1;
            SetLayoutBindings[Binding].stageFlags = VK_SHADER_STAGE_ALL;
            SetLayoutBindings[Binding].pImmutableSamplers = NULL;
        }

        VkDescriptorSetLayoutCreateInfo SetLayoutCreateInfo = {};
        SetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        SetLayoutCreateInfo.bindingCount = Bindings;
        SetLayoutCreateInfo.pBindings = SetLayoutBindings;

        VkResult Result = vkCreateDescriptorSetLayout(Vulkan->LogicalDevice, &SetLayoutCreateInfo, NULL, SetLayout);
        if (Result != VK_SUCCESS) {
            Raise("Set layout creation went wrong for UBOs.");
        }
    }

// Samplers (set = 2)
    VkDescriptorSetLayout* SetLayout = &Sets[2];
    VkDescriptorSetLayoutBinding SetLayoutBindings[MAX_SHADER_SET_BINDINGS] = {};
    uint32 Bindings = Assets->nSamplers;
    for(int Binding = 0; Binding < Bindings; Binding++) {
        SetLayoutBindings[Binding].binding = Binding;
        SetLayoutBindings[Binding].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        SetLayoutBindings[Binding].descriptorCount = 1;
        SetLayoutBindings[Binding].stageFlags = VK_SHADER_STAGE_ALL;
        SetLayoutBindings[Binding].pImmutableSamplers = NULL;
    }

    VkDescriptorSetLayoutCreateInfo SetLayoutCreateInfo = {};
    SetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    SetLayoutCreateInfo.bindingCount = Bindings;
    SetLayoutCreateInfo.pBindings = SetLayoutBindings;

    VkResult Result = vkCreateDescriptorSetLayout(Vulkan->LogicalDevice, &SetLayoutCreateInfo, NULL, SetLayout);
    if (Result != VK_SUCCESS) {
        Raise("Set layout creation went wrong for samplers.");
    }

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo = {};
    PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInfo.setLayoutCount = SHADER_SETS;
    PipelineLayoutCreateInfo.pSetLayouts = Sets;
    PipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    PipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    VkResult PipelineLayoutCreateResult = vkCreatePipelineLayout(Vulkan->LogicalDevice, &PipelineLayoutCreateInfo, NULL, &Vulkan->PipelineLayout);
    if (PipelineLayoutCreateResult != VK_SUCCESS) {
        Raise("Pipeline layout creation went wrong.");
    }
}

void CreatePipeline(
    vulkan* Vulkan, 
    game_assets* Assets, 
    VkShaderModule* ShaderModules, 
    VkPipelineDynamicStateCreateInfo* DynamicState, 
    game_shader_pipeline_id PipelineID
) {
    game_shader_pipeline* Pipeline = &Assets->ShaderPipeline[PipelineID];

    VkPipelineShaderStageCreateInfo ShaderStageCreateInfos[game_shader_type_count] = {};
    std::vector<VkPipelineShaderStageCreateInfo> ShaderStages = {};
    for (int i = 0; i < game_shader_type_count; i++) {
        if (Pipeline->IsProvided[i]) {
            game_shader* Shader = &Assets->Shader[Pipeline->Pipeline[i]];
            VkPipelineShaderStageCreateInfo ShaderStageCreateInfo = {};
            ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            ShaderStageCreateInfo.stage = GetShaderStage(Shader->Type);
            ShaderStageCreateInfo.module = ShaderModules[Shader->ID];
            ShaderStageCreateInfo.pName = "main";
            ShaderStages.push_back(ShaderStageCreateInfo);
        }
    }

// Vertex attributes
    game_shader* VertexShader = &Assets->Shader[Pipeline->Pipeline[Vertex_Shader]];
    uint32 VertexSize = 0;
    for (int j = 0; j < VertexShader->nAttributes; j++) {
        VertexSize += VertexShader->Attributes[j].Size;
    }
    VkVertexInputBindingDescription Binding;
    Binding.binding = 0;
    Binding.stride = VertexShader->VertexSize;
    Binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription Attributes[MAX_SHADER_ATTRIBUTES];
    for (int j = 0; j < VertexShader->nAttributes; j++) {
        shader_attribute Attribute = VertexShader->Attributes[j];
        VkVertexInputAttributeDescription* AttributeDescription = &Attributes[j];
        AttributeDescription->binding = 0;
        AttributeDescription->offset = Attribute.Offset;
        AttributeDescription->location = Attribute.Location;
        AttributeDescription->format = GetShaderAttributeFormat(Attribute.Type);
    }

    VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo = {};
    VertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputCreateInfo.vertexBindingDescriptionCount = 1;
    VertexInputCreateInfo.pVertexBindingDescriptions = &Binding;
    VertexInputCreateInfo.vertexAttributeDescriptionCount = VertexShader->nAttributes;
    VertexInputCreateInfo.pVertexAttributeDescriptions = Attributes;

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo = {};
    InputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo ViewportState = {};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    ViewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo RasterizerCreateInfo = {};
    RasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizerCreateInfo.depthClampEnable = VK_FALSE;
    RasterizerCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    RasterizerCreateInfo.depthBiasEnable = VK_FALSE;
    RasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
    RasterizerCreateInfo.depthBiasClamp = 0.0f;
    RasterizerCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo Multisampling = {};
    Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    Multisampling.sampleShadingEnable = VK_FALSE;
    Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    Multisampling.minSampleShading = 1.0f; // Optional
    Multisampling.pSampleMask = nullptr; // Optional
    Multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    Multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineDepthStencilStateCreateInfo DepthStencil = {};

    VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_FALSE;
    ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo ColorBlending{};
    ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlending.logicOpEnable = VK_FALSE;
    ColorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    ColorBlending.attachmentCount = 1;
    ColorBlending.pAttachments = &ColorBlendAttachment;
    ColorBlending.blendConstants[0] = 0.0f; // Optional
    ColorBlending.blendConstants[1] = 0.0f; // Optional
    ColorBlending.blendConstants[2] = 0.0f; // Optional
    ColorBlending.blendConstants[3] = 0.0f; // Optional

    VkGraphicsPipelineCreateInfo PipelineCreateInfo = {};
    PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInfo.stageCount = ShaderStages.size();
    PipelineCreateInfo.pStages = ShaderStages.data();
    PipelineCreateInfo.pVertexInputState = &VertexInputCreateInfo;
    PipelineCreateInfo.pInputAssemblyState = &InputAssemblyCreateInfo;
    PipelineCreateInfo.pViewportState = &ViewportState;
    PipelineCreateInfo.pRasterizationState = &RasterizerCreateInfo;
    PipelineCreateInfo.pMultisampleState = &Multisampling;
    PipelineCreateInfo.pDepthStencilState = NULL;
    PipelineCreateInfo.pColorBlendState = &ColorBlending;
    PipelineCreateInfo.pDynamicState = DynamicState;
    PipelineCreateInfo.layout = Vulkan->PipelineLayout;
    PipelineCreateInfo.renderPass = Vulkan->RenderPass;
    PipelineCreateInfo.subpass = 0;
    PipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // For similar pipelines
    PipelineCreateInfo.basePipelineIndex = -1;

    VkResult PipelineCreateResult = vkCreateGraphicsPipelines(
        Vulkan->LogicalDevice, 
        VK_NULL_HANDLE,
        1, 
        &PipelineCreateInfo, 
        NULL, 
        &Vulkan->Pipelines[PipelineID]
    );

    if (PipelineCreateResult != VK_SUCCESS) {
        Raise("Shader pipeline creation failed.");
    }
}

void InitializeRenderer( vulkan* Vulkan, HWND Window, HINSTANCE Instance, game_assets* Assets) {
    VkApplicationInfo AppInfo = {};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "VulkanTest";
    AppInfo.applicationVersion = VK_MAKE_API_VERSION(0,1,0,0);
    AppInfo.pEngineName = "No Engine";
    AppInfo.engineVersion = VK_MAKE_API_VERSION(0,1,0,0);
    AppInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;

// Extensions
    const char* Extensions[3] = { "VK_KHR_surface", "VK_KHR_win32_surface" };

    CreateInfo.enabledExtensionCount = 2;
    CreateInfo.enabledLayerCount = 0;
// Validation Layers
#ifdef _DEBUG
    Extensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    CreateInfo.enabledExtensionCount = 3;
    // uint32 LayerCount = 0;
    // vkEnumerateInstanceLayerProperties(&LayerCount, NULL);
    // VkLayerProperties LayerProperties[20];
    // vkEnumerateInstanceLayerProperties(&LayerCount, LayerProperties);
    const char* ValidationLayer = "VK_LAYER_KHRONOS_validation";
    CreateInfo.enabledLayerCount = 1;
    CreateInfo.ppEnabledLayerNames = &ValidationLayer;
#endif
    CreateInfo.ppEnabledExtensionNames = Extensions;
    VkResult Result = vkCreateInstance(&CreateInfo, NULL, &Vulkan->Instance);
    
    if (Result != VK_SUCCESS) {
        Raise("Vulkan instance was not correctly initialized.");
    }

// Debug message callback
#ifdef _DEBUG
    VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {};
    MessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    MessengerCreateInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        MessengerCreateInfo.messageType = 
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    MessengerCreateInfo.pfnUserCallback = DebugCallback;
    MessengerCreateInfo.pUserData = NULL;    

    if (CreateDebugUtilsMessengerEXT(Vulkan->Instance, &MessengerCreateInfo, nullptr, &Vulkan->DebugMessenger) != VK_SUCCESS) {
        Raise("Failed to set up Vulkan debug messenger.");
    }
#endif
    Log(Info, "Vulkan successfully initialized.");

// Create Win32 Surface
    VkWin32SurfaceCreateInfoKHR Win32SurfaceCreateInfo = {};
    Win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    Win32SurfaceCreateInfo.hwnd = Window;
    Win32SurfaceCreateInfo.hinstance = Instance;

    VkResult SurfaceCreationResult = vkCreateWin32SurfaceKHR(Vulkan->Instance, &Win32SurfaceCreateInfo, NULL, &Vulkan->Surface);
    if (SurfaceCreationResult != VK_SUCCESS) {
        Raise("Surface creation went wrong.");
    }

// Device
    uint32_t DeviceCount = 0;
    vkEnumeratePhysicalDevices(Vulkan->Instance, &DeviceCount, nullptr);

    if (DeviceCount == 0) {
        Raise("Failed to find GPUs with Vulkan support.");
    }

    VkPhysicalDevice Devices[8];
    vkEnumeratePhysicalDevices(Vulkan->Instance, &DeviceCount, Devices);

// Check if device is suitable
    bool SuitableDeviceFound = false;
    uint32 GraphicsFamily;
    uint32 PresentationFamily;
    for (int i = 0; i < DeviceCount; i++) {
        VkPhysicalDevice Device = Devices[0];
        VkPhysicalDeviceProperties Properties;
        vkGetPhysicalDeviceProperties(Device, &Properties);

        VkPhysicalDeviceFeatures Features;
        vkGetPhysicalDeviceFeatures(Device, &Features);
        
        // Queue families
        uint32 QueueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);

        VkQueueFamilyProperties QueueFamilies[8];
        vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies);

        bool GraphicsFamilyFound = false;
        bool PresentationFamilyFound = false;
        for (int j = 0; j < QueueFamilyCount; j++) {
            VkQueueFamilyProperties QueueFamily = QueueFamilies[j];
            if (!GraphicsFamilyFound && (QueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) { GraphicsFamilyFound = true; GraphicsFamily = j; }
            VkBool32 PresentationSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(Device, j, Vulkan->Surface, &PresentationSupport);
            if (!PresentationFamilyFound && PresentationSupport) { PresentationFamilyFound = true; PresentationFamily = j; };
        }

        // Check required device extensions
        std::vector<const char*> RequiredDeviceExtensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        std::set<std::string> RequiredDeviceExtensions(RequiredDeviceExtensionNames.begin(), RequiredDeviceExtensionNames.end());
        uint32 DeviceExtensionCount;
        vkEnumerateDeviceExtensionProperties(Device, NULL, &DeviceExtensionCount, NULL);
        std::vector<VkExtensionProperties> AvailableExtensions(DeviceExtensionCount);
        vkEnumerateDeviceExtensionProperties(Device, NULL, &DeviceExtensionCount, AvailableExtensions.data());
        for (const auto& Extension : AvailableExtensions) {
            RequiredDeviceExtensions.erase(Extension.extensionName);
            if (RequiredDeviceExtensions.empty()) break;
        }
        
        // Create physical device
        if (GraphicsFamilyFound && PresentationFamilyFound && RequiredDeviceExtensions.empty() && Features.tessellationShader) {
            const int MaxFamilies = 2;
            std::set<uint32> UniqueQueueFamilies = { GraphicsFamily, PresentationFamily };
            std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
            Vulkan->PhysicalDevice = Device;

            float QueuePriority = 1.0f;
            for (uint32 QueueFamilyIndex : UniqueQueueFamilies) {
                VkDeviceQueueCreateInfo QueueCreateInfo = {};
                QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                QueueCreateInfo.queueFamilyIndex = GraphicsFamily;
                QueueCreateInfo.queueCount = 1;
                QueueCreateInfo.pQueuePriorities = &QueuePriority;
                QueueCreateInfos.push_back(QueueCreateInfo);
            }

            VkDeviceCreateInfo LogicalDeviceCreateInfo = {};
            LogicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            LogicalDeviceCreateInfo.queueCreateInfoCount = (uint32)QueueCreateInfos.size();
            LogicalDeviceCreateInfo.pQueueCreateInfos = QueueCreateInfos.data();
            LogicalDeviceCreateInfo.pEnabledFeatures = &Features;
            LogicalDeviceCreateInfo.enabledExtensionCount = (uint32)RequiredDeviceExtensionNames.size();
            LogicalDeviceCreateInfo.ppEnabledExtensionNames = RequiredDeviceExtensionNames.data();

            VkResult LogicalDeviceCreationResult = vkCreateDevice(Vulkan->PhysicalDevice, &LogicalDeviceCreateInfo, NULL, &Vulkan->LogicalDevice);

            if (LogicalDeviceCreationResult != VK_SUCCESS) {
                Raise("Logical device creation failed.");
            }

            vkGetDeviceQueue(Vulkan->LogicalDevice, GraphicsFamily, 0, &Vulkan->GraphicsQueue);
            vkGetDeviceQueue(Vulkan->LogicalDevice, PresentationFamily, 0, &Vulkan->PresentationQueue);

            SuitableDeviceFound = true;
            break;
        }
    }
    
    if (!SuitableDeviceFound) {
       Raise("Vulkan device selection went wrong.");
    }

// Swap chain creation
    uint32 FormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Vulkan->PhysicalDevice, Vulkan->Surface, &FormatCount, NULL);
    if (FormatCount > 0) {
        std::vector<VkSurfaceFormatKHR> AvailableFormats(FormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(Vulkan->PhysicalDevice, Vulkan->Surface, &FormatCount, AvailableFormats.data());
        for (const auto& AvailableFormat : AvailableFormats) {
            if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                Vulkan->SurfaceFormat = AvailableFormat;
                break;
            }
        }
    }
    
    uint32 PresentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Vulkan->PhysicalDevice, Vulkan->Surface, &PresentModeCount, NULL);
    Vulkan->PresentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (PresentModeCount > 0) {
        std::vector<VkPresentModeKHR> AvailablePresentModes(PresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(Vulkan->PhysicalDevice, Vulkan->Surface, &PresentModeCount, AvailablePresentModes.data());
        for (const auto& AvailablePresentMode : AvailablePresentModes) {
            if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                Vulkan->PresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            }
        }
    }

// Framebuffers setup
    VkAttachmentDescription ColorAttachment = {};
    ColorAttachment.format = Vulkan->SurfaceFormat.format;
    ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorAttachmentReference = {};
    ColorAttachmentReference.attachment = 0;
    ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription Subpass = {};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.colorAttachmentCount = 1;
    Subpass.pColorAttachments = &ColorAttachmentReference;

    VkSubpassDependency Dependency = {};
    Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    Dependency.dstSubpass = 0;
    Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.srcAccessMask = 0;
    Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo RenderPassCreateInfo = {};
    RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInfo.attachmentCount = 1;
    RenderPassCreateInfo.pAttachments = &ColorAttachment;
    RenderPassCreateInfo.subpassCount = 1;
    RenderPassCreateInfo.pSubpasses = &Subpass;
    RenderPassCreateInfo.dependencyCount = 1;
    RenderPassCreateInfo.pDependencies = &Dependency;

    VkResult RenderPassCreateResult = vkCreateRenderPass(Vulkan->LogicalDevice, &RenderPassCreateInfo, NULL, &Vulkan->RenderPass);
    if (RenderPassCreateResult != VK_SUCCESS) {
        Raise("Render pass creation went wrong.");
    }

// Set width and height
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    uint32 Width = ClientRect.right - ClientRect.left;
    uint32 Height = ClientRect.bottom - ClientRect.top;

    Vulkan->Viewport.x = 0.0f;
    Vulkan->Viewport.y = 0.0f;
    Vulkan->Viewport.width = Width;
    Vulkan->Viewport.height = Height;
    Vulkan->Viewport.minDepth = 0.0f;
    Vulkan->Viewport.maxDepth = 1.0f;

    CreateSwapChain(Vulkan, Width, Height);

// Compile shaders
    VkShaderModule ShaderModules[game_shader_id_count] = {};
    shaderc::Compiler Compiler;
    shaderc::CompileOptions Options;

    Options.AddMacroDefinition("VULKAN");
    Options.SetOptimizationLevel(shaderc_optimization_level_performance);

    for(int i = 0; i < game_shader_id_count; i++) {
        game_shader* Shader = &Assets->Shader[i];
        CompileShader(&Compiler, &Options, Shader);

        VkShaderModuleCreateInfo ShaderModuleCreateInfo = {};
        ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        ShaderModuleCreateInfo.codeSize = Shader->BinarySize;
        ShaderModuleCreateInfo.pCode = Shader->Binary;

        VkResult ShaderModuleCreateResult = vkCreateShaderModule(Vulkan->LogicalDevice, &ShaderModuleCreateInfo, NULL, &ShaderModules[i]);
        if (ShaderModuleCreateResult != VK_SUCCESS) {
            Raise("Shader module creation went wrong.");
        }
    }

    VkDynamicState DynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo = {};
    DynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateCreateInfo.dynamicStateCount = 2;
    DynamicStateCreateInfo.pDynamicStates = DynamicStates;

    CreatePipelineLayout(Vulkan, Assets);
    CreatePipeline(Vulkan, Assets, ShaderModules, &DynamicStateCreateInfo, Shader_Pipeline_Vulkan_Test_ID);

    // for(int i = 0; i < game_compute_shader_id_count; i++) {
    //     game_compute_shader* Shader = &Assets->ComputeShader[i];
    //     CompileShader(Shader);

    //     VkShaderModuleCreateInfo ShaderModuleCreateInfo = {};
    //     ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    //     ShaderModuleCreateInfo.codeSize = Shader->BinarySize;
    //     ShaderModuleCreateInfo.pCode = Shader->Binary;

    //     VkResult ShaderModuleCreateResult = vkCreateShaderModule(Vulkan->LogicalDevice, &ShaderModuleCreateInfo, NULL, &Vulkan->ComputeShader[i]);
    //     if (ShaderModuleCreateResult != VK_SUCCESS) {
    //         Raise("Shader module creation went wrong.");
    //     }
    // }

    VkCommandPoolCreateInfo PoolCreateInfo = {};
    PoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    PoolCreateInfo.queueFamilyIndex = GraphicsFamily;

    VkResult PoolCreateResult = vkCreateCommandPool(Vulkan->LogicalDevice, &PoolCreateInfo, NULL, &Vulkan->ComandPool);
    if (PoolCreateResult != VK_SUCCESS) {
        Raise("Command pool creation went wrong.");
    }

    VkCommandBufferAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = Vulkan->ComandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    VkResult CommandBufferAllocationResult = vkAllocateCommandBuffers(Vulkan->LogicalDevice, &AllocInfo, Vulkan->CommandBuffer);
    if (CommandBufferAllocationResult != VK_SUCCESS) {
        Raise("Command buffer allocation went wrong.");
    }

    VkSemaphoreCreateInfo SemaphoreInfo = {};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo FenceInfo = {};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkResult ImageAvailableCreateResult = vkCreateSemaphore(Vulkan->LogicalDevice, &SemaphoreInfo, NULL, &Vulkan->ImageAvailable[i]);
        VkResult RenderFinishedCreateResult = vkCreateSemaphore(Vulkan->LogicalDevice, &SemaphoreInfo, NULL, &Vulkan->RenderFinished[i]);
        VkResult InFlightFenceCreateResult = vkCreateFence(Vulkan->LogicalDevice, &FenceInfo, NULL, &Vulkan->InFlightFence[i]);
        if (ImageAvailableCreateResult != VK_SUCCESS || RenderFinishedCreateResult != VK_SUCCESS || InFlightFenceCreateResult != VK_SUCCESS) {
            Raise("Semaphore or fence creation went wrong.");
        }
    }

    Vulkan->CurrentFrame = 0;
    Vulkan->Initialized = true;
}

void RecordCommandBuffer(vulkan* Vulkan, uint32 ImageIndex) {
    VkCommandBufferBeginInfo BeginInfo = {};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = 0;
    BeginInfo.pInheritanceInfo = NULL;

    VkCommandBuffer CommandBuffer = Vulkan->CommandBuffer[Vulkan->CurrentFrame];

    VkResult BeginResult = vkBeginCommandBuffer(CommandBuffer, &BeginInfo);
    if (BeginResult != VK_SUCCESS) {
        Raise("Begin command buffer went wrong.");
    }

    VkRenderPassBeginInfo RenderPassInfo = {};
    RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInfo.renderPass = Vulkan->RenderPass;
    RenderPassInfo.framebuffer = Vulkan->SwapChain.Framebuffers[ImageIndex];
    RenderPassInfo.renderArea.offset = {0, 0};
    RenderPassInfo.renderArea.extent = Vulkan->SwapChain.Extent;

    VkClearValue ClearColor = {{{1.0f, 0.0f, 1.0f, 1.0f}}};
    RenderPassInfo.clearValueCount = 1;
    RenderPassInfo.pClearValues = &ClearColor;

    vkCmdBeginRenderPass(CommandBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Vulkan->Pipelines[Shader_Pipeline_Vulkan_Test_ID]);

    VkViewport Viewport = {};
    Viewport.x = 0.0f;
    Viewport.y = 0.0f;
    Viewport.width = (float)Vulkan->SwapChain.Extent.width;
    Viewport.height = (float)Vulkan->SwapChain.Extent.height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;
    vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);

    VkRect2D Scissor = {};
    Scissor.offset = { 0, 0 };
    Scissor.extent = Vulkan->SwapChain.Extent;
    vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);

    vkCmdDraw(CommandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(CommandBuffer);
    VkResult EndResult = vkEndCommandBuffer(CommandBuffer);
    if (EndResult != VK_SUCCESS) {
        Raise("End render pass failed.");
    }
}

void ResizeWindow(vulkan* Vulkan, uint32 Width, uint32 Height) {
    vkDeviceWaitIdle(Vulkan->LogicalDevice);

// Cleanup
    for (uint64 i = 0; i < Vulkan->SwapChain.Framebuffers.size(); i++) {
        vkDestroyFramebuffer(Vulkan->LogicalDevice, Vulkan->SwapChain.Framebuffers[i], NULL);
    }

    for (uint64 i = 0; i < Vulkan->SwapChain.ImageViews.size(); i++) {
        vkDestroyImageView(Vulkan->LogicalDevice, Vulkan->SwapChain.ImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(Vulkan->LogicalDevice, Vulkan->SwapChain.SwapChain, NULL);

// Recreation
    CreateSwapChain(Vulkan, Width, Height);
}

void ScreenCapture(vulkan* Vulkan, int Width, int Height) {
    // TODO
}

void Render(HWND Window, render_group* Group, vulkan* Vulkan, double Time) {
    uint32 CurrentFrame = Vulkan->CurrentFrame;

    vkWaitForFences(Vulkan->LogicalDevice, 1, &Vulkan->InFlightFence[CurrentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(Vulkan->LogicalDevice, 1, &Vulkan->InFlightFence[CurrentFrame]);

    uint32 ImageIndex;
    vkAcquireNextImageKHR(
        Vulkan->LogicalDevice, 
        Vulkan->SwapChain.SwapChain, 
        UINT64_MAX, 
        Vulkan->ImageAvailable[CurrentFrame], 
        VK_NULL_HANDLE, 
        &ImageIndex
    );
    vkResetCommandBuffer(Vulkan->CommandBuffer[CurrentFrame], 0);
    RecordCommandBuffer(Vulkan, ImageIndex);

    VkSubmitInfo SubmitInfo = {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore WaitSemaphores[] = { Vulkan->ImageAvailable[CurrentFrame] };
    VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    SubmitInfo.waitSemaphoreCount = 1;
    SubmitInfo.pWaitSemaphores = WaitSemaphores;
    SubmitInfo.pWaitDstStageMask = WaitStages;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &Vulkan->CommandBuffer[CurrentFrame];

    VkSemaphore SignalSemaphores[] = { Vulkan->RenderFinished[CurrentFrame] };
    SubmitInfo.signalSemaphoreCount = 1;
    SubmitInfo.pSignalSemaphores = SignalSemaphores;

    VkResult QueueSubmitResult = vkQueueSubmit(Vulkan->GraphicsQueue, 1, &SubmitInfo, Vulkan->InFlightFence[CurrentFrame]);
    if (QueueSubmitResult != VK_SUCCESS) {
        Raise("Queue submit went wrong.");
    }

    VkPresentInfoKHR PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = SignalSemaphores;

    VkSwapchainKHR SwapChains[] = { Vulkan->SwapChain.SwapChain };
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = SwapChains;
    PresentInfo.pImageIndices = &ImageIndex;
    PresentInfo.pResults = NULL;

    vkQueuePresentKHR(Vulkan->PresentationQueue, &PresentInfo);

    Vulkan->CurrentFrame += 1;
    if (Vulkan->CurrentFrame >= MAX_FRAMES_IN_FLIGHT) Vulkan->CurrentFrame = 0;
}