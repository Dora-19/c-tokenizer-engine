//
//  main.c
//  C_project
//
//  Created by Dora Alkan on 31.03.2026.
//
// main.c
#include "engine.h"

int main(void) {
    char input_buffer[200];
    Token my_tokens[MAX_TOKENS]; // Token dizimizi oluşturuyoruz

    printf("===========================================\n");
    printf("   ADVANCED NLP LEXER & PARSER ENGINE (C)  \n");
    printf("===========================================\n");
    printf("Example: 'Order 5 laptops for $1200.50!'\n");
    printf("Enter your prompt: ");

    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = '\0'; // Sondaki Enter'ı temizle
        
        // Adım 1: Cümleyi Token'lara Ayır (Lexical Analysis)
        int num_tokens = tokenize_input(input_buffer, my_tokens);
        
        // Adım 2: Oluşan Token'ları Ekrana Bas (Şov Kısmı)
        print_tokens(my_tokens, num_tokens);
        
        // Adım 3: Token'ları Anlamlandır ve Veriyi Çek
        parse_tokens(my_tokens, num_tokens);
    }

    return 0;
}
