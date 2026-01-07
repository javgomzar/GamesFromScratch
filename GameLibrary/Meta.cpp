#include <stdio.h>
#include <stdlib.h>

#include "Tokenizer.h"
#include <map>
#include <vector>
#include <string>

#define ArrayCount(arr) (sizeof((arr)) / sizeof((arr)[0]))

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
    FILE* EnumsFile = OpenFile("GameLibrary\\GameEnums.h", "w");
    FILE* StructsFile = OpenFile("GameLibrary\\GameStructs.h", "w");

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

    fprintf(EnumsFile,
    "#ifndef GAME_ENUMS\n"
    "#define GAME_ENUMS\n\n"
    );

    fprintf(EnumsFile, "enum debug_type {\n");
    for (int i = 0; i < nPrimitiveTypes; i++) {
        fprintf(EnumsFile, "    Debug_Type_%s,\n", PrimitiveTypes[i]);
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
        "GameLibrary\\GameMath.h",
        "GameAssets\\GameAssets.h",
        "GameLibrary\\GameState.h",
        "GameLibrary\\GameRender.h",
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
                    strncpy_s(StructTypeText, MAX_TOKEN_LENGTH, StructType.Text, StructType.Length);

                    sprintf_s(Buffer, "Debug_Type_%s", StructTypeText);
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
                                tokenizer Parser = InitTokenizer(Token.Text);
                                ArraySize = ParseInt(Parser);
                            }
                            Token = RequireToken(Tokenizer, Token_CloseBracket);
                            Token = RequireToken(Tokenizer, Token_Semicolon);
                        }
                        char MemberTypeDebugText[MAX_TOKEN_LENGTH] = {};
                        char MemberTypeText[MAX_TOKEN_LENGTH] = {};
                        if (MemberType == "char" && (ArraySize > 0 || IsPointer)) {
                            strcpy_s(MemberTypeDebugText, "string");
                            strcpy_s(MemberTypeText, "char");
                            ArraySize = 0;
                        }
                        else {
                            strncpy_s(MemberTypeDebugText, MAX_TOKEN_LENGTH, MemberType.Text, MemberType.Length);
                            strncpy_s(MemberTypeText, MAX_TOKEN_LENGTH, MemberType.Text, MemberType.Length);
                        }
                        char MemberNameText[MAX_TOKEN_LENGTH] = {};
                        strncpy_s(MemberNameText, MAX_TOKEN_LENGTH, MemberName.Text, MemberName.Length);
                        sprintf_s(Buffer, "    {\"%s\", Debug_Type_%s, Debug_Type_%s, sizeof(%s), (uint64)(&((%s*)0)->%s),%d, %s},\n", 
                            MemberNameText, StructTypeText, MemberTypeDebugText, MemberTypeText, StructTypeText, MemberNameText,
                            ArraySize, IsPointer ? "true" : "false");
                        std::string StructMember = Buffer;
                        StructMembers.push_back(StructMember);
                        if (Token.Type != Token_Semicolon) {
                            throw "Should be a semicolon.";
                        };
                        Token = GetToken(Tokenizer);
                    }
                }
            }
            else if (Token == "ENUM") {
                RequireToken(Tokenizer, Token_OpenParen);

                token EnumName = RequireToken(Tokenizer, Token_Identifier);
                char EnumNameText[MAX_TOKEN_LENGTH] = {};
                strncpy_s(EnumNameText, MAX_TOKEN_LENGTH, EnumName.Text, EnumName.Length);
                sprintf_s(Buffer, "Debug_Type_%s", EnumNameText);
                EnumDebugTypes.push_back(std::string(Buffer));

                Token = RequireToken(Tokenizer, Token_Comma);

                int Value = 0;
                while(Token.Type != Token_CloseParen) {
                    Token = RequireToken(Tokenizer, Token_Identifier);
                    char TokenText[MAX_TOKEN_LENGTH] = {};
                    strncpy_s(TokenText, MAX_TOKEN_LENGTH, Token.Text, Token.Length);
                    sprintf_s(Buffer, "    {Debug_Type_%s, \"%s\", %d},\n", EnumNameText, TokenText, Value++);
                    EnumValues.push_back(std::string(Buffer));
                    Token = GetToken(Tokenizer);
                    if (Token.Type != Token_CloseParen && Token.Type != Token_Comma) {
                        throw "Should be a comma.";
                    };
                }
                sprintf_s(Buffer, "    {Debug_Type_%s, \"%s_count\", %d},\n", EnumNameText, EnumNameText, Value);
                EnumValues.push_back(std::string(Buffer));
                sprintf_s(Buffer, "%s_count", EnumNameText);
                Constants[std::string(Buffer)] = Value;
            }
            else if (Token == "FLAGS") {
                RequireToken(Tokenizer, Token_OpenParen);

                token FlagsName = RequireToken(Tokenizer, Token_Identifier);
                char FlagsNameText[MAX_TOKEN_LENGTH] = {};
                strncpy_s(FlagsNameText, MAX_TOKEN_LENGTH, FlagsName.Text, FlagsName.Length);
                sprintf_s(Buffer, "Debug_Type_%s", FlagsNameText);
                FlagDebugTypes.push_back(std::string(Buffer));

                sprintf_s(Buffer, "enum %s {\n", FlagsNameText);
                FlagDeclarations += std::string(Buffer);

                RequireToken(Tokenizer, Token_Comma);

                int Bit = 0;
                while(Token.Type != Token_CloseParen) {
                    token FlagValue = RequireToken(Tokenizer, Token_Identifier);
                    char FlagValueText[MAX_TOKEN_LENGTH] = {};
                    strncpy_s(FlagValueText, MAX_TOKEN_LENGTH, FlagValue.Text, FlagValue.Length);
                    Token = GetToken(Tokenizer);
                    if (Token.Type == Token_Comma || Token.Type == Token_CloseParen) {
                        sprintf_s(Buffer, "    {Debug_Type_%s, \"%s\", %d},\n", FlagsNameText, FlagValueText, 1 << Bit);
                        FlagValues.push_back(std::string(Buffer));
                        sprintf_s(Buffer, "    %s = 1 << %d,\n", FlagValueText, Bit);
                        FlagDeclarations += std::string(Buffer);
                        Bit++;
                    }
                    else if (Token.Type == Token_Equal) {
                        FlagDeclarations += std::string("    ") + std::string(FlagValueText);
                        Token = GetToken(Tokenizer);
                        while (Token.Type != Token_Comma && Token.Type != Token_CloseParen) {
                            FlagDeclarations += std::string(Token.Text, Token.Length);
                        }
                        FlagDeclarations += std::string(",\n");
                    }
                }
                FlagDeclarations += std::string("};\n\n");
                nFlagTypes += 1;
            }

            Token = GetToken(Tokenizer);
        }
    }

    for (const std::string& EnumName : EnumDebugTypes) {
        fprintf(EnumsFile, "    %s,\n", EnumName.c_str());
    }

    for (const std::string& FlagName : FlagDebugTypes) {
        fprintf(EnumsFile, "    %s,\n", FlagName.c_str());
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
    }
    else {
        fprintf(EnumsFile, "debug_enum_value* EnumValues = 0;\n\n");
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
        fprintf(StructsFile, "debug_struct_member StructMembers[STRUCT_MEMBERS_SIZE] = {\n", (int)StructMembers.size());

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

    return 0;
}