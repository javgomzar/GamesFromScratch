#include <stdio.h>
#include <stdlib.h>

#include "Tokenizer.h"
#include <map>
#include <vector>
#include <string>

FILE* OpenFile(const char* Path, const char* Permissions) {
    FILE* File = fopen(Path, Permissions);
    if (File == NULL) {
        perror("Failed to open file");
        abort();
    }
    fseek(File, 0, SEEK_SET);
    return File;
}

char* ReadFile(const char* Path) {
    FILE* File = OpenFile(Path, "r");

    fseek(File, 0, SEEK_END);
    size_t FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    char* Result = (char*)malloc(FileSize + 1);
    fread(Result, FileSize, 1, File);
    Result[FileSize] = 0;
    return Result;
}

int main() {
    FILE* DebugTypesFile = OpenFile("..\\GameLibrary\\GameDebugTypes.h", "w");

    const char* PrimitiveTypes[] = {
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
    int nPrimitiveTypes = ArrayCount(PrimitiveTypes);

    fprintf(DebugTypesFile, "enum debug_type {\n");
    for (int i = 0; i < nPrimitiveTypes; i++) {
        fprintf(DebugTypesFile, "    Debug_Type_%s,\n", PrimitiveTypes[i]);
    }

    std::vector<std::string> EnumDebugTypes = {};
    std::vector<std::string> EnumValues = {};
    std::vector<std::string> StructDebugTypes = {};
    std::vector<std::string> StructMembers = {};

    const char* ProcessingFiles[] = {
        "..\\GameLibrary\\GamePlatform.h",
        "..\\GameLibrary\\GameMath.h",
        "..\\GameAssets\\GameAssets.h",
        "..\\GameLibrary\\GameEntity.h",
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
                    std::string ConstantName(Token.Text);
                    Token = RequireToken(Tokenizer, Token_Equal);
                    int Value = ParseInt(Tokenizer);
                    Constants[ConstantName] = Value;
                }
                else if (Token == "int32") {
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    std::string ConstantName(Token.Text);
                    Token = RequireToken(Tokenizer, Token_Equal);
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    std::string FirstAddend(Token.Text);
                    int Value = Constants[FirstAddend];
                    Token = RequireToken(Tokenizer, Token_Plus);
                    while (Token.Type != Token_Semicolon) {
                        Assert(Token.Type == Token_Plus);
                        Token = RequireToken(Tokenizer, Token_Identifier);
                        std::string Addend(Token.Text);
                        Value += Constants[Addend];
                        Token = GetToken(Tokenizer);
                    }
                    Constants[ConstantName] = Value;
                }
            }
            if (Token == "INTROSPECT") {
                Token = GetToken(Tokenizer);
                if (Token == "struct") {
                    token StructType = RequireToken(Tokenizer, Token_Identifier);

                    sprintf_s(Buffer, "Debug_Type_%s", StructType.Text);
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
                                ArraySize = Constants[std::string(Token.Text)];
                            }
                            else if (Token.Type == Token_Constant_Integer) {
                                tokenizer Parser = InitTokenizer(Token.Text);
                                ArraySize = ParseInt(Parser);
                            }
                            Token = RequireToken(Tokenizer, Token_CloseBracket);
                            Token = RequireToken(Tokenizer, Token_Semicolon);
                        }
                        const char* MemberTypeText = MemberType.Text;
                        if (MemberType == "char" && (ArraySize > 0 || IsPointer)) {
                            MemberTypeText = "string";
                            ArraySize = 0;
                        }
                        sprintf_s(Buffer, "    {\"%s\", Debug_Type_%s, Debug_Type_%s, sizeof(%s), (uint64)(&((%s*)0)->%s),%d, %s},\n", 
                            MemberName.Text, StructType.Text, MemberTypeText, MemberType.Text, StructType.Text, MemberName.Text,
                            ArraySize, IsPointer ? "true" : "false");
                        std::string StructMember = Buffer;
                        StructMembers.push_back(StructMember);
                        Assert(Token.Type == Token_Semicolon);
                        Token = GetToken(Tokenizer);
                    }
                }
                else if (Token == "enum") {
                    token EnumName = RequireToken(Tokenizer, Token_Identifier);
                    sprintf_s(Buffer, "Debug_Type_%s", EnumName.Text);
                    EnumDebugTypes.push_back(std::string(Buffer));
                    Token = RequireToken(Tokenizer, Token_OpenBrace);
                    int Value = 0;
                    while(Token.Type != Token_CloseBrace) {
                        Token = RequireToken(Tokenizer, Token_Identifier);
                        sprintf_s(Buffer, "    {Debug_Type_%s, \"%s\", %d},\n", EnumName.Text, Token.Text, Value++);
                        EnumValues.push_back(std::string(Buffer));
                        Token = GetToken(Tokenizer);
                        if (Token.Type != Token_CloseBrace) Assert(Token.Type == Token_Comma);
                    }
                }
            }

            Token = GetToken(Tokenizer);
        }
    }

    for (const std::string& EnumName : EnumDebugTypes) {
        fprintf(DebugTypesFile, "    %s,\n", EnumName.c_str());
    }

    for (const std::string& StructName : StructDebugTypes) {
        fprintf(DebugTypesFile, "    %s,\n", StructName.c_str());
    }

    fprintf(DebugTypesFile, "};\n\n");

    int nEnums = EnumDebugTypes.size();
    fprintf(DebugTypesFile, "bool IsEnumType(debug_type Type) { return Type > %d && Type < %d; }\n", 
        nPrimitiveTypes - 1, nPrimitiveTypes + nEnums
    );

    int nStructs = StructDebugTypes.size();
    fprintf(DebugTypesFile, "bool IsStructType(debug_type Type) { return Type > %d && Type < %d; }\n\n", 
        nPrimitiveTypes + nEnums - 1, nPrimitiveTypes + nEnums + nStructs
    );

    fprintf(DebugTypesFile,
    "struct debug_enum_value {\n"
    "    debug_type EnumType;\n"
    "    const char* Identifier;\n"
    "    int Value;\n"
    "};\n\n");

    fprintf(DebugTypesFile, "const int ENUM_VALUES_SIZE = %d;\n", (int)EnumValues.size());
    if (EnumValues.size() > 0) {
        fprintf(DebugTypesFile, "debug_enum_value EnumValues[ENUM_VALUES_SIZE] = {\n", (int)EnumValues.size());

        for (const std::string& EnumValue : EnumValues) {
            fprintf(DebugTypesFile, "%s", EnumValue.c_str());
        }

        fprintf(DebugTypesFile, "};\n\n");
    }
    else {
        fprintf(DebugTypesFile, "debug_enum_value* EnumValues = 0;\n\n");
    }

    fprintf(DebugTypesFile,
    "struct debug_struct_member {\n"
    "    const char* Name;\n"
    "    debug_type StructType;\n"
    "    debug_type MemberType;\n"
    "    uint64 Size;\n"
    "    uint64 Offset;\n"
    "    int ArraySize;\n"
    "    bool IsPointer;\n"
    "};\n\n");

    fprintf(DebugTypesFile, "const int STRUCT_MEMBERS_SIZE = %d;\n", (int)StructMembers.size());
    if (StructMembers.size() > 0) {
        fprintf(DebugTypesFile, "debug_struct_member StructMembers[STRUCT_MEMBERS_SIZE] = {\n", (int)StructMembers.size());

        for (const std::string& StructMember : StructMembers) {
            fprintf(DebugTypesFile, "%s", StructMember.c_str());
        }

        fprintf(DebugTypesFile, "};\n");
    }
    else {
        fprintf(DebugTypesFile, "debug_struct_member* StructMembers = 0;\n\n");
    }

    fclose(DebugTypesFile);
}