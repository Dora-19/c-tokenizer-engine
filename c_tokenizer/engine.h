//
//  engine.h
//  C_project
//
//  Created by Dora Alkan on 5.06.2026.
//
// engine.h
#ifndef ENGINE_H
#define ENGINE_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKENS 50
#define MAX_TOKEN_LEN 30

// Token Types (Kategoriler)
typedef enum {
    TOKEN_WORD,       // Harflerden oluşan kelimeler
    TOKEN_NUMBER,     // Tam sayılar veya ondalıklı sayılar
    TOKEN_PUNCT,      // Noktalama işaretleri ($, ,, !)
    TOKEN_UNKNOWN     // Bilinmeyen karakterler
} TokenType;

// Token Data Structure (Token Yapısı)
typedef struct {
    char value[MAX_TOKEN_LEN];
    TokenType type;
} Token;

// Function Prototypes
int tokenize_input(const char *input, Token tokens[]);
void print_tokens(Token tokens[], int count);
void parse_tokens(Token tokens[], int count);

#endif
