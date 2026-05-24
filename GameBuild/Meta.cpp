#include <stdio.h>
#include <stdlib.h>

#include "Tokenizer.h"
#include <map>
#include <vector>
#include <string>
#include <cstring>

#if _WIN32
#define PATH_SEPARATOR "\\"
#elif __linux__
#define PATH_SEPARATOR "/"
#endif

#define ArrayCount(arr) (sizeof((arr)) / sizeof((arr)[0]))

FILE* OpenFile(const char* Path, const char* Permissions) {
    FILE* File = fopen(Path, Permissions);
    if (!File) {
        perror("Failed to open file");
        abort();
    }
    fseek(File, 0, SEEK_SET);
    return File;
}

char* ReadFile(const char* Path, size_t* FileSize = nullptr) {
    FILE* File = OpenFile(Path, "r");

    fseek(File, 0, SEEK_END);
    size_t Size = ftell(File);
    if (FileSize) {
        *FileSize = Size;
    }
    fseek(File, 0, SEEK_SET);

    char* Result = (char*)malloc(Size + 1);
    fread(Result, Size, 1, File);
    Result[Size] = 0;
    fclose(File);
    return Result;
}

const char* const PrimitiveTypes[] = {
    "bool",
    "char",
    "string",
    "int8",
    "int16",
    "int",
    "int32",
    "int64",
    "uint8",
    "uint16",
    "uint32",
    "uint64",
    "memory_index",
    "float",
    "double",
    "v2",
    "v3",
    "v4",
    "scale",
    "quaternion",
    "color",
    "collider",
    "memory_arena"
};
const int nPrimitiveTypes = ArrayCount(PrimitiveTypes);

