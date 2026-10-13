#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <cstring>

enum Token_Type {
    Token_Unknown,

    // `'\n'`
    Token_LineJump,
    // `' '`
    Token_Space,
    // `'('`
    Token_OpenParen,
    // `')'`
    Token_CloseParen,
    // `'['`
    Token_OpenBracket,
    // `']'`
    Token_CloseBracket,
    // `'{'`
    Token_OpenBrace,
    // `'}'`
    Token_CloseBrace,
    // `'.'`
    Token_Dot,
    // `','`
    Token_Comma,
    // `':'`
    Token_Colon,
    // `';'`
    Token_Semicolon,
    // `'_'`
    Token_Underscore,
    // `'#'`
    Token_Pound,
    // `'='`
    Token_Equal,
    // `'<'`
    Token_LessThan,
    // `'>'`
    Token_GreaterThan,
    // `'+'`
    Token_Plus,
    // `'-'`
    Token_Minus,
    // `'*'`
    Token_Asterisk,
    // `'%'`
    Token_Percent,
    // `'\'`
    Token_Backslash,
    // `'/'`
    Token_Forwardslash,
    // `'?'`
    Token_Interrogation,
    // `'!'`
    Token_Exclamation,
    // `
    Token_Tilde,
    // `'@'`
    Token_At,
    // `'|'`
    Token_Bar,
    // `'&'`
    Token_And,
    // `'^'`
    Token_Caret,
    // `'`
    Token_Apostrophe,

    Token_Identifier,
    Token_String,
    Token_Constant_Integer,
    Token_Constant_Decimal,
    Token_Constant_Hexadecimal,
    Token_Constant_Binary,

    // `'\0'`
    Token_End
};

const char* TOKEN_TYPE_NAME[] = {
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
    "forward slash",
    "interrogation",
    "exclamation",
    "tilde",
    "at",
    "bar",
    "and",
    "caret",
    "apostrophe",

    "identifier",
    "string",
    "constant integer",
    "constant decimal",
    "constant hexadecimal",
    "constant binary",

    "EOF"
};

const int MAX_TOKEN_LENGTH = 128;

