#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "GamePlatform.h"
#include "GameMath.h"

enum token_type {
    Token_Unknown,

    // Braces
    Token_OpenParen,
    Token_CloseParen,
    Token_OpenBracket,
    Token_CloseBracket,
    Token_OpenBrace,
    Token_CloseBrace,

    // Separators
    Token_Dot,
    Token_Comma,
    Token_Colon,
    Token_Semicolon,
    Token_Underscore,
    Token_Pound,

    // Single character operators
    Token_Equal,
    Token_LessThan,
    Token_GreaterThan,
    Token_Plus,
    Token_Minus,
    Token_Asterisk,
    Token_Percent,
    Token_Backslash,
    Token_Fwdslash,
    Token_Interrogation,
    Token_Exclamation,
    Token_Tilde,
    Token_At,
    Token_Bar,
    Token_And,
    Token_Caret,

    // Text
    Token_Identifier,
    Token_String,
    Token_Constant_Integer,
    Token_Constant_Decimal,
    Token_Constant_Hexadecimal,
    Token_Constant_Binary,

    Token_End
};

const int MAX_TOKEN_LENGTH = 128;

struct token {
    token_type Type;
    char Text[MAX_TOKEN_LENGTH];
    int Length;
    int Line;
    int Column;
};

bool IsWhitespace(char C) {
    return C == ' ' | C == '\n' | C == '\t' | C == '\r';
}

bool IsNumber(char C) {
    return C >= '0' && C <= '9';
}

bool IsAlphabet(char C) {
    return (C >= 'A' && C <= 'Z') || C >= 'a' && C <= 'z'; 
}

bool IsAlphanumeric(char C) {
    return IsNumber(C) || IsAlphabet(C);
}

bool operator==(token& T1, token& T2) {
    if (T1.Length != T2.Length) return false;
    for (int i = 0; i < T1.Length; i++) {
        if (T1.Text[i] != T2.Text[i]) return false;
    }
    return true;
};

bool operator==(token& T1, const char* Str) {
    int StrLen = strlen(Str);
    if (StrLen != T1.Length) return false;
    for (int i = 0; i < StrLen; i++) {
        if (T1.Text[i] != Str[i]) return false;
    }
    return true;
};

struct tokenizer {
    char* At;
    int Line;
    int Column;
};

tokenizer InitTokenizer(char* At) {
    return { At, 1, 1 };
}

tokenizer InitTokenizer(void* At) {
    return { (char*)At, 1, 1 };
}

void Advance(tokenizer& Tokenizer) {
    if (Tokenizer.At[0] == '\0') {
        Assert(false); // Tokenizer reached EOF
        return;
    }
    else if (Tokenizer.At[0] == '\n') {
        Tokenizer.Line++;
        Tokenizer.Column = 1;
    }
    else {
        Tokenizer.Column++;
    }
    Tokenizer.At++;
}

void AdvanceUntil(tokenizer& Tokenizer, char C) {
    while (Tokenizer.At[0] != C && Tokenizer.At[0] != '\0') {
        Advance(Tokenizer);
    }
}

void AdvanceUntil(tokenizer& Tokenizer, char C1, char C2) {
    while ((Tokenizer.At[0] != C1 || Tokenizer.At[1] != C2) && Tokenizer.At[0] != '\0') {
        Advance(Tokenizer);
    }
}

void AdvanceUntilLine(tokenizer& Tokenizer, int Line) {
    while(Tokenizer.Line < Line) {
        Advance(Tokenizer);
    }
}

