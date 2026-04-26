#ifndef GAME_SHADER_H
#define GAME_SHADER_H

#include <slang.h>
#include <slang-com-ptr.h>

#include "GamePlatform.h"
#include "GameBuild.h"


struct game_shader_entry_point {
    char Name[64];
    SlangStage Stage;
    Slang::ComPtr<slang::IEntryPoint> EntryPoint;
    Slang::ComPtr<slang::IComponentType> ComposedProgram;
    Slang::ComPtr<slang::IComponentType> LinkedProgram;
    Slang::ComPtr<slang::IBlob> Binary;
};

ENUM(game_shader_module_id,
    Shader_Module_Buffers_ID,
    Shader_Module_Color_ID,
    Shader_Module_Screen_ID,
    Shader_Module_Mesh_ID,
    Shader_Module_Sky_ID
);

struct game_shader_module {
    game_shader_module_id ID;
    file_info File;
    char Name[64];
    char* Code;
    Slang::ComPtr<slang::IModule> Module;
    uint32 nEntryPoints;
    game_shader_entry_point* EntryPoints;
    bool Compiled;
};

struct slang_context {
    Slang::ComPtr<slang::IGlobalSession> GlobalSession;
    Slang::ComPtr<slang::IBlob> Diagnostic;
    Slang::ComPtr<slang::ISession> Session;
    game_shader_module Modules[game_shader_module_id_count];
    uint32 nEntryPoints;
    game_shader_entry_point* EntryPoints;
};

slang_context Context;

void CompileShaderModule(game_shader_module* Module) {
    using namespace slang;

    Module->Module = Context.Session->loadModuleFromSourceString(Module->Name, nullptr, Module->Code, Context.Diagnostic.writeRef());
    if (Module->Module) {
        Module->Compiled = true;
        
        char LogBuffer[128];
        sprintf(LogBuffer, "Loaded Slang module '%s' from %s.", Module->Name, Module->File.Path);
        Log(Info, LogBuffer);

        Module->nEntryPoints = Module->Module->getDefinedEntryPointCount();
    }
    else {
        Log(Error, (char*)Context.Diagnostic->getBufferPointer());
    }
}

void LoadShaderModule(game_shader_module_id ID, const char* Path) {
    game_shader_module* Module = &Context.Modules[ID];
    Module->ID = ID;

    const char* Filename = GetFileName(Path);
    char ModuleName[64];
    strcpy(ModuleName, Filename);
    strtok(ModuleName, ".");
    strcpy(Module->Name, ModuleName);
    
    Module->Code = (char*)Platform.ReadEntireFile(Path, &Module->File);
    CompileShaderModule(Module);
    Context.nEntryPoints += Module->nEntryPoints;
}

void ReloadShaderModule(game_shader_module_id ID) {
    game_shader_module* Module = Context.Modules + ID;
    file_info File = Platform.GetFileInfo(Module->File.Path);
    if (File.Size > 0 && File.Timestamp > Module->File.Timestamp) {
        Module->Code = (char*)Platform.ReadEntireFile(Module->File.Path, &Module->File);
        CompileShaderModule(Module);
    }
}

void InitializeSlang(memory_arena* Arena, renderer Renderer) {
    using namespace slang;

    SlangGlobalSessionDesc GlobalSessionDescription = {};
    if (Renderer == Renderer_OpenGL || Renderer == Renderer_Vulkan) {
        GlobalSessionDescription.enableGLSL = true;
    }
    
    if (SLANG_FAILED(createGlobalSession(&GlobalSessionDescription, Context.GlobalSession.writeRef()))) {
        Raise("Slang global session creation failed.");
    }

    TargetDesc Target = {};
    switch (Renderer) {
        case Renderer_DirectX: {
            Target.format = SLANG_DXBC;
            Target.profile = Context.GlobalSession->findProfile("sm_5_0");
        } break;
        
        case Renderer_OpenGL: {
            Target.format = SLANG_GLSL;
            Target.profile = Context.GlobalSession->findProfile("glsl_450");
        } break;
        
        case Renderer_Vulkan: {
            Target.format = SLANG_SPIRV;
            Target.profile = Context.GlobalSession->findProfile("glsl_450");
        }
        
        default: {
            Raise("Invalid renderer.");
        }
    }
    
    SessionDesc SessionDescription = {};
    SessionDescription.targets = &Target;
    SessionDescription.targetCount = 1;

    const char* SearchPaths[] = {
        "GameRender/Shaders/Slang/shared",
        "GameRender/Shaders/Slang"
    };
    SessionDescription.searchPaths = SearchPaths;
    SessionDescription.searchPathCount = ArrayCount(SearchPaths);

    if (SLANG_FAILED(Context.GlobalSession->createSession(SessionDescription, Context.Session.writeRef()))) {
        Raise("Slang session creation failed.");
    };

    Log(Info, "Slang compiler initialized.");

    // Shader modules
    Context.nEntryPoints = 0;
    LoadShaderModule(Shader_Module_Buffers_ID, "GameRender/Shaders/Slang/shared/Buffers.slang");
    LoadShaderModule(Shader_Module_Color_ID,   "GameRender/Shaders/Slang/shared/Color.slang");
    LoadShaderModule(Shader_Module_Screen_ID,  "GameRender/Shaders/Slang/Screen.slang");
    LoadShaderModule(Shader_Module_Mesh_ID,    "GameRender/Shaders/Slang/Mesh.slang");
    LoadShaderModule(Shader_Module_Sky_ID,    "GameRender/Shaders/Slang/Sky.slang");

    Context.EntryPoints = PushArray(Arena, Context.nEntryPoints, game_shader_entry_point);
    game_shader_entry_point* pEntryPoints = Context.EntryPoints;
    for (int i = 0; i < game_shader_module_id_count; i++) {
        game_shader_module* Module = Context.Modules + i;
        Module->EntryPoints = pEntryPoints;
        pEntryPoints += Module->nEntryPoints;
        for (int j = 0; j < Module->nEntryPoints; j++) {
            game_shader_entry_point* EntryPoint = Module->EntryPoints + j;
            if (SLANG_FAILED(Module->Module->getDefinedEntryPoint(j, EntryPoint->EntryPoint.writeRef()))) {
                Log(Error, "Slang entry point creation failed.");
                continue;
            }
            
            EntryPointLayout* Refl = EntryPoint->EntryPoint->getLayout(0, Context.Diagnostic.writeRef())->getEntryPointByIndex(0);
            strcpy(EntryPoint->Name, Refl->getName());
            EntryPoint->Stage = Refl->getStage();

            IComponentType* ComponentTypes[] = { Module->Module, EntryPoint->EntryPoint };
            if (SLANG_FAILED(Context.Session->createCompositeComponentType(ComponentTypes, 2, EntryPoint->ComposedProgram.writeRef(), Context.Diagnostic.writeRef()))) {
                Log(Error, (char*)Context.Diagnostic->getBufferPointer());
                continue;
            }

            if (SLANG_FAILED(EntryPoint->ComposedProgram->link(EntryPoint->LinkedProgram.writeRef(), Context.Diagnostic.writeRef()))) {
                Log(Error, (char*)Context.Diagnostic->getBufferPointer());
                continue;
            }

            if (SLANG_FAILED(EntryPoint->LinkedProgram->getEntryPointCode(0, 0, EntryPoint->Binary.writeRef(), Context.Diagnostic.writeRef()))) {
                Log(Error, (char*)Context.Diagnostic->getBufferPointer());
                continue;
            }
        }
    }
}

#endif