int main() {
    FILE* EnumsFile = OpenFile("GameLibrary" PATH_SEPARATOR "GameEnums.h", "w");
    FILE* StructsFile = OpenFile("GameLibrary" PATH_SEPARATOR "GameStructs.h", "w");

    fprintf(EnumsFile,
    "#ifndef GAME_ENUMS\n"
    "#define GAME_ENUMS\n\n"
    );

    fprintf(EnumsFile, "enum debug_type {\n");
    for (int i = 0; i < nPrimitiveTypes; i++) {
        fprintf(EnumsFile, "    debug_%s,\n", PrimitiveTypes[i]);
    }

    std::vector<std::string> EnumDebugTypes = {};
    std::vector<std::string> EnumValues = {};
    std::vector<std::string> FlagDebugTypes = {};
    std::vector<std::string> FlagValues = {};
    std::string FlagDeclarations = "";
    unsigned int nFlagTypes = 0;
    std::vector<std::string> StructDebugTypes = {};
    std::vector<std::string> StructMembers = {};

    const char* ProcessingFiles[] = {
        "GameAsset"   PATH_SEPARATOR "GameAsset.h",
        "GameAsset"   PATH_SEPARATOR "GameMesh.h",
        "GameAsset"   PATH_SEPARATOR "GameTexture.h",
        "GameRender"  PATH_SEPARATOR "GameRender.h",
        "GameLibrary" PATH_SEPARATOR "GameMath.h",
        "GameLibrary" PATH_SEPARATOR "GameState.h",
        "GameLibrary" PATH_SEPARATOR "GameUI.h",
    };

    char Buffer[256];
    std::map<std::string, int> Constants;
    for (int i = 0; i < ArrayCount(ProcessingFiles); i++) {
        char* FileContent = ReadFile(ProcessingFiles[i]);

        tokenizer Tokenizer = InitTokenizer(FileContent);
        token Token = GetToken(Tokenizer);
        while (Token.Type != Token_End) {
            if (Token == "const") {
                Token = GetToken(Tokenizer);
                if (Token == "int") {
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    std::string ConstantName(Token.Text, Token.Length);
                    Token = RequireToken(Tokenizer, Token_Equal);
                    int Value = ParseInt(Tokenizer);
                    Constants[ConstantName] = Value;
                }
                else if (Token == "int32") {
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    std::string ConstantName(Token.Text, Token.Length);
                    Token = RequireToken(Tokenizer, Token_Equal);
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    std::string FirstAddend(Token.Text, Token.Length);
                    int Value = Constants[FirstAddend];
                    Token = RequireToken(Tokenizer, Token_Plus);
                    while (Token.Type != Token_Semicolon) {
                        if (Token.Type != Token_Plus) {
                            throw "Should be a plus sign.";
                        };
                        Token = RequireToken(Tokenizer, Token_Identifier);
                        std::string Addend(Token.Text, Token.Length);
                        Value += Constants[Addend];
                        Token = GetToken(Tokenizer);
                    }
                    Constants[ConstantName] = Value;
                }
            }
            else if (Token == "INTROSPECT") {
                Token = GetToken(Tokenizer);
                if (Token == "struct") {
                    token StructType = RequireToken(Tokenizer, Token_Identifier);
                    char StructTypeText[MAX_TOKEN_LENGTH] = {};
                    strncpy(StructTypeText, StructType.Text, StructType.Length);

                    sprintf(Buffer, "debug_%s", StructTypeText);
                    StructDebugTypes.push_back(std::string(Buffer));

                    Token = RequireToken(Tokenizer, Token_OpenBrace);
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    while (Token.Type != Token_CloseBrace && Token.Type != Token_End) {
                        token MemberType = Token;
                        token MemberName = GetToken(Tokenizer);
                        bool IsPointer = false;
                        if (MemberName.Type == Token_Asterisk) {
                            MemberName = GetToken(Tokenizer);
                            IsPointer = true;
                        }
                        Token = GetToken(Tokenizer);
                        int ArraySize = 0;
                        if (Token.Type == Token_OpenBracket) {
                            Token = GetToken(Tokenizer);
                            if (Token.Type == Token_Identifier) {
                                ArraySize = Constants[std::string(Token.Text, Token.Length)];
                            }
                            else if (Token.Type == Token_Constant_Integer) {
                                char* End;
                                ArraySize = strtol(Token.Text, &End, 10);
                            }
                            Token = RequireToken(Tokenizer, Token_CloseBracket);
                            Token = RequireToken(Tokenizer, Token_Semicolon);
                        }
                        char MemberTypeDebugText[MAX_TOKEN_LENGTH] = {};
                        char MemberTypeText[MAX_TOKEN_LENGTH] = {};
                        if (MemberType == "char" && (ArraySize > 0 || IsPointer)) {
                            strcpy(MemberTypeDebugText, "string");
                            strcpy(MemberTypeText, "char");
                            ArraySize = 0;
                        }
                        else {
                            strncpy(MemberTypeDebugText, MemberType.Text, MemberType.Length);
                            strncpy(MemberTypeText, MemberType.Text, MemberType.Length);
                        }
                        char MemberNameText[MAX_TOKEN_LENGTH] = {};
                        strncpy(MemberNameText, MemberName.Text, MemberName.Length);
                        sprintf(Buffer, "    {\"%s\", debug_%s, debug_%s, sizeof(%s), (uint64)(&((%s*)0)->%s),%d, %s},\n", 
                            MemberNameText, StructTypeText, MemberTypeDebugText, MemberTypeText, StructTypeText, MemberNameText,
                            ArraySize, IsPointer ? "true" : "false");
                        std::string StructMember = Buffer;
                        StructMembers.push_back(StructMember);
                        while (Token.Type != Token_Semicolon && Token.Type != Token_End) {
                            Token = GetToken(Tokenizer);
                        };
                        Token = GetToken(Tokenizer);
                    }
                }
            }
            else if (Token == "ENUM") {
                RequireToken(Tokenizer, Token_OpenParen);

                token EnumName = RequireToken(Tokenizer, Token_Identifier);
                char EnumNameText[MAX_TOKEN_LENGTH] = {};
                strncpy(EnumNameText, EnumName.Text, EnumName.Length);
                sprintf(Buffer, "debug_%s", EnumNameText);
                EnumDebugTypes.push_back(std::string(Buffer));

                Token = RequireToken(Tokenizer, Token_Comma);

                int Value = 0;
                while(Token.Type != Token_CloseParen) {
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    char TokenText[MAX_TOKEN_LENGTH] = {};
                    strncpy(TokenText, Token.Text, Token.Length);
                    sprintf(Buffer, "    {debug_%s, \"%s\", %d},\n", EnumNameText, TokenText, Value++);
                    EnumValues.push_back(std::string(Buffer));
                    Token = GetToken(Tokenizer);
                    if (Token.Type != Token_CloseParen && Token.Type != Token_Comma) {
                        throw "Should be a comma.";
                    };
                }
                sprintf(Buffer, "    {debug_%s, \"%s_count\", %d},\n", EnumNameText, EnumNameText, Value);
                EnumValues.push_back(std::string(Buffer));
                sprintf(Buffer, "%s_count", EnumNameText);
                Constants[std::string(Buffer)] = Value;
            }
            else if (Token == "FLAGS") {
                RequireToken(Tokenizer, Token_OpenParen);

                token FlagsName = RequireToken(Tokenizer, Token_Identifier);
                char FlagsNameText[MAX_TOKEN_LENGTH] = {};
                strncpy(FlagsNameText, FlagsName.Text, FlagsName.Length);
                FlagDebugTypes.push_back(std::string(FlagsNameText));

                sprintf(Buffer, "enum class %s : uint64 {\n    none = 0,\n", FlagsNameText);
                FlagDeclarations += std::string(Buffer);

                RequireToken(Tokenizer, Token_Comma);

                int Bit = 0;
                while(Token.Type != Token_CloseParen) {
                    token FlagValue = RequireToken(Tokenizer, Token_Identifier);
                    char FlagValueText[MAX_TOKEN_LENGTH] = {};
                    strncpy(FlagValueText, FlagValue.Text, FlagValue.Length);
                    Token = GetToken(Tokenizer);
                    if (Token.Type == Token_Comma || Token.Type == Token_CloseParen) {
                        sprintf(Buffer, "    {debug_type::debug_%s, \"%s\", %d},\n", FlagsNameText, FlagValueText, 1 << Bit);
                        FlagValues.push_back(std::string(Buffer));
                        sprintf(Buffer, "    %s = 1 << %d,\n", FlagValueText, Bit);
                        FlagDeclarations += std::string(Buffer);
                        Bit++;
                    }
                    else if (Token.Type == Token_Equal) {
                        FlagDeclarations += std::string("    ") + std::string(FlagValueText) + std::string(" = ");
                        Token = GetToken(Tokenizer);
                        while (Token.Type != Token_Comma && Token.Type != Token_CloseParen) {
                            FlagDeclarations += std::string(Token.Text, Token.Length);
                            Token = GetToken(Tokenizer);
                        }
                        FlagDeclarations += std::string(",\n");
                    }
                }
                FlagDeclarations += std::string("};\n\n");
                nFlagTypes += 1;
            }

            Token = GetToken(Tokenizer);
        }

        free(FileContent);
    }

    for (const std::string& EnumName : EnumDebugTypes) {
        fprintf(EnumsFile, "    %s,\n", EnumName.c_str());
    }

    for (const std::string& FlagName : FlagDebugTypes) {
        fprintf(EnumsFile, "    debug_%s,\n", FlagName.c_str());
    }

    for (const std::string& StructName : StructDebugTypes) {
        fprintf(EnumsFile, "    %s,\n", StructName.c_str());
    }

    fprintf(EnumsFile, "};\n\n");

    fprintf(EnumsFile, "%s", FlagDeclarations.c_str());

    int nEnums = EnumDebugTypes.size();
    fprintf(EnumsFile, "bool IsEnumType(debug_type Type) { return Type > %d && Type < %d; }\n", 
        nPrimitiveTypes - 1, nPrimitiveTypes + nEnums
    );

    fprintf(EnumsFile, "bool IsFlagType(debug_type Type) { return Type > %d && Type < %d; }\n\n",
        nPrimitiveTypes + nEnums - 1, nPrimitiveTypes + nEnums + nFlagTypes
    );

    fprintf(EnumsFile,
    "struct debug_enum_value {\n"
    "    debug_type EnumType;\n"
    "    const char* Identifier;\n"
    "    int Value;\n"
    "};\n\n");

    fprintf(EnumsFile, "const int ENUM_VALUES_SIZE = %d;\n", (int)EnumValues.size());
    if (EnumValues.size() > 0) {
        fprintf(EnumsFile, "debug_enum_value EnumValues[ENUM_VALUES_SIZE] = {\n");

        for (const std::string& EnumValue : EnumValues) {
            fprintf(EnumsFile, "%s", EnumValue.c_str());
        }

        fprintf(EnumsFile, "};\n\n");
    }
    else {
        fprintf(EnumsFile, "debug_enum_value* EnumValues = 0;\n\n");
    }

    fprintf(EnumsFile, "const int FLAG_VALUES_SIZE = %d;\n", (int)FlagValues.size());
    if (FlagValues.size() > 0) {
        fprintf(EnumsFile, "debug_enum_value FlagValues[FLAG_VALUES_SIZE] = {\n");

        for (const std::string& FlagValue : FlagValues) {
            fprintf(EnumsFile, "%s", FlagValue.c_str());
        }

        fprintf(EnumsFile, "};\n\n");

        for (const std::string& FlagName : FlagDebugTypes) {
            fprintf(EnumsFile, "constexpr %s operator|(%s a, %s b) { return (%s)((uint64)a | (uint64)b); }\n", 
                FlagName.c_str(), FlagName.c_str(), FlagName.c_str(), FlagName.c_str());
            fprintf(EnumsFile, "constexpr %s& operator|=(%s& a, %s b) { return a = a | b; }\n", 
                FlagName.c_str(), FlagName.c_str(), FlagName.c_str());
            fprintf(EnumsFile, "constexpr bool operator&(%s a, %s b) { return (uint64)a & (uint64)b; }\n", 
                FlagName.c_str(), FlagName.c_str());
            fprintf(EnumsFile, "constexpr %s operator~(%s a) { return (%s)(~(uint64)a); }\n\n", 
                FlagName.c_str(), FlagName.c_str(), FlagName.c_str());
        }
    }
    else {
        fprintf(EnumsFile, "debug_enum_value* FlagValues = nullptr;\n\n");
    }

    fprintf(EnumsFile, "#endif");

    int nStructs = StructDebugTypes.size();
    fprintf(StructsFile, "bool IsStructType(debug_type Type) { return Type > %d && Type < %d; }\n\n", 
        nPrimitiveTypes + nEnums + nFlagTypes - 1, nPrimitiveTypes + nEnums + nFlagTypes + nStructs
    );

    fprintf(StructsFile,
    "struct debug_struct_member {\n"
    "    const char* Name;\n"
    "    debug_type StructType;\n"
    "    debug_type MemberType;\n"
    "    uint64 Size;\n"
    "    uint64 Offset;\n"
    "    int ArraySize;\n"
    "    bool IsPointer;\n"
    "};\n\n");

    fprintf(StructsFile, "const int STRUCT_MEMBERS_SIZE = %d;\n", (int)StructMembers.size());
    if (StructMembers.size() > 0) {
        fprintf(StructsFile, "debug_struct_member StructMembers[STRUCT_MEMBERS_SIZE] = {\n");

        for (const std::string& StructMember : StructMembers) {
            fprintf(StructsFile, "%s", StructMember.c_str());
        }

        fprintf(StructsFile, "};\n");
    }
    else {
        fprintf(StructsFile, "debug_struct_member* StructMembers = 0;\n\n");
    }

    fclose(StructsFile);
    fclose(EnumsFile);

    // Tests

    size_t TestsFileSize;
    char* TestsContent = ReadFile("GameTest" PATH_SEPARATOR "Tests.h", &TestsFileSize);
    FILE* TestOutputFile = OpenFile("GameTest" PATH_SEPARATOR "GameTest.h", "w");

    fprintf(TestOutputFile, 
    "#include \"Tests.h\"\n\n");

    std::string MemorySetup = "    render_group* Group = &Memory->RenderGroup;\n"
    "    game_state* State = Memory->GameState;\n"
    "    memory_arena* Permanent = &Memory->Permanent;\n"
    "    memory_arena* Transient = &Memory->Transient;\n"
    "    game_input* Input = &Memory->Input;\n"
    "    light* Light = &Memory->RenderGroup.Light;\n"
    "    float Time = State->Time;\n"
    "    bool Result = false;\n";

    std::string OnceTests = std::string("void RunOnceTests(game_memory* Memory) {\n") + MemorySetup;
    std::string ReloadTests = std::string("void RunReloadTests(game_memory* Memory) {\n" + MemorySetup);
    std::string EveryFrameTests = std::string("void RunEveryFrameTests(game_memory* Memory) {\n" + MemorySetup);

    tokenizer Tokenizer = InitTokenizer(TestsContent, TestsFileSize);
    token Token = GetToken(Tokenizer);
    while (Token.Type != Token_End) {
        if (Token.Type == Token_Pound) {
            AdvanceUntilNextLine(Tokenizer);
        }
        else if (Token == "TEST") {
            RequireToken(Tokenizer, Token_OpenParen);
            token TestNameToken = RequireToken(Tokenizer, Token_Identifier);

            RequireToken(Tokenizer, Token_Comma);
            token TestType = RequireToken(Tokenizer, Token_Identifier);
            
            RequireToken(Tokenizer, Token_Comma);
            bool TestActive = false;
            bool MustPass = false;
            token Separator;
            do {
                token OptionalFlag = GetToken(Tokenizer);
                if (OptionalFlag == "ACTIVE") {
                    TestActive = true;
                }
                else if (OptionalFlag == "INACTIVE") {
                    TestActive = false;
                }
                else if (OptionalFlag == "MUST_PASS") {
                    MustPass = true;
                }
                else {
                    throw "Invalid optional flag.";
                }
                Separator = GetToken(Tokenizer);
            } while(Separator.Type == Token_Comma);

            if (TestActive) {
                std::string TestName = std::string(TestNameToken.Text, TestNameToken.Length);
                std::string* Target = nullptr;
                if (TestType == "test_once") {
                    Target = &OnceTests;
                }
                else if (TestType == "test_reload") {
                    Target = &ReloadTests;
                }
                else if (TestType == "test_every_frame") {
                    Target = &EveryFrameTests;
                }

                *Target += std::string("\n    ");
                if (!MustPass) {
                    *Target += std::string("try { ");
                }
                *Target += std::string("Result = ") + TestName + std::string("(");
                
                token PreviousToken = RequireToken(Tokenizer, Token_OpenParen);
                Token = GetToken(Tokenizer);
                while (Token.Type != Token_CloseParen) {
                    PreviousToken = Token;
                    Token = GetToken(Tokenizer);
                    if (Token.Type == Token_Comma) {
                        *Target += std::string(PreviousToken.Text, PreviousToken.Length) + std::string(", ");
                    }
                    else if (Token.Type == Token_CloseParen) {
                        *Target += std::string(PreviousToken.Text, PreviousToken.Length);
                    }
                }
                *Target += std::string(");");
                if (MustPass) {
                    *Target += "\n";
                }
                else {
                    *Target += std::string(" }\n"
                    "    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); Result = false; }\n"
                    "    catch(...) { Log(log_level::Error, \"Unknown test error.\"); Result = false; }\n"
                    );

                    OnceTests += std::string("    if(Result) { Log(log_level::Test, \"Test '") + TestName + std::string("' was correctly executed.\"); }\n");
                    OnceTests += std::string("    else       { Log(log_level::Error, \"Test '") + TestName + std::string("' failed.\"); }\n");
                }
            }
        }

        Token = GetToken(Tokenizer);
    }
    OnceTests += std::string("}");
    ReloadTests += std::string("}");
    EveryFrameTests += std::string("}");
    fprintf(TestOutputFile,"%s\n\n%s\n\n%s", OnceTests.c_str(), ReloadTests.c_str(), EveryFrameTests.c_str());

    fclose(TestOutputFile);

    return 0;
}