token GetToken(tokenizer& Tokenizer) {
    token Token = {};
    Token.Length = 1;
    
    // Ommit whitespace and comments
    while (Tokenizer.At[0] != '\0') {
        if (IsWhitespace(Tokenizer.At[0])) {
            Advance(Tokenizer);
            continue;
        }
        else if (Tokenizer.At[0] == '/') {
            if (Tokenizer.At[1] == '/') {
                AdvanceUntil(Tokenizer, '\n');
                continue;
            }
            else if (Tokenizer.At[1] == '*') {
                AdvanceUntil(Tokenizer, '*', '/');
                Advance(Tokenizer);
                Advance(Tokenizer);
                continue;
            }
        }
        break;
    }

    char* TokenStart = Tokenizer.At;
    Token.Line = Tokenizer.Line;
    Token.Column = Tokenizer.Column;
    char C = Tokenizer.At[0];
    if (C != '\0') Advance(Tokenizer);

    switch(C) {
        case '(': { Token.Type = Token_OpenParen; } break;
        case ')': { Token.Type = Token_CloseParen; } break;
        case '[': { Token.Type = Token_OpenBracket; } break;
        case ']': { Token.Type = Token_CloseBracket; } break;
        case '{': { Token.Type = Token_OpenBrace; } break;
        case '}': { Token.Type = Token_CloseBrace; } break;
        case '.': { Token.Type = Token_Dot; } break;
        case ',': { Token.Type = Token_Comma; } break;
        case ':': { Token.Type = Token_Colon; } break;
        case ';': { Token.Type = Token_Semicolon; } break;
        case '#': { Token.Type = Token_Pound; } break;
        case '=': { Token.Type = Token_Equal; } break;
        case '<': { Token.Type = Token_LessThan; } break;
        case '>': { Token.Type = Token_GreaterThan; } break;
        case '+': { Token.Type = Token_Plus; } break;
        case '-': { Token.Type = Token_Minus; } break;
        case '*': { Token.Type = Token_Asterisk; } break;
        case '%': { Token.Type = Token_Percent; } break;
        case '\\': { Token.Type = Token_Backslash; } break;
        case '/': { Token.Type = Token_Fwdslash; } break;
        case '?': { Token.Type = Token_Interrogation; } break;
        case '!': { Token.Type = Token_Exclamation; } break;
        case '~': { Token.Type = Token_Tilde; } break;
        case '@': { Token.Type = Token_At; } break;
        case '|': { Token.Type = Token_Bar; } break;
        case '&': { Token.Type = Token_And; } break;
        case '^': { Token.Type = Token_Caret; } break;
        case '\0': { Token.Type = Token_End; } break;

        case '"': {
            Token.Type = Token_String;
            TokenStart = Tokenizer.At;
            AdvanceUntil(Tokenizer, '"');
            Token.Length = Tokenizer.At - TokenStart;
            Advance(Tokenizer);
        } break;

        case '0': {
            // Hexadecimal
            if (Tokenizer.At[0] == 'x') {
                Token.Type = Token_Constant_Hexadecimal;
                Token.Length++;
                Advance(Tokenizer);
                while(IsNumber(Tokenizer.At[0]) || Tokenizer.At[0] >= 'a' && Tokenizer.At[0] <= 'f') {
                    Token.Length++;
                    Advance(Tokenizer);
                }
            }

            // Binary
            else if (Tokenizer.At[0] == 'b') {
                Token.Type = Token_Constant_Binary;
                Token.Length++;
                Advance(Tokenizer);
                while(Tokenizer.At[0] == '0' || Tokenizer.At[0] == '1') {
                    Token.Length++;
                    Advance(Tokenizer);
                }
            }

            // Decimal
            else {
                while (IsNumber(Tokenizer.At[0])) {
                    Token.Length++;
                    Advance(Tokenizer);
                }
                if (Tokenizer.At[0] == '.') {
                    Token.Type = Token_Constant_Decimal;
                    Token.Length++;
                    Advance(Tokenizer);
                    while(IsNumber(Tokenizer.At[0])) {
                        Token.Length++;
                        Advance(Tokenizer);
                    }
                    if (Tokenizer.At[0] == 'f') {
                        Token.Length++;
                        Advance(Tokenizer);
                    }
                }
                else {
                    Token.Type = Token_Constant_Integer;
                }
            }
        } break;

        default: {
            if (IsNumber(C)) {
                while (IsNumber(Tokenizer.At[0])) {
                    Token.Length++;
                    Advance(Tokenizer);
                }
                if (Tokenizer.At[0] == '.') {
                    Token.Type = Token_Constant_Decimal;
                    Token.Length++;
                    Advance(Tokenizer);
                    while(IsNumber(Tokenizer.At[0])) {
                        Token.Length++;
                        Advance(Tokenizer);
                    }
                    if (Tokenizer.At[0] == 'f') {
                        Token.Length++;
                        Advance(Tokenizer);
                    }
                    else if (Tokenizer.At[0] == 'e') {
                        Token.Length++;
                        Advance(Tokenizer);
                        if (Tokenizer.At[0] == '-') {
                            Token.Length++;
                            Advance(Tokenizer);
                        }
                        while(IsNumber(Tokenizer.At[0])) {
                            Token.Length++;
                            Advance(Tokenizer);
                        }
                    }
                }
                else {
                    Token.Type = Token_Constant_Integer;
                }
            }
            else if (IsAlphabet(C) || C == '_') {
                Token.Type = Token_Identifier;
                if (C == '_' && IsWhitespace(Tokenizer.At[0])) {
                    Token.Type = Token_Underscore;
                }
                else while (IsAlphanumeric(Tokenizer.At[0]) || Tokenizer.At[0] == '_') {
                    Token.Length++;
                    Advance(Tokenizer);
                }
            }
            else {
                Advance(Tokenizer);
            }
        }
    }

    for (int i = 0; i < Token.Length; i++) {
        Token.Text[i] = TokenStart[i];
    }
    
    return Token;
}

