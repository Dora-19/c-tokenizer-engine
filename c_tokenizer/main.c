// main.c
#include "engine.h"
#include "bpe.h"

int main(void) {
    char input_buffer[200];
    Token my_tokens[MAX_TOKENS];

    static BPEModel bpe_model;  // large struct — keep off the call stack
    int  bpe_ids[MAX_SEG];
    char bpe_tok[MAX_SEG][MAX_SUB_LEN];

    printf("===========================================\n");
    printf("   ADVANCED NLP LEXER & PARSER ENGINE (C)  \n");
    printf("===========================================\n");

    // Stage 0: train BPE on the hardcoded corpus
    bpe_train(&bpe_model);
    bpe_print_vocab(&bpe_model);

    printf("\nExample: 'Order 5 laptops for $1200.50!'\n");
    printf("Enter your prompt: ");

    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL)
        return 0;
    input_buffer[strcspn(input_buffer, "\n")] = '\0';

    // Stage 1: word-level lexer (the classic approach — similar to what strtok does)
    printf("\n===========================================\n");
    printf("  STAGE 1: WORD-LEVEL LEXER\n");
    printf("  (classic approach — splits by char type,\n");
    printf("   like strtok but with type labels)\n");
    printf("===========================================\n");
    int num_tokens = tokenize_input(input_buffer, my_tokens);
    print_tokens(my_tokens, num_tokens);

    // Stage 2: BPE tokenizer — how an LLM actually sees the same text
    printf("\n===========================================\n");
    printf("  STAGE 2: BPE TOKENIZER\n");
    printf("  (subword units + integer IDs — what GPT\n");
    printf("   and LLaMA actually receive as input)\n");
    printf("===========================================\n");
    int bpe_count = bpe_encode(&bpe_model, input_buffer, bpe_ids, bpe_tok);
    printf("\n  Word-level tokens : %d\n", num_tokens);
    printf("  BPE tokens        : %d\n", bpe_count);

    // Stage 3: NLP parser extracts structured intent from the word-level tokens
    printf("\n===========================================\n");
    printf("  STAGE 3: NLP PARSER (intent extraction)\n");
    printf("===========================================\n");
    parse_tokens(my_tokens, num_tokens);

    return 0;
}
