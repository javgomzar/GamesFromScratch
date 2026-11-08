#include "tokenizer.h"


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

bool Token::operator==(Token& rhs) {
    if (length != rhs.length) return false;
    for (int i = 0; i < length; i++) {
        if (text[i] != rhs.text[i]) return false;
    }
    return true;
};

bool Token::operator==(const char* rhs) {
    int len = strlen(rhs);
    if (len != length) return false;
    for (int i = 0; i < len; i++) {
        if (text[i] != rhs[i]) return false;
    }
    return true;
};

Tokenizer::Tokenizer(const char* text, int max_length, bool should_ignore_whitespace) {
    start = text;
    at = text;
    line = 1;
    column = 1;
    length = max_length;
    end = length > 0 ? start + length : nullptr;
    ignore_whitespace = should_ignore_whitespace;
}

Tokenizer::Tokenizer(void* memory, int max_length, bool should_ignore_whitespace) {
    start = (char*)memory;
    at = start;
    line = 1;
    column = 1;
    length = max_length;
    end = length > 0 ? start + length : nullptr;
    ignore_whitespace = should_ignore_whitespace;
}

bool Tokenizer::Advance() {
    if (at[0] == '\0') {
        return false;
    }
    else if (at[0] == '\n') {
        ++line;
        column = 1;
    }
    else {
        ++column;
    }
    ++at;
    return true;
}

void Tokenizer::Advance(int char_count) {
    for (int i = 0; i < char_count; i++) {
        Advance();
    }
}

void Tokenizer::AdvanceUntil(char c) {
    while (at[0] != c && at[0] != '\0') {
        Advance();
    }
}

void Tokenizer::AdvanceUntil(char c1, char c2) {
    while ((at[0] != c1 || at[1] != c2) && at[0] != '\0') {
        Advance();
    }
}

void Tokenizer::AdvanceUntilLine(int line_number) {
    while(line < line_number) {
        Advance();
    }
}

void Tokenizer::AdvanceUntilNextLine() {
    AdvanceUntilLine(line + 1);
}

void Tokenizer::AdvanceWhitespaceAndComments() {
    while (at != end && at[0] != '\0') {
        if (ignore_whitespace && IsWhitespace(at[0])) {
            Advance();
            continue;
        }
        else if (at[0] == '/') {
            if (at[1] == '/') {
                AdvanceUntil('\n');
                continue;
            }
            else if (at[1] == '*') {
                AdvanceUntil('*', '/');
                Advance();
                Advance();
                continue;
            }
        }
        break;
    }
}

Token Tokenizer::GetToken() {
    Token token = {};
    token.length = 1;
    
    // Ommit whitespace and comments
    AdvanceWhitespaceAndComments();

    token.line = line;
    token.column = column;

    if (length > 0 && at == end) {
        token.type = Token_End;
        token.length = 0;
        return token;
    }

    const char* token_start = at;
    char c = at[0];
    if (c != '\0') Advance();

    switch(c) {
        case ' ':  { token.type = Token_Space; } break;
        case '\n': { token.type = Token_LineJump; } break;
        case '(':  { token.type = Token_OpenParen; } break;
        case ')':  { token.type = Token_CloseParen; } break;
        case '[':  { token.type = Token_OpenBracket; } break;
        case ']':  { token.type = Token_CloseBracket; } break;
        case '{':  { token.type = Token_OpenBrace; } break;
        case '}':  { token.type = Token_CloseBrace; } break;
        case '.':  { token.type = Token_Dot; } break;
        case ',':  { token.type = Token_Comma; } break;
        case ':':  { token.type = Token_Colon; } break;
        case ';':  { token.type = Token_Semicolon; } break;
        case '#':  { token.type = Token_Pound; } break;
        case '=':  { token.type = Token_Equal; } break;
        case '<':  { token.type = Token_LessThan; } break;
        case '>':  { token.type = Token_GreaterThan; } break;
        case '+':  { token.type = Token_Plus; } break;
        case '-':  { token.type = Token_Minus; } break;
        case '*':  { token.type = Token_Asterisk; } break;
        case '%':  { token.type = Token_Percent; } break;
        case '\\': { token.type = Token_Backslash; } break;
        case '/':  { token.type = Token_Forwardslash; } break;
        case '?':  { token.type = Token_Interrogation; } break;
        case '!':  { token.type = Token_Exclamation; } break;
        case '~':  { token.type = Token_Tilde; } break;
        case '@':  { token.type = Token_At; } break;
        case '|':  { token.type = Token_Bar; } break;
        case '&':  { token.type = Token_And; } break;
        case '^':  { token.type = Token_Caret; } break;
        case '\'': { token.type = Token_Apostrophe; } break;
        case '\0': { token.type = Token_End; } break;

        case '"': {
            token.type = Token_String;
            token_start = at;
            do {
                AdvanceUntil('"');
                if (at[-1] == '\\') {
                    Advance();
                    continue;
                }
                else {
                    break;
                }
            } while(true);
            length = at - token_start;
            Advance();
        } break;

        case '0': {
            // Hexadecimal
            if (at[0] == 'x') {
                token.type = Token_Constant_Hexadecimal;
                token.length++;
                Advance();
                while(IsNumber(at[0]) || at[0] >= 'a' && at[0] <= 'f') {
                    token.length++;
                    Advance();
                }
            }

            // Binary
            else if (at[0] == 'b') {
                token.type = Token_Constant_Binary;
                token.length++;
                Advance();
                while(at[0] == '0' || at[0] == '1') {
                    token.length++;
                    Advance();
                }
            }

            // Decimal
            else {
                while (IsNumber(at[0])) {
                    token.length++;
                    Advance();
                }
                if (at[0] == '.') {
                    token.type = Token_Constant_Decimal;
                    token.length++;
                    Advance();
                    while(IsNumber(at[0])) {
                        token.length++;
                        Advance();
                    }
                    if (at[0] == 'f') {
                        token.length++;
                        Advance();
                    }
                }
                else {
                    token.type = Token_Constant_Integer;
                }
            }
        } break;

        default: {
            if (IsNumber(c)) {
                while (IsNumber(at[0])) {
                    token.length++;
                    Advance();
                }
                if (at[0] == '.') {
                    token.type = Token_Constant_Decimal;
                    token.length++;
                    Advance();
                    while(IsNumber(at[0])) {
                        token.length++;
                        Advance();
                    }
                    if (at[0] == 'f') {
                        token.length++;
                        Advance();
                    }
                    else if (at[0] == 'e') {
                        token.length++;
                        Advance();
                        if (at[0] == '-') {
                            token.length++;
                            Advance();
                        }
                        while(IsNumber(at[0])) {
                            token.length++;
                            Advance();
                        }
                    }
                }
                else {
                    token.type = Token_Constant_Integer;
                }
            }
            else if (IsAlphabet(c) || c == '_') {
                token.type = Token_Identifier;
                if (c == '_' && IsWhitespace(at[0])) {
                    token.type = Token_Underscore;
                }
                else while (IsAlphanumeric(at[0]) || at[0] == '_') {
                    token.length++;
                    Advance();
                }
            }
            else {
                Advance();
            }
        }
    }

    token.text = token_start;
    return token;
}
