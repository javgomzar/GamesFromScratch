#include <stdio.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include <cstring>

#include "GameBuild.h"
#include "Tokenizer.h"

#if _WIN32
#define PATH_SEPARATOR "\\"
#elif __linux__
#define PATH_SEPARATOR "/"
#endif

#define ArrayCount(arr) (sizeof((arr)) / sizeof((arr)[0]))

FILE* OpenFile(const char* path, const char* permissions) {
    FILE* file = fopen(path, permissions);
    if (!file) {
        perror("Failed to open file");
        abort();
    }
    fseek(file, 0, SEEK_SET);
    return file;
}

char* ReadFile(const char* path, size_t* file_size = nullptr) {
    FILE* file = OpenFile(path, "r");

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    if (file_size) {
        *file_size = size;
    }
    fseek(file, 0, SEEK_SET);

    char* result = (char*)malloc(size + 1);
    fread(result, size, 1, file);
    result[size] = 0;
    fclose(file);
    return result;
}

const char* const PRIMITIVE_TYPES[] = {
    "void",
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
    "memory_size",
    "float",
    "double",
    "v2",
    "v3",
    "v4",
    "scale",
    "quaternion",
    "color",
    "collider",
    "Arena"
};
const int PRIMITIVE_TYPE_COUNT = ArrayCount(PRIMITIVE_TYPES);

