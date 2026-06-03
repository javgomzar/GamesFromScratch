#include "GameMath.h"
#include "Tokenizer.h"
#include "GameScene.h"
#include "GameMaterial.h"
#include "GameRender.h"


enum gltf_type {
    gltf_void,
    gltf_bool,
    gltf_string,
    gltf_int,
    gltf_float,
    gltf_struct,
    gltf_array
};

string ToString(token Token) {
    string Result;
    Result.Length = Token.Length;
    Result.Content = Token.Text;
    return Result;
}

// This function returns the GLTF type of the next token without advancing the tokenizer
gltf_type GetGLTFType(tokenizer Tokenizer) {
    AdvanceWhitespace(Tokenizer);

    token Token = GetToken(Tokenizer);

    switch (Token.Type) {
        case Token_Identifier: {
            if (Token == "true" || Token == "false") {
                return gltf_bool;
            }
            Raise("Invalid GLTF type. Found an identifier that is not a boolean value.");
        } break;

        case Token_String: {
            return gltf_string;
        } break;

        case Token_Minus: {
            Token = GetToken(Tokenizer);
            if (Token.Type == Token_Constant_Integer) {
                return gltf_int;
            }
            else if (Token.Type == Token_Constant_Decimal) {
                return gltf_float;
            }
        } break;

        case Token_Constant_Integer: {
            return gltf_int;
        } break;

        case Token_Constant_Decimal: {
            return gltf_float;
        } break;

        case Token_OpenBrace: {
            return gltf_struct;
        } break;

        case Token_OpenBracket: {
            return gltf_array;
        } break;

        default: {
            Raise("Invalid GLTF type.");
        }
    }
}

struct gltf_entry {
    string Name;
    gltf_type Type;
    gltf_type ArrayType;
    gltf_entry* Parent;
    gltf_entry* Children;
    gltf_entry* Next;
    int Length;
    string String;
    int Int;
    float Float;
    bool Bool;
    void* ArrayData;
};

gltf_entry* ParseGLTFEntry(
    memory_arena* Arena, 
    tokenizer& Tokenizer, 
    gltf_entry* Parent
);

void ParseGLTFStruct(
    memory_arena* Arena,
    tokenizer& Tokenizer,
    gltf_entry* Entry,
    gltf_entry* Parent
) {
    Entry->Type = gltf_struct;
    Entry->ArrayType = gltf_void;
    Entry->Length = 0;
    Entry->Parent = Parent;
    Entry->Children = nullptr;
    Entry->Next = nullptr;
    Entry->ArrayData = nullptr;

    RequireToken(Tokenizer, Token_OpenBrace);
    token Token;
    gltf_entry* Previous = nullptr;
    do {
        Entry->Length += 1;
        gltf_entry* Child = ParseGLTFEntry(Arena, Tokenizer, Entry);
        if (Previous) {
            Previous->Next = Child;
        }
        Previous = Child;
        if (!Entry->Children) {
            Entry->Children = Child;
        }
        Token = GetToken(Tokenizer);
    } while(Token.Type == Token_Comma);
    Assert(Token.Type == Token_CloseBrace);
}

