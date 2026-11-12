#pragma once

#include <platform.h>
#include "tokenizer.h"
#include "mathematics.h"

inline constexpr int MAX_TOKEN_LENGTH = 128;

void ReportIncorrectToken(Token token, const char* text);
Token RequireToken(Tokenizer& tokenizer, const char* text);
void ReportIncorrectTokenType(Token token, Token_Type type);
Token RequireToken(Tokenizer& tokenizer, Token_Type type);

bool ParseBool(Tokenizer& tokenizer);
unsigned int Parseuint32(Tokenizer& tokenizer);
bool ParseSign(Tokenizer& tokenizer, Token& number);
int ParseInt(Tokenizer& tokenizer);
float ParseFloat(Tokenizer& tokenizer);
bool ParseDouble(Tokenizer& tokenizer, double& result);
int ParsePath(const char* text);
V2 ParseV2(Tokenizer& tokenizer);
V3 ParseV3(Tokenizer& tokenizer);
V4 ParseV4(Tokenizer& tokenizer);
Quaternion ParseQuaternion(Tokenizer& tokenizer);
IV2 ParseIV2(Tokenizer& tokenizer);
IV3 ParseIV3(Tokenizer& tokenizer);
UV2 ParseUV2(Tokenizer& tokenizer);
UV3 ParseUV3(Tokenizer& tokenizer);