int main() {
    FILE* enums_file = OpenFile("GameBuild" PATH_SEPARATOR "GameEnums.h", "w");
    FILE* structs_file = OpenFile("GameBuild" PATH_SEPARATOR "GameStructs.h", "w");

    fprintf(enums_file,
    "#ifndef GAME_ENUMS\n"
    "#define GAME_ENUMS\n\n"
    );

    fprintf(enums_file, "enum Type {\n");
    for (int i = 0; i < PRIMITIVE_TYPE_COUNT; i++) {
        fprintf(enums_file, "    Type_%s,\n", PRIMITIVE_TYPES[i]);
    }

    fprintf(structs_file, 
    "#ifndef GAME_STRUCTS\n"
    "#define GAME_STRUCTS\n\n"
    );

    std::vector<std::string> enum_debug_types = {};
    std::vector<std::string> enum_values = {};
    std::vector<std::string> flag_debug_types = {};
    std::vector<std::string> flag_values = {};
    std::string flag_declarations = "";
    unsigned int flag_type_count = 0;
    std::vector<std::string> struct_debug_types = {};
    std::vector<std::string> struct_members = {};

    char buffer[256];
    Arena arena = AllocateMemoryArena(Kilobytes(8));
    Build_Configuration config = ReadBuildConfiguration(&arena, "GameBuild/build.conf");
    Tokenizer input_files_tokenizer = InitTokenizer(config.metaprogramming_input_files.content, config.metaprogramming_input_files.length);
    
    std::map<std::string, int> constants;
    for (int i = 0; i < config.metaprogramming_input_file_count; i++) {
        int path_length = ParsePath(input_files_tokenizer.at);
        strncpy(buffer, input_files_tokenizer.at, path_length);
        buffer[path_length] = '\0';
        Advance(input_files_tokenizer, path_length);
        if (i < config.metaprogramming_input_file_count-1) {
            RequireToken(input_files_tokenizer, Token_Semicolon);
            AdvanceWhitespaceAndComments(input_files_tokenizer);
        }

        char* file_content = ReadFile(buffer);

        Tokenizer tokenizer = InitTokenizer(file_content);
        Token token = GetToken(tokenizer);
        while (token.type != Token_End) {
            if (token == "const") {
                token = GetToken(tokenizer);
                if (token == "int") {
                    token = RequireToken(tokenizer, Token_Identifier);
                    std::string constant_name(token.text, token.length);
                    token = RequireToken(tokenizer, Token_Equal);
                    int Value = ParseInt(tokenizer);
                    constants[constant_name] = Value;
                }
                else if (token == "int32") {
                    token = RequireToken(tokenizer, Token_Identifier);
                    std::string constant_name(token.text, token.length);
                    token = RequireToken(tokenizer, Token_Equal);
                    token = RequireToken(tokenizer, Token_Identifier);
                    std::string first_addend(token.text, token.length);
                    int Value = constants[first_addend];
                    token = RequireToken(tokenizer, Token_Plus);
                    while (token.type != Token_Semicolon) {
                        if (token.type != Token_Plus) {
                            throw "Should be a plus sign.";
                        };
                        token = RequireToken(tokenizer, Token_Identifier);
                        std::string addend(token.text, token.length);
                        Value += constants[addend];
                        token = GetToken(tokenizer);
                    }
                    constants[constant_name] = Value;
                }
            }
            else if (token == "INTROSPECT") {
                token = GetToken(tokenizer);
                if (token == "struct") {
                    Token struct_type = RequireToken(tokenizer, Token_Identifier);
                    char struct_type_text[MAX_TOKEN_LENGTH] = {};
                    strncpy(struct_type_text, struct_type.text, struct_type.length);

                    sprintf(buffer, "Type_%s", struct_type_text);
                    struct_debug_types.push_back(std::string(buffer));

                    token = RequireToken(tokenizer, Token_OpenBrace);
                    token = RequireToken(tokenizer, Token_Identifier);
                    while (token.type != Token_CloseBrace && token.type != Token_End) {
                        Token member_type = token;
                        Token member_name = GetToken(tokenizer);
                        bool is_pointer = false;
                        if (member_name.type == Token_Asterisk) {
                            member_name = GetToken(tokenizer);
                            is_pointer = true;
                        }
                        token = GetToken(tokenizer);
                        int array_size = 0;
                        if (token.type == Token_OpenBracket) {
                            token = GetToken(tokenizer);
                            if (token.type == Token_Identifier) {
                                array_size = constants[std::string(token.text, token.length)];
                            }
                            else if (token.type == Token_Constant_Integer) {
                                char* end;
                                array_size = strtol(token.text, &end, 10);
                            }
                            token = RequireToken(tokenizer, Token_CloseBracket);
                            token = RequireToken(tokenizer, Token_Semicolon);
                        }
                        char member_type_debug_text[MAX_TOKEN_LENGTH] = {};
                        char member_type_text[MAX_TOKEN_LENGTH] = {};
                        strncpy(member_type_debug_text, member_type.text, member_type.length);
                        strncpy(member_type_text, member_type.text, member_type.length);

                        char member_name_text[MAX_TOKEN_LENGTH] = {};
                        strncpy(member_name_text, member_name.text, member_name.length);
                        std::string sizeof_arg = is_pointer ? "void*" : std::string(member_type_text);
                        sprintf(buffer, "    {\"%s\", Type_%s, Type_%s, sizeof(%s), (uint64)(&((%s*)0)->%s),%d, %s},\n", 
                            member_name_text, struct_type_text, member_type_debug_text, sizeof_arg.c_str(), struct_type_text, member_name_text,
                            array_size, is_pointer ? "true" : "false");
                        std::string struct_member = buffer;
                        struct_members.push_back(struct_member);
                        while (token.type != Token_Semicolon && token.type != Token_End) {
                            token = GetToken(tokenizer);
                        };
                        token = GetToken(tokenizer);
                    }
                }
            }
            else if (token == "ENUM") {
                RequireToken(tokenizer, Token_OpenParen);

                Token enum_name = RequireToken(tokenizer, Token_Identifier);
                char enum_name_text[MAX_TOKEN_LENGTH] = {};
                strncpy(enum_name_text, enum_name.text, enum_name.length);
                sprintf(buffer, "Type_%s", enum_name_text);
                enum_debug_types.push_back(std::string(buffer));

                token = RequireToken(tokenizer, Token_Comma);

                int value = 0;
                while(token.type != Token_CloseParen) {
                    token = RequireToken(tokenizer, Token_Identifier);
                    char token_text[MAX_TOKEN_LENGTH] = {};
                    strncpy(token_text, token.text, token.length);
                    sprintf(buffer, "    {Type_%s, \"%s\", %d},\n", enum_name_text, token_text, value++);
                    enum_values.push_back(std::string(buffer));
                    token = GetToken(tokenizer);
                    if (token.type != Token_CloseParen && token.type != Token_Comma) {
                        throw "Should be a comma.";
                    };
                }
                sprintf(buffer, "    {Type_%s, \"%s_count\", %d},\n", enum_name_text, enum_name_text, value);
                enum_values.push_back(std::string(buffer));
                sprintf(buffer, "%s_count", enum_name_text);
                constants[std::string(buffer)] = value;
            }
            else if (token == "FLAGS") {
                RequireToken(tokenizer, Token_OpenParen);

                Token flags_name = RequireToken(tokenizer, Token_Identifier);
                char flags_name_text[MAX_TOKEN_LENGTH] = {};
                strncpy(flags_name_text, flags_name.text, flags_name.length);
                flag_debug_types.push_back(std::string(flags_name_text));

                sprintf(buffer, "enum class %s : uint64 {\n    none = 0,\n", flags_name_text);
                flag_declarations += std::string(buffer);

                RequireToken(tokenizer, Token_Comma);

                int bit = 0;
                while(token.type != Token_CloseParen) {
                    Token flag_value = RequireToken(tokenizer, Token_Identifier);
                    char flag_value_text[MAX_TOKEN_LENGTH] = {};
                    strncpy(flag_value_text, flag_value.text, flag_value.length);
                    token = GetToken(tokenizer);
                    if (token.type == Token_Comma || token.type == Token_CloseParen) {
                        sprintf(buffer, "    {Type_%s, \"%s\", %d},\n", flags_name_text, flag_value_text, 1 << bit);
                        flag_values.push_back(std::string(buffer));
                        sprintf(buffer, "    %s = 1 << %d,\n", flag_value_text, bit);
                        flag_declarations += std::string(buffer);
                        bit++;
                    }
                    else if (token.type == Token_Equal) {
                        flag_declarations += std::string("    ") + std::string(flag_value_text) + std::string(" = ");
                        token = GetToken(tokenizer);
                        while (token.type != Token_Comma && token.type != Token_CloseParen) {
                            flag_declarations += std::string(token.text, token.length);
                            token = GetToken(tokenizer);
                        }
                        flag_declarations += std::string(",\n");
                    }
                }
                flag_declarations += std::string("};\n\n");
                flag_type_count += 1;
            }

            token = GetToken(tokenizer);
        }

        free(file_content);
    }

    for (const std::string& enum_name : enum_debug_types) {
        fprintf(enums_file, "    %s,\n", enum_name.c_str());
    }

    for (const std::string& flag_name : flag_debug_types) {
        fprintf(enums_file, "    Type_%s,\n", flag_name.c_str());
    }

    for (const std::string& struct_name : struct_debug_types) {
        fprintf(enums_file, "    %s,\n", struct_name.c_str());
    }

    fprintf(enums_file, "};\n\n");

    fprintf(enums_file, "%s", flag_declarations.c_str());

    int enum_count = enum_debug_types.size();
    fprintf(enums_file, "bool IsEnumType(Type type) { return type > %d && type < %d; }\n", 
        PRIMITIVE_TYPE_COUNT - 1, PRIMITIVE_TYPE_COUNT + enum_count
    );

    fprintf(enums_file, "bool IsFlagType(Type type) { return type > %d && type < %d; }\n\n",
        PRIMITIVE_TYPE_COUNT + enum_count - 1, PRIMITIVE_TYPE_COUNT + enum_count + flag_type_count
    );

    fprintf(enums_file,
    "struct Debug_Enum_Value {\n"
    "    Type enum_type;\n"
    "    const char* identifier;\n"
    "    int value;\n"
    "};\n\n");

    fprintf(enums_file, "const int ENUM_VALUES_SIZE = %d;\n", (int)enum_values.size());
    if (enum_values.size() > 0) {
        fprintf(enums_file, "Debug_Enum_Value ENUM_VALUES[ENUM_VALUES_SIZE] = {\n");

        for (const std::string& EnumValue : enum_values) {
            fprintf(enums_file, "%s", EnumValue.c_str());
        }

        fprintf(enums_file, "};\n\n");
    }
    else {
        fprintf(enums_file, "Debug_Enum_Value* ENUM_VALUES = 0;\n\n");
    }

    fprintf(enums_file, "const int FLAG_VALUES_SIZE = %d;\n", (int)flag_values.size());
    if (flag_values.size() > 0) {
        fprintf(enums_file, "Debug_Enum_Value FLAG_VALUES[FLAG_VALUES_SIZE] = {\n");

        for (const std::string& flag_value : flag_values) {
            fprintf(enums_file, "%s", flag_value.c_str());
        }

        fprintf(enums_file, "};\n\n");

        for (const std::string& flag_name : flag_debug_types) {
            fprintf(enums_file, "constexpr %s operator|(%s a, %s b) { return (%s)((uint64)a | (uint64)b); }\n", 
                flag_name.c_str(), flag_name.c_str(), flag_name.c_str(), flag_name.c_str());
            fprintf(enums_file, "constexpr %s& operator|=(%s& a, %s b) { return a = a | b; }\n", 
                flag_name.c_str(), flag_name.c_str(), flag_name.c_str());
            fprintf(enums_file, "constexpr bool operator&(%s a, %s b) { return (uint64)a & (uint64)b; }\n", 
                flag_name.c_str(), flag_name.c_str());
            fprintf(enums_file, "constexpr %s operator~(%s a) { return (%s)(~(uint64)a); }\n\n", 
                flag_name.c_str(), flag_name.c_str(), flag_name.c_str());
        }
    }
    else {
        fprintf(enums_file, "debug_enum_value* flag_values = nullptr;\n\n");
    }

    fprintf(enums_file, "#endif");

    int struct_count = struct_debug_types.size();
    fprintf(structs_file, "bool IsStructType(Type type) { return Type > %d && Type < %d; }\n\n", 
        PRIMITIVE_TYPE_COUNT + enum_count + flag_type_count - 1, PRIMITIVE_TYPE_COUNT + enum_count + flag_type_count + struct_count
    );

    fprintf(structs_file,
    "struct Debug_Struct_Member {\n"
    "    const char* name;\n"
    "    Type struct_type;\n"
    "    Type member_type;\n"
    "    uint64 size;\n"
    "    uint64 offset;\n"
    "    int array_size;\n"
    "    bool is_pointer;\n"
    "};\n\n");

    fprintf(structs_file, "const int STRUCT_MEMBERS_SIZE = %d;\n", (int)struct_members.size());
    if (struct_members.size() > 0) {
        fprintf(structs_file, "Debug_Struct_Member struct_members[STRUCT_MEMBERS_SIZE] = {\n");

        for (const std::string& StructMember : struct_members) {
            fprintf(structs_file, "%s", StructMember.c_str());
        }

        fprintf(structs_file, "};\n");
    }
    else {
        fprintf(structs_file, "Debug_Struct_Member* struct_members = 0;\n\n");
    }

    fprintf(structs_file, "\n#endif");

    fclose(structs_file);
    fclose(enums_file);

    // Tests

    size_t tests_file_size;
    char* tests_content = ReadFile("GameTest" PATH_SEPARATOR "Tests.h", &tests_file_size);
    FILE* test_output_file = OpenFile("GameTest" PATH_SEPARATOR "GameTest.h", "w");

    fprintf(test_output_file, 
    "#include \"Tests.h\"\n\n");

    std::string memory_setup = "    Render_Group* group = &memory->render_group;\n"
    "    game_asset_manager* AssetManager = &Memory->AssetManager;\n"
    "    game_state* State = Memory->GameState;\n"
    "    Arena* Permanent = &Memory->Permanent;\n"
    "    Arena* Transient = &Memory->Transient;\n"
    "    game_input* Input = &Memory->Input;\n"
    "    Light* light = &Memory->RenderGroup.Light;\n"
    "    float Time = State->Time;\n"
    "    bool Result = false;\n";

    std::string once_tests = std::string("void RunOnceTests(game_memory* Memory) {\n") + memory_setup;
    std::string reload_tests = std::string("void Runreload_tests(game_memory* Memory) {\n" + memory_setup);
    std::string every_frame_tests = std::string("void Runevery_frame_tests(game_memory* Memory) {\n" + memory_setup);

    Tokenizer tokenizer = InitTokenizer(tests_content, tests_file_size);
    Token token = GetToken(tokenizer);
    while (token.type != Token_End) {
        if (token.type == Token_Colon) {
            token = GetToken(tokenizer);
            if (token.type == Token_Colon) {
                token = GetToken(tokenizer);
            }
        }
        else if (token == "TEST") {
            RequireToken(tokenizer, Token_OpenParen);
            Token test_name_token = RequireToken(tokenizer, Token_Identifier);

            RequireToken(tokenizer, Token_Comma);
            Token test_type = RequireToken(tokenizer, Token_Identifier);
            
            RequireToken(tokenizer, Token_Comma);
            Token test_active_token = RequireToken(tokenizer, Token_Identifier);
            bool test_active = test_active_token == "ACTIVE";
            RequireToken(tokenizer, Token_CloseParen);

            if (test_active) {
                std::string test_name = std::string(test_name_token.text, test_name_token.length);
                if (test_type == "test_once") {
                    once_tests += std::string("\n"
                    "    try { Result = "
                    ) + test_name + std::string("(");
                    Token previous_token = RequireToken(tokenizer, Token_OpenParen);
                    token = GetToken(tokenizer);
                    while (token.type != Token_CloseParen) {
                        previous_token = token;
                        token = GetToken(tokenizer);
                        if (token.type == Token_Comma) {
                            once_tests += std::string(previous_token.text, previous_token.length) + std::string(", ");
                        }
                        else if (token.type == Token_CloseParen) {
                            once_tests += std::string(previous_token.text, previous_token.length);
                        }
                    }
                    once_tests += std::string(
                    "); }\n"
                    "    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }\n"
                    "    catch(...) { Log(log_level::Error, \"Unknown test error.\"); }\n"
                    );

                    once_tests += std::string("    if(Result) { Log(log_level::Test, \"Test '") + test_name + std::string("' was correctly executed.\"); }\n");
                    once_tests += std::string("    else       { Log(log_level::Error, \"Test '") + test_name + std::string("' failed.\"); }\n");
                }
                else if (test_type == "test_reload") {
                    reload_tests += std::string("\n"
                    "    try { Result = "
                    ) + test_name + std::string("(");
                    Token previous_token = RequireToken(tokenizer, Token_OpenParen);
                    token = GetToken(tokenizer);
                    while (token.type != Token_CloseParen) {
                        previous_token = token;
                        token = GetToken(tokenizer);
                        if (token.type == Token_Comma) {
                            reload_tests += std::string(previous_token.text, previous_token.length) + std::string(", ");
                        }
                        else if (token.type == Token_CloseParen) {
                            reload_tests += std::string(previous_token.text, previous_token.length);
                        }
                    }
                    reload_tests += std::string(
                    "); }\n"
                    "    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }\n"
                    "    catch(...) { Log(log_level::Error, \"Unknown test error.\"); }\n"
                    );
                    reload_tests += std::string("    if(Result) { Log(log_level::Test, \"Test '") + test_name + std::string("' was correctly executed.\"); }\n");
                    reload_tests += std::string("    else       { Log(log_level::Error, \"Test '") + test_name + std::string("' failed.\"); }\n");
                }
                else if (test_type == "test_every_frame") {
                    every_frame_tests += std::string("\n"
                    "    try { Result = "
                    ) + test_name + std::string("(");
                    Token previous_token = RequireToken(tokenizer, Token_OpenParen);
                    token = GetToken(tokenizer);
                    while (token.type != Token_CloseParen) {
                        previous_token = token;
                        token = GetToken(tokenizer);
                        if (token.type == Token_Comma) {
                            every_frame_tests += std::string(previous_token.text, previous_token.length) + std::string(", ");
                        }
                        else if (token.type == Token_CloseParen) {
                            every_frame_tests += std::string(previous_token.text, previous_token.length);
                        }
                    }
                    every_frame_tests += std::string(
                    "); }\n"
                    "    catch(const char* ErrorMessage) { Log(log_level::Error, ErrorMessage); }\n"
                    "    catch(...) { Log(log_level::Error, \"Unknown test error.\"); }\n"
                    );
                    every_frame_tests += std::string("    if (!Result) { Log(log_level::Error, \"Test '") + test_name + std::string("' failed.\"); }\n");
                }
            }
        }

        token = GetToken(tokenizer);
    }
    once_tests += std::string("}");
    reload_tests += std::string("}");
    every_frame_tests += std::string("}");
    fprintf(test_output_file,"%s\n\n%s\n\n%s", once_tests.c_str(), reload_tests.c_str(), every_frame_tests.c_str());

    fclose(test_output_file);

    return 0;
}