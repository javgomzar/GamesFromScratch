#pragma once

#include <platform.h>
#include "tokenizer.h"


void ReportIncorrectToken(Token token, const char* text);
Token RequireToken(Tokenizer& tokenizer, const char* text);
void ReportIncorrectTokenType(Token token, Token_Type type);
Token RequireToken(Tokenizer& tokenizer, Token_Type type);

bool ParseBool(Tokenizer& tokenizer);
unsigned int Parseuint32(Tokenizer& tokenizer);
bool ParseSign(Tokenizer& tokenizer, OUT Token& number);
int ParseInt(Tokenizer& tokenizer);
float ParseFloat(Tokenizer& tokenizer);
bool ParseDouble(Tokenizer& tokenizer, double& result);
int ParsePath(const char* text);
