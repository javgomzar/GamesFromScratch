#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>

enum token_type {
    Token_Unknown,

    // Whitespace
    Token_LineJump,
    Token_Space,

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

const char* TokenTypeName[] = {
    "unknown",

    "line jump",
    "space",

    "open parenthesis",
    "close parenthesis",
    "open bracket",
    "close bracket",
    "open brace",
    "close brace",

    "dot",
    "comma",
    "colon",
    "semicolon",
    "underscore",
    "pound",

    "equal",
    "lessThan",
    "greaterThan",
    "plus",
    "minus",
    "asterisk",
    "percent",
    "back slash",
    "forwardd slash",
    "interrogation",
    "exclamation",
    "tilde",
    "at",
    "bar",
    "and",
    "caret",

    "identifier",
    "string",
    "constant integer",
    "constant decimal",
    "constant hexadecimal",
    "constant binary",

    "EOF"
};

const int MAX_TOKEN_LENGTH = 128;

struct token {
    token_type Type;
    char* Text;
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

bool IsFileSeparator(char C) {
    return C == '/' || C == '\\';
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
    bool IgnoreWhitespace;
};

tokenizer InitTokenizer(char* At, bool IgnoreWhitespace = true) {
    return { At, 1, 1, IgnoreWhitespace };
}

tokenizer InitTokenizer(void* At, bool IgnoreWhitespace = true) {
    return { (char*)At, 1, 1, IgnoreWhitespace };
}

void Advance(tokenizer& Tokenizer) {
    if (Tokenizer.At[0] == '\0') {
        throw "Tokenizer reached EOF";
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

void AdvanceUntilNextLine(tokenizer& Tokenizer) {
    AdvanceUntilLine(Tokenizer, Tokenizer.Line + 1);
}

token GetToken(tokenizer& Tokenizer) {
    token Token = {};
    Token.Length = 1;
    
    // Ommit whitespace and comments
    while (Tokenizer.At[0] != '\0') {
        if (Tokenizer.IgnoreWhitespace && IsWhitespace(Tokenizer.At[0])) {
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
        case ' ':  { Token.Type = Token_Space; } break;
        case '\n': { Token.Type = Token_LineJump; } break;
        case '(':  { Token.Type = Token_OpenParen; } break;
        case ')':  { Token.Type = Token_CloseParen; } break;
        case '[':  { Token.Type = Token_OpenBracket; } break;
        case ']':  { Token.Type = Token_CloseBracket; } break;
        case '{':  { Token.Type = Token_OpenBrace; } break;
        case '}':  { Token.Type = Token_CloseBrace; } break;
        case '.':  { Token.Type = Token_Dot; } break;
        case ',':  { Token.Type = Token_Comma; } break;
        case ':':  { Token.Type = Token_Colon; } break;
        case ';':  { Token.Type = Token_Semicolon; } break;
        case '#':  { Token.Type = Token_Pound; } break;
        case '=':  { Token.Type = Token_Equal; } break;
        case '<':  { Token.Type = Token_LessThan; } break;
        case '>':  { Token.Type = Token_GreaterThan; } break;
        case '+':  { Token.Type = Token_Plus; } break;
        case '-':  { Token.Type = Token_Minus; } break;
        case '*':  { Token.Type = Token_Asterisk; } break;
        case '%':  { Token.Type = Token_Percent; } break;
        case '\\': { Token.Type = Token_Backslash; } break;
        case '/':  { Token.Type = Token_Fwdslash; } break;
        case '?':  { Token.Type = Token_Interrogation; } break;
        case '!':  { Token.Type = Token_Exclamation; } break;
        case '~':  { Token.Type = Token_Tilde; } break;
        case '@':  { Token.Type = Token_At; } break;
        case '|':  { Token.Type = Token_Bar; } break;
        case '&':  { Token.Type = Token_And; } break;
        case '^':  { Token.Type = Token_Caret; } break;
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

    Token.Text = TokenStart;
    
    return Token;
}

token RequireToken(tokenizer& Tokenizer, const char* Text) {
    token Token = GetToken(Tokenizer);
    if (Token == Text) {
        return Token;
    }
    else {
        char ErrorBuffer[256];
        char TokenText[MAX_TOKEN_LENGTH] = "";
        strcpy_s(TokenText, Token.Length * sizeof(char), Token.Text);
        sprintf_s(
            ErrorBuffer, 
            "Token `%s` at line %d, column %d should be `%s`.", 
            TokenText, Token.Line, Token.Column, Text
        );
        throw ErrorBuffer;
    }
    return Token;
}

token RequireToken(tokenizer& Tokenizer, token_type Type) {
    token Token = GetToken(Tokenizer);
    if (Token.Type == Type) {
        return Token;
    }
    else {
        char ErrorBuffer[256];
        char TokenText[MAX_TOKEN_LENGTH] = {};
        strcpy_s(TokenText, Token.Length * sizeof(char), Token.Text);
        sprintf_s(
            ErrorBuffer, 
            "Token `%s` at line %d, column %d is type '%s' but should be '%s'.", 
            TokenText, Token.Line, Token.Column, TokenTypeName[Token.Type], TokenTypeName[Type]
        );
        throw ErrorBuffer;
    }
    return Token;
}

// Parsing
unsigned int Parseuint32(tokenizer& Tokenizer) {
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
    if (Token.Type != Token_Constant_Integer) {
        throw "Tried to parse int but didn't find a number.";
    };
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
    if (Token.Type != Token_Constant_Decimal && Token.Type != Token_Constant_Integer) {
        throw "Tried to parse float but didn't find a number.";
    }
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
    if (Token.Type == Token_Constant_Decimal || Token.Type == Token_Constant_Integer) {
        throw "Tried to parse float but didn't find a number.";
    };
    double Result = strtod(Token.Text, &End);
    return Negative ? -Result : Result;
}

// If input pointer is a string that represents a path, returns the string length of the path. If not, returns 0.
int ParsePath(char* Text) {
    int Result = 0;
    while (Text[0] != '\0') {
        if (Text[0] == ';' || Text[0] == '\n' || Text[0] == '\r') break;

#ifdef _WIN32
        if (
            Text[0] == '<' || Text[0] == '>' || Text[0] == '|' || Text[0] == '?' || Text[0] == '*' ||
            Text[0] >= 0 && Text[0] < 32
        ) {
            throw "Invalid character in path.";
        }
#endif
        Result++;
        Text++;
    }
    return Result;
}

#endif