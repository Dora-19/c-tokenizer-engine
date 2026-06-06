// bpe.h
#ifndef BPE_H
#define BPE_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_VOCAB   300   // base chars + learned merge tokens
#define MAX_MERGES   60   // merge rules we can store (must be >= NUM_MERGES in bpe.c)
#define MAX_SEG     250   // max segments when encoding (input up to 200 chars)
#define MAX_SUB_LEN  30   // max chars in one subword token

// One BPE merge rule: left + right -> merged
typedef struct {
    char left[MAX_SUB_LEN];
    char right[MAX_SUB_LEN];
    char merged[MAX_SUB_LEN];
} BPEMerge;

// One entry in the vocabulary: token string -> integer ID
typedef struct {
    char token[MAX_SUB_LEN];
    int  id;
} VocabEntry;

// The full trained BPE model
typedef struct {
    BPEMerge   merges[MAX_MERGES];
    int        num_merges;
    VocabEntry vocab[MAX_VOCAB];
    int        vocab_size;
} BPEModel;

void bpe_train(BPEModel *model);
int  bpe_encode(BPEModel *model, const char *input,
                int out_ids[], char out_tokens[][MAX_SUB_LEN]);
void bpe_print_vocab(BPEModel *model);

#endif
