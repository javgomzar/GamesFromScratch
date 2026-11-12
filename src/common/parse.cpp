#include "pch.h"
#include "parse.h"


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

void ReportIncorrectToken(Token token, const char* text) {
    char error_buffer[256];
    char token_text[MAX_TOKEN_LENGTH] = "";
    strncpy(token_text, token.text, token.length);
    sprintf(
        error_buffer, 
        "Token `%s` at line %d, column %d should be `%s`.", 
        token_text, token.line, token.column, text
    );
    Log::Fatal(error_buffer);
}

Token RequireToken(Tokenizer& tokenizer, const char* text) {
    Token token = tokenizer.GetToken();
    if (token != text) {
        ReportIncorrectToken(token, text);
    }
    return token;
}

void ReportIncorrectTokenType(Token token, Token_Type type) {
    char error_buffer[256];
    char token_text[MAX_TOKEN_LENGTH] = {};
    strncpy(token_text, token.text, token.length);
    sprintf(
        error_buffer, 
        "Token `%s` at line %d, column %d is type '%s' but should be '%s'.", 
        token_text, token.line, token.column, TOKEN_TYPE_NAME[token.type], TOKEN_TYPE_NAME[type]
    );
    Log::Fatal(error_buffer);
}

Token RequireToken(Tokenizer& tokenizer, Token_Type type) {
    Token token = tokenizer.GetToken();
    if (token.type != type) {
        ReportIncorrectTokenType(token, type);
    }
    return token;
}

bool ParseBool(Tokenizer& tokenizer) {
    Token token = RequireToken(tokenizer, Token_Identifier);
    return token == "true" || token == "True" || token == "TRUE" || token == "1";
}

unsigned int Parseuint32(Tokenizer& tokenizer) {
    Token token = RequireToken(tokenizer, Token_Constant_Integer);
    char* end;
    return strtol(token.text, &end, 10);
}

bool ParseSign(Tokenizer& tokenizer, OUT Token& number) {
    number = tokenizer.GetToken();
    bool negative = number.type == Token_Minus;
    if (negative) {
        number = tokenizer.GetToken();
    }
    return negative;
}

int ParseInt(Tokenizer& tokenizer) {
    Token token;
    bool negative = ParseSign(tokenizer, token);
    if (token.type != Token_Constant_Integer) {
        ReportIncorrectTokenType(token, Token_Constant_Integer);
    }
    char* end;
    int result = strtol(token.text, &end, 10);
    result = negative ? -result : result;
    return result;
}

float ParseFloat(Tokenizer& tokenizer) {
    Token token;
    bool negative = ParseSign(tokenizer, token);
    bool valid = token.type == Token_Constant_Decimal || token.type == Token_Constant_Integer;
    if (!valid) {
        char error_buffer[256];
        char token_text[MAX_TOKEN_LENGTH] = {};
        strncpy(token_text, token.text, token.length);
        sprintf(
            error_buffer, 
            "Token `%s` at line %d, column %d is type '%s' but should be 'constant decimal' or 'constant integer'.", 
            token_text, token.line, token.column, TOKEN_TYPE_NAME[token.type]
        );
        Log::Fatal(error_buffer);
    }
    char* end;
    float result = strtof(token.text, &end);
    result = negative ? -result : result;
    return valid;
}

bool ParseDouble(Tokenizer& tokenizer, double& result) {
    Token token;
    bool negative = ParseSign(tokenizer, token);
    bool valid = token.type == Token_Constant_Decimal || token.type == Token_Constant_Integer;
    if (!valid) {
        char error_buffer[256];
        char token_text[MAX_TOKEN_LENGTH] = {};
        strncpy(token_text, token.text, token.length);
        sprintf(
            error_buffer, 
            "Token `%s` at line %d, column %d is type '%s' but should be 'constant decimal' or 'constant integer'.", 
            token_text, token.line, token.column, TOKEN_TYPE_NAME[token.type]
        );
        Log::Fatal(error_buffer);
    }
    char* end;
    result = strtod(token.text, &end);
    result = negative ? -result : result;
    return valid;
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
            return result;
        }
#endif
        result++;
        text++;
    }
    return result;
}

V2 ParseV2(Tokenizer& tokenizer) {
    float x = ParseFloat(tokenizer);
    float y = ParseFloat(tokenizer);
    return V2(x, y);
}

V3 ParseV3(Tokenizer& tokenizer) {
    float x = ParseFloat(tokenizer);
    float y = ParseFloat(tokenizer);
    float z = ParseFloat(tokenizer);
    return V3(x, y, z);
}

V4 ParseV4(Tokenizer& tokenizer) {
    float x = ParseFloat(tokenizer);
    float y = ParseFloat(tokenizer);
    float z = ParseFloat(tokenizer);
    float w = ParseFloat(tokenizer);
    return V4(x, y, z, w);
}

Quaternion ParseQuaternion(Tokenizer& tokenizer) {
    float c = ParseFloat(tokenizer);
    float i = ParseFloat(tokenizer);
    float j = ParseFloat(tokenizer);
    float k = ParseFloat(tokenizer);
    return Quaternion(c, i, j, k);
}

IV2 ParseIV2(Tokenizer& tokenizer) {
    int x = ParseInt(tokenizer);
    int y = ParseInt(tokenizer);
    return IV2(x, y);
}

IV3 ParseIV3(Tokenizer& tokenizer) {
    int x = ParseInt(tokenizer);
    int y = ParseInt(tokenizer);
    int z = ParseInt(tokenizer);
    return IV3(x, y, z);
}

UV2 ParseUV2(Tokenizer& tokenizer) {
    uint32 x = Parseuint32(tokenizer);
    uint32 y = Parseuint32(tokenizer);
    return UV2(x, y);
}

UV3 ParseUV3(Tokenizer& tokenizer) {
    uint32 x = Parseuint32(tokenizer);
    uint32 y = Parseuint32(tokenizer);
    uint32 z = Parseuint32(tokenizer);
    return UV3(x, y, z);
}
