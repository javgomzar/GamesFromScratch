#pragma once

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

struct Token {
    Token_Type type;
    const char* text;
    int length;
    int line;
    int column;

    bool operator==(Token& rhs);
    bool operator==(const char* str);
};

bool IsWhitespace(char C);
bool IsNumber(char C);
bool IsAlphabet(char C);
bool IsAlphanumeric(char C);
bool IsFileSeparator(char C);

struct Tokenizer {
    const char* at;
    const char* start;
    const char* end;
    int line;
    int column;
    int length;
    bool ignore_whitespace;

    Tokenizer(const char* text, int max_length = 0, bool should_ignore_whitespace = true);
    Tokenizer(void* memory, int max_length = 0, bool should_ignore_whitespace = true);
    bool Advance();
    void Advance(int char_count);
    void AdvanceUntil(char c);
    void AdvanceUntil(char c1, char c2);
    void AdvanceUntilLine(int Line);
    void AdvanceUntilNextLine();
    void AdvanceWhitespaceAndComments();
    Token GetToken();
};