struct Token {
    Token_Type type;
    const char* text;
    int length;
    int line;
    int column;
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

bool operator==(Token& T1, Token& T2) {
    if (T1.length != T2.length) return false;
    for (int i = 0; i < T1.length; i++) {
        if (T1.text[i] != T2.text[i]) return false;
    }
    return true;
};

bool operator==(Token& T1, const char* Str) {
    int StrLen = strlen(Str);
    if (StrLen != T1.length) return false;
    for (int i = 0; i < StrLen; i++) {
        if (T1.text[i] != Str[i]) return false;
    }
    return true;
};

struct Tokenizer {
    const char* at;
    const char* start;
    const char* end;
    int line;
    int column;
    int length;
    bool ignore_whitespace;
};

Tokenizer InitTokenizer(const char* at, int length = 0, bool ignore_whitespace = true) {
    Tokenizer result = {};
    result.start = at;
    result.at = at;
    result.line = 1;
    result.column = 1;
    result.length = length;
    result.end = length > 0 ? result.start + length : nullptr;
    result.ignore_whitespace = ignore_whitespace;

    return result;
}

Tokenizer InitTokenizer(void* at, int length = 0, bool ignore_whitespace = true) {
    Tokenizer result = {};
    result.start = (char*)at;
    result.at = result.start;
    result.line = 1;
    result.column = 1;
    result.length = length;
    result.end = length > 0 ? result.start + length : nullptr;
    result.ignore_whitespace = ignore_whitespace;

    return result;
}

void Advance(Tokenizer& Tokenizer) {
    if (Tokenizer.at[0] == '\0') {
        throw "Tokenizer reached EOF";
        return;
    }
    else if (Tokenizer.at[0] == '\n') {
        Tokenizer.line++;
        Tokenizer.column = 1;
    }
    else {
        Tokenizer.column++;
    }
    Tokenizer.at++;
}

void Advance(Tokenizer& Tokenizer, int nChars) {
    for (int i = 0; i < nChars; i++) {
        Advance(Tokenizer);
    }
}

void AdvanceUntil(Tokenizer& Tokenizer, char C) {
    while (Tokenizer.at[0] != C && Tokenizer.at[0] != '\0') {
        Advance(Tokenizer);
    }
}

void AdvanceUntil(Tokenizer& Tokenizer, char C1, char C2) {
    while ((Tokenizer.at[0] != C1 || Tokenizer.at[1] != C2) && Tokenizer.at[0] != '\0') {
        Advance(Tokenizer);
    }
}

void AdvanceUntilLine(Tokenizer& Tokenizer, int Line) {
    while(Tokenizer.line < Line) {
        Advance(Tokenizer);
    }
}

void AdvanceUntilNextLine(Tokenizer& Tokenizer) {
    AdvanceUntilLine(Tokenizer, Tokenizer.line + 1);
}

void AdvanceWhitespaceAndComments(Tokenizer& Tokenizer) {
    while (Tokenizer.at != Tokenizer.end && Tokenizer.at[0] != '\0') {
        if (Tokenizer.ignore_whitespace && IsWhitespace(Tokenizer.at[0])) {
            Advance(Tokenizer);
            continue;
        }
        else if (Tokenizer.at[0] == '/') {
            if (Tokenizer.at[1] == '/') {
                AdvanceUntil(Tokenizer, '\n');
                continue;
            }
            else if (Tokenizer.at[1] == '*') {
                AdvanceUntil(Tokenizer, '*', '/');
                Advance(Tokenizer);
                Advance(Tokenizer);
                continue;
            }
        }
        break;
    }
}

Token GetToken(Tokenizer& Tokenizer) {
    Token Token = {};
    Token.length = 1;
    
    // Ommit whitespace and comments
    AdvanceWhitespaceAndComments(Tokenizer);

    Token.line = Tokenizer.line;
    Token.column = Tokenizer.column;

    if (Tokenizer.length > 0 && Tokenizer.at == Tokenizer.end) {
        Token.type = Token_End;
        Token.length = 0;
        return Token;
    }

    const char* TokenStart = Tokenizer.at;
    char C = Tokenizer.at[0];
    if (C != '\0') Advance(Tokenizer);

    switch(C) {
        case ' ':  { Token.type = Token_Space; } break;
        case '\n': { Token.type = Token_LineJump; } break;
        case '(':  { Token.type = Token_OpenParen; } break;
        case ')':  { Token.type = Token_CloseParen; } break;
        case '[':  { Token.type = Token_OpenBracket; } break;
        case ']':  { Token.type = Token_CloseBracket; } break;
        case '{':  { Token.type = Token_OpenBrace; } break;
        case '}':  { Token.type = Token_CloseBrace; } break;
        case '.':  { Token.type = Token_Dot; } break;
        case ',':  { Token.type = Token_Comma; } break;
        case ':':  { Token.type = Token_Colon; } break;
        case ';':  { Token.type = Token_Semicolon; } break;
        case '#':  { Token.type = Token_Pound; } break;
        case '=':  { Token.type = Token_Equal; } break;
        case '<':  { Token.type = Token_LessThan; } break;
        case '>':  { Token.type = Token_GreaterThan; } break;
        case '+':  { Token.type = Token_Plus; } break;
        case '-':  { Token.type = Token_Minus; } break;
        case '*':  { Token.type = Token_Asterisk; } break;
        case '%':  { Token.type = Token_Percent; } break;
        case '\\': { Token.type = Token_Backslash; } break;
        case '/':  { Token.type = Token_Forwardslash; } break;
        case '?':  { Token.type = Token_Interrogation; } break;
        case '!':  { Token.type = Token_Exclamation; } break;
        case '~':  { Token.type = Token_Tilde; } break;
        case '@':  { Token.type = Token_At; } break;
        case '|':  { Token.type = Token_Bar; } break;
        case '&':  { Token.type = Token_And; } break;
        case '^':  { Token.type = Token_Caret; } break;
        case '\'': { Token.type = Token_Apostrophe; } break;
        case '\0': { Token.type = Token_End; } break;

        case '"': {
            Token.type = Token_String;
            TokenStart = Tokenizer.at;
            do {
                AdvanceUntil(Tokenizer, '"');
                if (Tokenizer.at[-1] == '\\') {
                    Advance(Tokenizer);
                    continue;
                }
                else {
                    break;
                }
            } while(true);
            Token.length = Tokenizer.at - TokenStart;
            Advance(Tokenizer);
        } break;

        case '0': {
            // Hexadecimal
            if (Tokenizer.at[0] == 'x') {
                Token.type = Token_Constant_Hexadecimal;
                Token.length++;
                Advance(Tokenizer);
                while(IsNumber(Tokenizer.at[0]) || Tokenizer.at[0] >= 'a' && Tokenizer.at[0] <= 'f') {
                    Token.length++;
                    Advance(Tokenizer);
                }
            }

            // Binary
            else if (Tokenizer.at[0] == 'b') {
                Token.type = Token_Constant_Binary;
                Token.length++;
                Advance(Tokenizer);
                while(Tokenizer.at[0] == '0' || Tokenizer.at[0] == '1') {
                    Token.length++;
                    Advance(Tokenizer);
                }
            }

            // Decimal
            else {
                while (IsNumber(Tokenizer.at[0])) {
                    Token.length++;
                    Advance(Tokenizer);
                }
                if (Tokenizer.at[0] == '.') {
                    Token.type = Token_Constant_Decimal;
                    Token.length++;
                    Advance(Tokenizer);
                    while(IsNumber(Tokenizer.at[0])) {
                        Token.length++;
                        Advance(Tokenizer);
                    }
                    if (Tokenizer.at[0] == 'f') {
                        Token.length++;
                        Advance(Tokenizer);
                    }
                }
                else {
                    Token.type = Token_Constant_Integer;
                }
            }
        } break;

        default: {
            if (IsNumber(C)) {
                while (IsNumber(Tokenizer.at[0])) {
                    Token.length++;
                    Advance(Tokenizer);
                }
                if (Tokenizer.at[0] == '.') {
                    Token.type = Token_Constant_Decimal;
                    Token.length++;
                    Advance(Tokenizer);
                    while(IsNumber(Tokenizer.at[0])) {
                        Token.length++;
                        Advance(Tokenizer);
                    }
                    if (Tokenizer.at[0] == 'f') {
                        Token.length++;
                        Advance(Tokenizer);
                    }
                    else if (Tokenizer.at[0] == 'e') {
                        Token.length++;
                        Advance(Tokenizer);
                        if (Tokenizer.at[0] == '-') {
                            Token.length++;
                            Advance(Tokenizer);
                        }
                        while(IsNumber(Tokenizer.at[0])) {
                            Token.length++;
                            Advance(Tokenizer);
                        }
                    }
                }
                else {
                    Token.type = Token_Constant_Integer;
                }
            }
            else if (IsAlphabet(C) || C == '_') {
                Token.type = Token_Identifier;
                if (C == '_' && IsWhitespace(Tokenizer.at[0])) {
                    Token.type = Token_Underscore;
                }
                else while (IsAlphanumeric(Tokenizer.at[0]) || Tokenizer.at[0] == '_') {
                    Token.length++;
                    Advance(Tokenizer);
                }
            }
            else {
                Advance(Tokenizer);
            }
        }
    }

    Token.text = TokenStart;
    
    return Token;
}

Token RequireToken(Tokenizer& tokenizer, const char* text) {
    Token token = GetToken(tokenizer);
    if (token == text) {
        return token;
    }
    else {
        char error_buffer[256];
        char token_text[MAX_TOKEN_LENGTH] = "";
        strncpy(token_text, token.text, token.length);
        sprintf(
            error_buffer, 
            "Token `%s` at line %d, column %d should be `%s`.", 
            token_text, token.line, token.column, text
        );
        throw error_buffer;
    }
    return token;
}

Token RequireToken(Tokenizer& tokenizer, Token_Type type) {
    Token token = GetToken(tokenizer);
    if (token.type == type) {
        return token;
    }
    else {
        char error_buffer[256];
        char token_text[MAX_TOKEN_LENGTH] = {};
        strncpy(token_text, token.text, token.length);
        sprintf(
            error_buffer, 
            "Token `%s` at line %d, column %d is type '%s' but should be '%s'.", 
            token_text, token.line, token.column, TOKEN_TYPE_NAME[token.type], TOKEN_TYPE_NAME[type]
        );
        throw error_buffer;
    }
    return token;
}

// Parsing
bool ParseBool(Tokenizer& tokenizer) {
    Token token = RequireToken(tokenizer, Token_Identifier);
    return token == "true" || token == "True" || token == "TRUE" || token == "1";
}

unsigned int Parseuint32(Tokenizer& tokenizer) {
    Token token = RequireToken(tokenizer, Token_Constant_Integer);
    char* end;
    return strtol(token.text, &end, 10);
}

int ParseInt(Tokenizer& tokenizer) {
    char* end = 0;
    Token token = GetToken(tokenizer);
    bool negative = false;
    if (token.type == Token_Minus) {
        negative = true;
        token = GetToken(tokenizer);
    }
    if (token.type != Token_Constant_Integer) {
        throw "Tried to parse int but didn't find a number.";
    };
    int result = strtol(token.text, &end, 10);
    return negative ? -result : result;
}

float ParseFloat(Tokenizer& tokenizer) {
    char* end = 0;
    Token token = GetToken(tokenizer);
    bool negative = false;
    if (token.type == Token_Minus) {
        negative = true;
        token = GetToken(tokenizer);
    }
    if (token.type != Token_Constant_Decimal && token.type != Token_Constant_Integer) {
        throw "Tried to parse float but didn't find a number.";
    }
    float result = strtof(token.text, &end);
    return negative ? -result : result;
}

double ParseDouble(Tokenizer& tokenizer) {
    char* end = 0;
    Token token = GetToken(tokenizer);
    bool negative = false;
    if (token.type == Token_Minus) {
        negative = true;
        token = GetToken(tokenizer);
    }
    if (token.type == Token_Constant_Decimal || token.type == Token_Constant_Integer) {
        throw "Tried to parse float but didn't find a number.";
    };
    double result = strtod(token.text, &end);
    return negative ? -result : result;
}

// If input pointer is a string that represents a path, returns the string length of the path. If not, returns 0.
int ParsePath(const char* text) {
    int result = 0;
    while (text[0] != '\0' && text[0] != ';' && text[0] != '\n' && text[0] != '\r') {
#ifdef _WIN32
        if (
            text[0] == '<' || text[0] == '>' || text[0] == '|' || text[0] == '?' || text[0] == '*' ||
            text[0] >= 0 && text[0] < 32
        ) {
            throw "Invalid character in path.";
        }
#endif
        result++;
        text++;
    }
    return result;
}

#endif