token RequireToken(tokenizer& Tokenizer, const char* Token) {
    token NextToken = GetToken(Tokenizer);
    if (NextToken == Token) {
        return NextToken;
    }
    else {
        char ErrorBuffer[256];
        sprintf_s(ErrorBuffer, "Token `%s` at line %d, column %d should be `%s`.", NextToken.Text, NextToken.Line, NextToken.Column, Token);
        Assert(false);
    }
    return NextToken;
}

token RequireToken(tokenizer& Tokenizer, token_type Type) {
    token NextToken = GetToken(Tokenizer);
    if (NextToken.Type == Type) {
        return NextToken;
    }
    else {
        char ErrorBuffer[256];
        sprintf_s(
            ErrorBuffer, 
            "Token `%s` at line %d, column %d is type %d but should be %d.", 
            NextToken.Text, 
            NextToken.Line, 
            NextToken.Column, 
            NextToken.Type,
            Type
        );
        Assert(false);
    }
    return NextToken;
}

// Parsing
uint32 Parseuint32(tokenizer& Tokenizer) {
    token Token = RequireToken(Tokenizer, Token_Constant_Integer);
    char* End;
    return strtol(Token.Text, &End, 10);
}

int ParseInt(tokenizer& Tokenizer) {
    char* End = 0;
    token Token = GetToken(Tokenizer);
    bool Negative = false;
    if (Token.Type == Token_Minus) {
        Negative = true;
        Token = GetToken(Tokenizer);
    }
    Assert(Token.Type == Token_Constant_Integer, "Tried to parse int but didn't find a number.");
    int Result = strtol(Token.Text, &End, 10);
    return Negative ? -Result : Result;
}

float ParseFloat(tokenizer& Tokenizer) {
    char* End = 0;
    token Token = GetToken(Tokenizer);
    bool Negative = false;
    if (Token.Type == Token_Minus) {
        Negative = true;
        Token = GetToken(Tokenizer);
    }
    Assert(Token.Type == Token_Constant_Decimal || Token.Type == Token_Constant_Integer, "Tried to parse float but didn't find a number.");
    float Result = strtof(Token.Text, &End);
    return Negative ? -Result : Result;
}

double ParseDouble(tokenizer& Tokenizer) {
    char* End = 0;
    token Token = GetToken(Tokenizer);
    bool Negative = false;
    if (Token.Type == Token_Minus) {
        Negative = true;
        Token = GetToken(Tokenizer);
    }
    Assert(Token.Type == Token_Constant_Decimal || Token.Type == Token_Constant_Integer, "Tried to parse float but didn't find a number.");
    double Result = strtod(Token.Text, &End);
    return Negative ? -Result : Result;
}

v2 ParseV2(tokenizer& Tokenizer) {
    v2 Result;
    Result.X = ParseFloat(Tokenizer);
    Result.Y = ParseFloat(Tokenizer);
    return Result;
}

v3 ParseV3(tokenizer& Tokenizer) {
    v3 Result;
    Result.X = ParseFloat(Tokenizer);
    Result.Y = ParseFloat(Tokenizer);
    Result.Z = ParseFloat(Tokenizer);
    return Result;
}

v4 ParseV4(tokenizer& Tokenizer) {
    v4 Result;
    Result.X = ParseFloat(Tokenizer);
    Result.Y = ParseFloat(Tokenizer);
    Result.Z = ParseFloat(Tokenizer);
    Result.W = ParseFloat(Tokenizer);
    return Result;
}

quaternion ParseQuaternion(tokenizer& Tokenizer) {
    quaternion Result;
    Result.c = ParseFloat(Tokenizer);
    Result.i = ParseFloat(Tokenizer);
    Result.j = ParseFloat(Tokenizer);
    Result.k = ParseFloat(Tokenizer);
    return Result;
}

iv2 ParseIV2(tokenizer& Tokenizer) {
    iv2 Result;
    Result.X = ParseInt(Tokenizer);
    Result.Y = ParseInt(Tokenizer);
    return Result;
}

iv3 ParseIV3(tokenizer& Tokenizer) {
    iv3 Result;
    Result.X = ParseInt(Tokenizer);
    Result.Y = ParseInt(Tokenizer);
    Result.Z = ParseInt(Tokenizer);
    return Result;
}

uv3 ParseUV3(tokenizer& Tokenizer) {
    uv3 Result;
    Result.X = Parseuint32(Tokenizer);
    Result.Y = Parseuint32(Tokenizer);
    Result.Z = Parseuint32(Tokenizer);
    return Result;
}

#endif