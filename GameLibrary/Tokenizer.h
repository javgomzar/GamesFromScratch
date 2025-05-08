#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "Win32PlatformLayer.h"

#include <stdlib.h>
#include <string.h>


enum token_type {
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
    Token_Constant,

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

void Advance(tokenizer* Tokenizer) {
    if (Tokenizer->At[0] == '\0') {
        Log(Warn, "Tokenizer reached EOF.");
        return;
    }
    else if (Tokenizer->At[0] == '\n') {
        Tokenizer->Line++;
        Tokenizer->Column = 1;
    }
    else {
        Tokenizer->Column++;
    }
    Tokenizer->At++;
}

void AdvanceUntil(tokenizer* Tokenizer, char C) {
    while (Tokenizer->At[0] != C && Tokenizer->At[0] != '\0') {
        Advance(Tokenizer);
    }
}

void AdvanceUntil(tokenizer* Tokenizer, char C1, char C2) {
    while ((Tokenizer->At[0] != C1 || Tokenizer->At[1] != C2) && Tokenizer->At[0] != '\0') {
        Advance(Tokenizer);
    }
}

token GetToken(tokenizer* Tokenizer) {
    token Token = {};
    Token.Length = 1;
    
    // Ommit whitespace and comments
    while (Tokenizer->At[0] != '\0') {
        if (IsWhitespace(Tokenizer->At[0])) {
            Advance(Tokenizer);
            continue;
        }
        else if (Tokenizer->At[0] == '/') {
            if (Tokenizer->At[1] == '/') {
                AdvanceUntil(Tokenizer, '\n');
                continue;
            }
            else if (Tokenizer->At[1] == '*') {
                AdvanceUntil(Tokenizer, '*', '/');
                Advance(Tokenizer);
                Advance(Tokenizer);
                continue;
            }
        }
        break;
    }

    char* TokenStart = Tokenizer->At;
    Token.Line = Tokenizer->Line;
    Token.Column = Tokenizer->Column;
    char C = Tokenizer->At[0];
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
            TokenStart = Tokenizer->At;
            AdvanceUntil(Tokenizer, '"');
            Token.Length = Tokenizer->At - Token.Text;
            Advance(Tokenizer);
        } break;

        default: {
            if (IsNumber(C)) {
                Token.Type = Token_Constant;
                while (IsNumber(Tokenizer->At[0])) {
                    Token.Length++;
                    Advance(Tokenizer);
                }
            }
            else if (IsAlphabet(C) || C == '_') {
                Token.Type = Token_Identifier;
                if (C == '_' && IsWhitespace(Tokenizer->At[0])) {
                    Token.Type = Token_Underscore;
                }
                else while (IsAlphanumeric(Tokenizer->At[0]) || Tokenizer->At[0] == '_') {
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

token RequireToken(tokenizer* Tokenizer, const char* Token) {
    token NextToken = GetToken(Tokenizer);
    if (NextToken == Token) {
        return NextToken;
    }
    else {
        char ErrorBuffer[256];
        sprintf_s(ErrorBuffer, "Token `%s` at line %d, column %d should be `%s`.", NextToken.Text, NextToken.Line, NextToken.Column, Token);
        Raise(ErrorBuffer);
    }
    return NextToken;
}

token RequireToken(tokenizer* Tokenizer, token_type Type) {
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
        Raise(ErrorBuffer);
    }
    return NextToken;
}

#endif