gltf_entry* ParseGLTFEntry(
    memory_arena* Arena, 
    tokenizer& Tokenizer, 
    gltf_entry* Parent
) {
    gltf_entry* Entry = PushStruct(Arena, gltf_entry);
    Entry->ArrayType = gltf_void;
    Entry->Length = -1;
    Entry->Parent = Parent;
    Entry->Children = nullptr;
    Entry->Next = nullptr;
    Entry->ArrayData = nullptr;

    Entry->Name = ToString(RequireToken(Tokenizer, Token_String));
    RequireToken(Tokenizer, Token_Colon);
    
    Entry->Type = GetGLTFType(Tokenizer);
    switch (Entry->Type) {
        case gltf_bool: {
            Entry->Bool = ParseBool(Tokenizer);
        } break;

        case gltf_string: {
            Entry->String = ToString(RequireToken(Tokenizer, Token_String));
        } break;

        case gltf_int: {
            Entry->Int = ParseInt(Tokenizer);
        } break;
    
        case gltf_float: {
            Entry->Float = ParseFloat(Tokenizer);
        } break;

        case gltf_array: {
            RequireToken(Tokenizer, Token_OpenBracket);
            Entry->Length = 0;
            Entry->ArrayType = GetGLTFType(Tokenizer);
            token Token;
            gltf_entry* Previous = nullptr;
            if (Entry->ArrayType != gltf_struct) {
                Entry->ArrayData = Arena->Base + Arena->Used;
            }
            do {
                Entry->Length += 1;
                gltf_entry* Child = nullptr;
                switch(Entry->ArrayType) {
                    case gltf_bool: {
                        bool* Bool = PushStruct(Arena, bool);
                        *Bool = ParseBool(Tokenizer);
                    } break;

                    case gltf_string: {
                        PushString(Arena, ToString(RequireToken(Tokenizer, Token_String)));
                    } break;

                    case gltf_int: {
                        int* Int = PushStruct(Arena, int);
                        *Int = ParseInt(Tokenizer);
                    } break;

                    case gltf_float: {
                        float* Float = PushStruct(Arena, float);
                        *Float = ParseFloat(Tokenizer);
                    } break;

                    case gltf_array: {
                        Raise("No arrays of arrays; only flat arrays are allowed.");
                    } break;

                    case gltf_struct: {
                        Child = PushStruct(Arena, gltf_entry);
                        ParseGLTFStruct(Arena, Tokenizer, Child, Entry);
                        if (Previous) {
                            Previous->Next = Child;
                        }
                        Previous = Child;

                        if (!Entry->Children) {
                            Entry->Children = Child;
                        }
                    } break;
                }
                Token = GetToken(Tokenizer);
            } while(Token.Type == Token_Comma);
            Assert(Token.Type == Token_CloseBracket);
        } break;

        case gltf_struct: {
            ParseGLTFStruct(Arena, Tokenizer, Entry, Parent);
        } break;
    }

    return Entry;
}

enum gltf_component_type {
    gltf_int8,
    gltf_uint8,
    gltf_int16,
    gltf_uint16,
    gltf_uint32,
    gltf_float32
};

gltf_component_type GetGLTFComponentType(int Value) {
    switch(Value) {
        case 5120: { return gltf_int8; } break;
        case 5121: { return gltf_uint8; } break;
        case 5122: { return gltf_int16; } break;
        case 5123: { return gltf_uint16; } break;
        case 5125: { return gltf_uint32; } break;
        case 5126: { return gltf_float32; } break;
        default: { Raise("Invalid GLTF accessor component type."); }
    }
    return gltf_float32;
}

struct gltf_buffer {
    string Path;
    int ByteLength;
};

enum gltf_buffer_target {
    gltf_array_buffer,
    gltf_element_buffer
};

gltf_buffer_target GetGLTFBufferTarget(int Value) {
    if (Value == 34962) {
        return gltf_array_buffer;
    }
    else if (Value == 34963) {
        return gltf_element_buffer;
    }
    return gltf_array_buffer;
}

struct gltf_buffer_view {
    string Name;
    int Buffer;
    int ByteOffset;
    int ByteLength;
    int ByteStride;
    gltf_buffer_target Target;
};

enum gltf_data_type {
    gltf_scalar,
    gltf_vec2,
    gltf_vec3,
    gltf_vec4,
    gltf_mat2,
    gltf_mat3,
    gltf_mat4
};

gltf_data_type GetGLTFDataType(string DataType) {
    if (DataType == "SCALAR") {
        return gltf_scalar;
    }
    else if (DataType == "VEC2") {
        return gltf_vec2;
    }
    else if (DataType == "VEC3") {
        return gltf_vec3;
    }
    else if (DataType == "VEC4") {
        return gltf_vec4;
    }
    else if (DataType == "MAT2") {
        return gltf_mat2;
    }
    else if (DataType == "MAT3") {
        return gltf_mat3;
    }
    else if (DataType == "MAT4") {
        return gltf_mat4;
    }
    else {
        Raise("Invalid GLTF accessor data type.");
    }
    return gltf_scalar;
}

