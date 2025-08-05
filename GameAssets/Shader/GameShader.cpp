#include "GameShader.h"

void LoadShader(platform_api* Platform, memory_arena* Arena, game_shader* Shader) {
    char* Destination = (char*)PushSize(Arena, Shader->File.ContentSize + 1);
    memcpy(Destination, Shader->Code, Shader->File.ContentSize);

    // Get attributes and uniforms
    tokenizer Tokenizer = InitTokenizer(Shader->Code);
    token Token = GetToken(Tokenizer);
    uint32 nAttributes = 0;
    vertex_attribute Attributes[MAX_VERTEX_ATTRIBUTES] = {};
    while (Token.Type != Token_End) {
        // Attributes
        if (Shader->Type == Vertex_Shader && Token == "layout") {
            vertex_attribute Attribute = {};

            Token = RequireToken(Tokenizer, Token_OpenParen);
            Token = RequireToken(Tokenizer, "location");
            Token = RequireToken(Tokenizer, Token_Equal);
            Attribute.Location = Parseuint32(Tokenizer);

            Token = RequireToken(Tokenizer, Token_CloseParen);
            Token = RequireToken(Tokenizer, Token_Identifier);
            // We only need input vertex attributes
            if (Token == "in") {
                Token = GetToken(Tokenizer);
                Attribute.Type = GetShaderType(Token);
                Attribute.Size = GetShaderTypeSize(Attribute.Type);
                Attributes[nAttributes++] = Attribute;
            }
        }

        // Uniforms
        if (Token == "VULKAN") {
            Token = RequireToken(Tokenizer, "layout");

            shader_uniform_block UBO = {};
            shader_uniform_sampler Sampler = {};

            Token = RequireToken(Tokenizer, Token_OpenParen);
            Token = RequireToken(Tokenizer, Token_Identifier);
            while (Token.Type != Token_CloseParen && Token.Type != Token_End) {
                if (Token == "set") {
                    Token = RequireToken(Tokenizer, Token_Equal);
                    UBO.Set = Parseuint32(Tokenizer);
                    Sampler.Set = UBO.Set;
                }
                else if (Token == "binding") {
                    Token = RequireToken(Tokenizer, Token_Equal);
                    UBO.Binding = Parseuint32(Tokenizer);
                    Sampler.Binding = UBO.Binding;
                }
                Token = GetToken(Tokenizer);
            }

            if (Token.Type == Token_End) {
                break;
            }

            Token = RequireToken(Tokenizer, Token_Identifier);
            if (Token == "uniform") {
                Token = GetToken(Tokenizer);
                if (Token == "sampler2D" || Token == "sampler2DMS") {
                    Assert(Sampler.Set == 2);
                    Shader->Sampler[Shader->nSamplers++] = Sampler;
                }
                else {
                    AdvanceUntil(Tokenizer, '{');
                    Token = RequireToken(Tokenizer, Token_OpenBrace);
                    Token = GetToken(Tokenizer);
                    while (Token.Type != Token_CloseBrace && Token.Type != Token_End) {
                        shader_type Type = GetShaderType(Token);
                        AdvanceUntil(Tokenizer, ';');
                        Token = GetToken(Tokenizer);
                        Token = GetToken(Tokenizer);
                        UBO.Member[UBO.nMembers++] = Type;
                    }
                    Shader->UBO[Shader->nUBOs++] = UBO;
                }
            }
        }
        Token = GetToken(Tokenizer);
    }
    
    Shader->VertexLayout = {};
    for (int i = 0; i < nAttributes; i++) {
        for (int j = 0; j < nAttributes; j++) {
            vertex_attribute Attribute = Attributes[j];
            if (Attribute.Location == i) {
                AddAttribute(&Shader->VertexLayout, Attribute.Type);
                break;
            }
        }
    }

    Platform->FreeFileMemory(Shader->Code);

    char LogBuffer[256];
    sprintf_s(LogBuffer, "Loaded shader %s.", Shader->File.Path);
    Log(Info, LogBuffer);
}

void LoadComputeShader(platform_api* Platform, memory_arena* Arena, game_compute_shader* Shader) {
    char* Destination = (char*)PushSize(Arena, Shader->Size + 1);
    memcpy(Destination, Shader->Code, Shader->Size);
    Platform->FreeFileMemory(Shader->Code);
}