struct gltf_accessor {
    string Name;
    int BufferView;
    int ByteOffset;
    gltf_component_type ComponentType;
    gltf_data_type DataType;
    int Count;
    void* Min;
    void* Max;
    bool Normalized;
};

struct gltf_primitive_attribute {
    string Semantic;
    int Accessor;
};

render_primitive GetGLTFPrimitive(int Mode) {
    if (Mode == 0) {
        return render_primitive::POINT;
    }
    else if (Mode == 1) {
        return render_primitive::LINE;
    }
    else if (Mode == 3) {
        return render_primitive::LINE_STRIP;
    }
    else if (Mode == 4) {
        return render_primitive::TRIANGLE;
    }
    else if (Mode == 5) {
        return render_primitive::TRIANGLE_STRIP;
    }
    else {
        Raise("Invalid GPTF primitive mode.");
    }
    return render_primitive::POINT;
}

struct gltf_primitive {
    render_primitive Mode;
    int nAttributes;
    gltf_primitive_attribute* Attributes;
    int Indices;
    int Material;
};

struct gltf_mesh {
    string Name;
    int nPrimitives;
    gltf_primitive* Primitives;
    int nWeights;
    float* Weights;
};

struct gltf_asset {
    int LoadScene;
    int nScenes;
    game_scene* Scenes;
    int nNodes;
    game_scene_node* Nodes;
    int nMaterials;
    game_material* Materials;
    int nBuffers;
    gltf_buffer* Buffers;
    int nBufferViews;
    gltf_buffer_view* BufferViews;
    int nAccessors;
    gltf_accessor* Accessors;
    int nMeshes;
    gltf_mesh* Meshes;
};

gltf_asset ParseGLTF(memory_arena* Arena, char* Content) {
    tokenizer Tokenizer = InitTokenizer(Content);

    gltf_asset Result = {};
    
    gltf_entry* Root = PushStruct(Arena, gltf_entry);
    Root->Name = "ROOT";
    ParseGLTFStruct(Arena, Tokenizer, Root, nullptr);

    gltf_entry* Entry = Root->Children;
    for(int i = 0; i < Root->Length; i++) {
        string Name = Entry->Name;
        if (Name == "scene") {
            Result.LoadScene = Entry->Int;
        }
        else if (Name == "scenes") {
            Result.nScenes = Entry->Length;
            Result.Scenes = PushArray(Arena, Entry->Length, game_scene);
            gltf_entry* Child = Entry->Children;
            for (int j = 0; j < Result.nScenes; j++) {
                game_scene* Scene = Result.Scenes + j;
                gltf_entry* Key = Child->Children;
                for (int k = 0; k < Child->Length; k++) {
                    if (Key->Name == "nodes") {
                        Scene->nNodes = Key->Length;
                        Scene->Nodes = (int*)Key->ArrayData;
                    }
                    Key = Key->Next;
                }
                Child = Child->Next;
            }
        }
        else if (Name == "nodes") {
            Result.nNodes = Entry->Length;
            Result.Nodes = PushArray(Arena, Result.nNodes, game_scene_node);
            gltf_entry* Child = Entry->Children;
            for (int j = 0; j < Result.nNodes; j++) {
                game_scene_node* Node = Result.Nodes + j;
                gltf_entry* Key = Child->Children;
                for (int k = 0; k < Child->Length; k++) {
                    if (Key->Name == "children") {
                        Node->nChildren = Key->Length;
                        Node->Children = (int*)Key->ArrayData;
                    }
                    else if (Key->Name == "matrix") {
                        Assert(Key->Length == 16, "GLTF matrix field must have legth 16.");
                        float* Data = (float*)Key->ArrayData;
                        for (int l = 0; l < 16; l++) {
                            Node->Matrix.Array[l] = Data[l];
                        }
                    }
                    else if (Key->Name == "mesh") {
                        Node->Mesh = Key->Int;
                    }
                    Key = Key->Next;
                }
                Child = Child->Next;
            }
        }
        else if (Name == "materials") {
            Result.nMaterials = Entry->Length;
            Result.Materials = PushArray(Arena, Entry->Length, game_material);
            gltf_entry* Child = Entry->Children;
            for (int j = 0; j < Entry->Length; j++) {
                game_material* Material = Result.Materials + j;
                *Material = {};
                gltf_entry* Key = Child->Children;
                for (int k = 0; k < Child->Length; k++) {
                    if (Key->Name == "name") {
                        Material->Name = Key->String;
                    }
                    else if (Key->Name == "pbrMetallicRoughness") {
                        gltf_entry* Attribute = Key->Children;
                        for (int l = 0; l < Key->Length; l++) {
                            if (Attribute->Name == "baseColorFactor") {
                                float* Data = (float*)Attribute->ArrayData;
                                Material->Color.R = Data[0];
                                Material->Color.G = Data[1];
                                Material->Color.B = Data[2];
                                Material->Color.A = Data[3];
                            }
                            else if (Attribute->Name == "metallicFactor") {
                                Material->Metallicity = Attribute->Float;
                            }

                             Attribute = Attribute->Next;
                        }
                    }
                    else if (Key->Name == "normalTexture") {
                        Material->NormalTexture = Key->Int;
                    }
                    else if (Key->Name == "occlusionTexture") {
                        Material->OcclusionTexture = Key->Int;
                    }
                    else if (Key->Name == "emissiveTexture") {
                        Material->EmissiveTexture = Key->Int;
                    }
                    else if (Key->Name == "doubleSided") {
                        Material->DoubleSided = Key->Bool;
                    }                    
                    Key = Key->Next;
                }
                Child = Child->Next;
            }
        }
        else if (Name == "meshes") {
            Result.nMeshes = Entry->Length;
            Result.Meshes = PushArray(Arena, Entry->Length, gltf_mesh);
            gltf_entry* MeshEntry = Entry->Children;
            for (int j = 0; j < Entry->Length; j++) {
                gltf_mesh* Mesh = Result.Meshes + j;
                gltf_entry* Key = MeshEntry->Children;
                for (int k = 0; k < MeshEntry->Length; k++) {
                    if (Key->Name == "primitives") {
                        Mesh->nPrimitives = Key->Length;
                        Mesh->Primitives = PushArray(Arena, Key->Length, gltf_primitive);
                        gltf_entry* PrimitiveEntry = Key->Children;
                        for (int l = 0; l < Key->Length; l++) {
                            gltf_primitive* Primitive = Mesh->Primitives + l;
                            gltf_entry* PrimitiveKey = PrimitiveEntry->Children;
                            for (int m = 0; m < PrimitiveKey->Length; m++) {
                                if (PrimitiveKey->Name == "attributes") {
                                    Primitive->nAttributes = PrimitiveKey->Length;
                                    Primitive->Attributes = PushArray(Arena, PrimitiveKey->Length, gltf_primitive_attribute);
                                    gltf_entry* SemanticKey = PrimitiveKey->Children;
                                    for (int n = 0; n < PrimitiveKey->Length; n++) {
                                        gltf_primitive_attribute* Attribute = Primitive->Attributes + m;
                                        Attribute->Semantic = SemanticKey->Name;
                                        Attribute->Accessor = SemanticKey->Int;
                                        SemanticKey = SemanticKey->Next;
                                    }
                                }
                                else if (PrimitiveKey->Name == "indices") {
                                    Primitive->Indices = PrimitiveKey->Int;
                                }
                                else if (PrimitiveKey->Name == "mode") {
                                    Primitive->Mode = GetGLTFPrimitive(PrimitiveKey->Int);
                                }
                                else if (PrimitiveKey->Name == "material") {
                                    Primitive->Material = PrimitiveKey->Int;
                                }
                                PrimitiveKey = PrimitiveKey->Next;
                            }
                            PrimitiveEntry = PrimitiveEntry->Next;
                        }
                    }
                    else if (Key->Name == "name") {
                        Mesh->Name = Key->String;
                    }
                    Key = Key->Next;
                }
                MeshEntry = MeshEntry->Next;
            }
        }
        else if (Name == "accessors") {
            Result.nAccessors = Entry->Length;
            Result.Accessors = PushArray(Arena, Entry->Length, gltf_accessor);
            gltf_entry* AccessorEntry = Entry->Children;
            for (int j = 0; j < Entry->Length; j++) {
                gltf_accessor* Accessor = Result.Accessors + j;
                gltf_entry* Key = AccessorEntry->Children;
                for (int k = 0; k < AccessorEntry->Length; k++) {
                    if (Key->Name == "bufferView") {
                        Accessor->BufferView = Key->Int;
                    }
                    else if (Key->Name == "byteOffset") {
                        Accessor->ByteOffset = Key->Int;
                    }
                    else if (Key->Name == "componentType") {
                        Accessor->ComponentType = GetGLTFComponentType(Key->Int);
                    }
                    else if (Key->Name == "count") {
                        Accessor->Count = Key->Int;
                    }
                    else if (Key->Name == "type") {
                        Accessor->DataType = GetGLTFDataType(Key->String);
                    }
                    else if (Key->Name == "min") {
                        Accessor->Min = Key->ArrayData;
                    }
                    else if (Key->Name == "max") {
                        Accessor->Max = Key->ArrayData;
                    }

                    Key = Key->Next;
                }
                AccessorEntry = AccessorEntry->Next;
            }
        }
        else if (Name == "buffers") {
            Result.nBuffers = Entry->Length;
            Result.Buffers = PushArray(Arena, Entry->Length, gltf_buffer);
            gltf_entry* BufferEntry = Entry->Children;
            for (int j = 0; j < Entry->Length; j++) {
                gltf_buffer* Buffer = Result.Buffers + j;
                int nKeys = BufferEntry->Length;
                gltf_entry* Key = BufferEntry->Children;
                for(int k = 0; k < BufferEntry->Length; k++) {
                    if (Key->Name == "byteLength") {
                        Buffer->ByteLength = Key->Int;
                    }
                    else if (Key->Name == "uri") {
                        Buffer->Path = Key->String;
                    }
                    Key = Key->Next;
                }
                BufferEntry = BufferEntry->Next;
            }
        }
        else if (Name == "bufferViews") {
            Result.nBufferViews = Entry->Length;
            Result.BufferViews = PushArray(Arena, Entry->Length, gltf_buffer_view);
            gltf_entry* BufferViewEntry = Entry->Children;
            for (int j = 0; j < Entry->Length; j++) {
                gltf_buffer_view* BufferView = Result.BufferViews + j;
                gltf_entry* Key = BufferViewEntry->Children;
                for (int k = 0; k < BufferViewEntry->Length; k++) {
                    if (Key->Name == "buffer") {
                        BufferView->Buffer = Key->Int;
                    }
                    else if (Key->Name == "byteOffset") {
                        BufferView->ByteOffset = Key->Int;
                    }
                    else if (Key->Name == "byteLength") {
                        BufferView->ByteLength = Key->Int;
                    }
                    else if (Key->Name == "byteStride") {
                        BufferView->ByteStride = Key->Int;
                    }
                    else if (Key->Name == "target") {
                        BufferView->Target = GetGLTFBufferTarget(Key->Int);
                    }
                    Key = Key->Next;
                }
                BufferViewEntry = BufferViewEntry->Next;
            }
        }
        Entry = Entry->Next;
    }

    return Result;
}
