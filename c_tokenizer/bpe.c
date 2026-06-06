// bpe.c
#include "bpe.h"

#define MAX_WORDS    400
#define MAX_WORD_SEG  20
#define MAX_PAIRS   1500
#define NUM_MERGES    50

// Training corpus — repeated words drive interesting merges
static const char *corpus[] = {
    // ordering and commerce
    "order 5 laptops for the office",
    "order 10 monitors for the team",
    "buy 5 keyboards and monitors",
    "cancel the order for laptops",
    "buy the laptop for the office",
    "cancel the purchase",
    "the price is 500 dollars",
    "order the item online for 1200",
    "order 20 tablets for the school",
    "buy the printer for the office",
    "cancel the monitor order today",
    "the shipping price is very high",
    "order the keyboard and the mouse",
    "buy 3 laptops and 5 monitors",
    "the total price is 3500 dollars",
    "cancel the purchase and get a refund",
    "order the items for fast delivery",
    "the delivery time is 5 days",
    "buy the cheapest laptop online",
    "the order was cancelled by the user",
    // tokenization and language models
    "the model learns from tokens",
    "tokens are the units of language",
    "the tokenizer processes every word",
    "language models process text tokens",
    "the model splits the input into tokens",
    "learning the patterns in text data",
    "the training data contains many tokens",
    "tokens can be words or subwords",
    "the model processes the input text",
    "language processing requires many tokens",
    "the tokenizer learns from the training data",
    "subword tokens are better than word tokens",
    "the model learns patterns from the data",
    "processing text is the first step",
    "the language model generates tokens",
    "tokens are mapped to integer ids",
    "the encoder splits text into subword tokens",
    "learning subwords helps the model generalize",
    "the model reads tokens not words",
    "tokenization is the first step in processing",
    // general computing
    "the computer processes the data very fast",
    "processing text requires splitting into tokens",
    "the system learns patterns from data",
    "data processing is fast and efficient",
    "the algorithm finds the best patterns in data",
    "the program reads the input and processes it",
    "splitting the input is the first step",
    "the output is a list of tokens",
    "the system splits text into smaller parts",
    "learning from data is how models improve",
    NULL
};

// Internal: one unique word from the corpus with its current segmentation
typedef struct {
    char word[MAX_SUB_LEN];
    char segs[MAX_WORD_SEG][MAX_SUB_LEN];
    int  seg_len;
    int  freq;
} WordEntry;

// Internal: an adjacent-pair count during one merge round
typedef struct {
    char left[MAX_SUB_LEN];
    char right[MAX_SUB_LEN];
    int  freq;
} PairCount;

static WordEntry words[MAX_WORDS];
static int num_words;

// --- helpers ---

static int vocab_id(BPEModel *model, const char *tok) {
    for (int i = 0; i < model->vocab_size; i++)
        if (strcmp(model->vocab[i].token, tok) == 0)
            return model->vocab[i].id;
    return -1;
}

static void vocab_add(BPEModel *model, const char *tok) {
    if (vocab_id(model, tok) >= 0 || model->vocab_size >= MAX_VOCAB) return;
    strncpy(model->vocab[model->vocab_size].token, tok, MAX_SUB_LEN - 1);
    model->vocab[model->vocab_size].token[MAX_SUB_LEN - 1] = '\0';
    model->vocab[model->vocab_size].id = model->vocab_size;
    model->vocab_size++;
}

// --- public API ---

void bpe_train(BPEModel *model) {
    model->vocab_size = 0;
    model->num_merges = 0;
    num_words = 0;

    printf("\n===========================================\n");
    printf("  STAGE 0: BPE TRAINING\n");
    printf("===========================================\n");
    printf("Training corpus:\n");

    // Step 1: parse corpus into unique word entries with frequencies
    for (int s = 0; corpus[s] != NULL; s++) {
        printf("  \"%s\"\n", corpus[s]);
        const char *p = corpus[s];
        while (*p) {
            while (*p == ' ') p++;
            if (!*p) break;

            char word[MAX_SUB_LEN];
            int wlen = 0;
            while (*p && *p != ' ' && wlen < MAX_SUB_LEN - 1)
                word[wlen++] = (char)tolower((unsigned char)*p++);
            word[wlen] = '\0';
            if (wlen == 0) continue;

            int found = -1;
            for (int w = 0; w < num_words; w++) {
                if (strcmp(words[w].word, word) == 0) { found = w; break; }
            }
            if (found == -1 && num_words < MAX_WORDS) {
                found = num_words++;
                strncpy(words[found].word, word, MAX_SUB_LEN - 1);
                words[found].word[MAX_SUB_LEN - 1] = '\0';
                words[found].freq = 0;
                words[found].seg_len = wlen;
                for (int c = 0; c < wlen && c < MAX_WORD_SEG; c++) {
                    words[found].segs[c][0] = word[c];
                    words[found].segs[c][1] = '\0';
                }
            }
            if (found >= 0) words[found].freq++;
        }
    }

    // Step 2: seed vocab from every unique character in the corpus
    // (scanning raw strings so spaces, digits, and punct are included)
    for (int s = 0; corpus[s] != NULL; s++) {
        for (int c = 0; corpus[s][c] != '\0'; c++) {
            char ch[2] = {(char)tolower((unsigned char)corpus[s][c]), '\0'};
            vocab_add(model, ch);
        }
    }

    printf("\nInitial character vocabulary (%d chars):\n  ", model->vocab_size);
    for (int i = 0; i < model->vocab_size; i++)
        printf("'%s' ", model->vocab[i].token);
    printf("\n");

    // Step 3: BPE merge loop — the core algorithm
    printf("\nLearning merge rules:\n");
    for (int round = 0; round < NUM_MERGES && model->num_merges < MAX_MERGES; round++) {

        // Count every adjacent pair across all word segmentations
        PairCount pairs[MAX_PAIRS];
        int num_pairs = 0;

        for (int w = 0; w < num_words; w++) {
            for (int s = 0; s < words[w].seg_len - 1; s++) {
                char *left  = words[w].segs[s];
                char *right = words[w].segs[s + 1];

                int found = -1;
                for (int p = 0; p < num_pairs; p++) {
                    if (strcmp(pairs[p].left, left) == 0 &&
                        strcmp(pairs[p].right, right) == 0) {
                        found = p; break;
                    }
                }
                if (found == -1 && num_pairs < MAX_PAIRS) {
                    found = num_pairs++;
                    strncpy(pairs[found].left,  left,  MAX_SUB_LEN - 1);
                    strncpy(pairs[found].right, right, MAX_SUB_LEN - 1);
                    pairs[found].left[MAX_SUB_LEN - 1]  = '\0';
                    pairs[found].right[MAX_SUB_LEN - 1] = '\0';
                    pairs[found].freq = 0;
                }
                if (found >= 0) pairs[found].freq += words[w].freq;
            }
        }

        if (num_pairs == 0) break;

        // Pick the most frequent pair
        int best = 0;
        for (int p = 1; p < num_pairs; p++)
            if (pairs[p].freq > pairs[best].freq) best = p;

        if (pairs[best].freq < 2) break;  // no useful merge left

        // Record the merge and add the new token to vocab
        BPEMerge *m = &model->merges[model->num_merges++];
        strncpy(m->left,  pairs[best].left,  MAX_SUB_LEN - 1);
        strncpy(m->right, pairs[best].right, MAX_SUB_LEN - 1);
        m->left[MAX_SUB_LEN - 1] = m->right[MAX_SUB_LEN - 1] = '\0';
        snprintf(m->merged, MAX_SUB_LEN, "%s%s", m->left, m->right);
        vocab_add(model, m->merged);

        printf("  Merge #%02d: '%s' + '%s'  ->  '%s'   (freq: %d)\n",
               model->num_merges, m->left, m->right, m->merged, pairs[best].freq);

        // Apply this merge to every word's segmentation
        for (int w = 0; w < num_words; w++) {
            int s = 0;
            while (s < words[w].seg_len - 1) {
                if (strcmp(words[w].segs[s],     m->left)  == 0 &&
                    strcmp(words[w].segs[s + 1], m->right) == 0) {
                    strncpy(words[w].segs[s], m->merged, MAX_SUB_LEN - 1);
                    words[w].segs[s][MAX_SUB_LEN - 1] = '\0';
                    for (int k = s + 1; k < words[w].seg_len - 1; k++)
                        strncpy(words[w].segs[k], words[w].segs[k + 1], MAX_SUB_LEN - 1);
                    words[w].seg_len--;
                    // re-check position s — don't advance yet
                } else {
                    s++;
                }
            }
        }
    }

    printf("\nTraining complete. Vocabulary size: %d | Merge rules learned: %d\n",
           model->vocab_size, model->num_merges);
}

int bpe_encode(BPEModel *model, const char *input,
               int out_ids[], char out_tokens[][MAX_SUB_LEN]) {
    char segs[MAX_SEG][MAX_SUB_LEN];
    int seg_len = 0;

    // Split input into individual lowercase characters
    for (int i = 0; input[i] != '\0' && seg_len < MAX_SEG; i++) {
        segs[seg_len][0] = (char)tolower((unsigned char)input[i]);
        segs[seg_len][1] = '\0';
        seg_len++;
    }

    printf("  Char split : [");
    for (int i = 0; i < seg_len; i++) {
        if (i > 0) printf(", ");
        printf("'%s'", segs[i]);
    }
    printf("]\n\n");

    // Apply each merge rule in training order (same order as learned)
    for (int m = 0; m < model->num_merges; m++) {
        for (int s = 0; s < seg_len - 1; ) {
            if (strcmp(segs[s],     model->merges[m].left)  == 0 &&
                strcmp(segs[s + 1], model->merges[m].right) == 0) {
                strncpy(segs[s], model->merges[m].merged, MAX_SUB_LEN - 1);
                segs[s][MAX_SUB_LEN - 1] = '\0';
                for (int k = s + 1; k < seg_len - 1; k++)
                    strncpy(segs[k], segs[k + 1], MAX_SUB_LEN - 1);
                seg_len--;
                // re-check s — a new adjacent pair may now be mergeable
            } else {
                s++;
            }
        }
    }

    // Display results and populate output arrays
    printf("  After BPE  : [");
    for (int i = 0; i < seg_len; i++) {
        if (i > 0) printf(", ");
        printf("'%s'", segs[i]);
        strncpy(out_tokens[i], segs[i], MAX_SUB_LEN - 1);
        out_tokens[i][MAX_SUB_LEN - 1] = '\0';
        out_ids[i] = vocab_id(model, segs[i]);
    }
    printf("]\n");

    printf("  Token IDs  : [");
    for (int i = 0; i < seg_len; i++) {
        if (i > 0) printf(", ");
        if (out_ids[i] < 0)
            printf(" ?");
        else
            printf("%2d", out_ids[i]);
    }
    printf("]\n");
    printf("  (? = out of vocabulary — token not seen during training)\n");

    return seg_len;
}

void bpe_print_vocab(BPEModel *model) {
    printf("\n--- BPE VOCABULARY ---\n");

    printf("Base chars:\n  ");
    int col = 0;
    for (int i = 0; i < model->vocab_size; i++) {
        if (strlen(model->vocab[i].token) == 1) {
            printf("[%3d]='%s'  ", model->vocab[i].id, model->vocab[i].token);
            if (++col % 10 == 0) printf("\n  ");
        }
    }

    printf("\nLearned subword tokens:\n  ");
    col = 0;
    for (int i = 0; i < model->vocab_size; i++) {
        if (strlen(model->vocab[i].token) > 1) {
            printf("[%3d]='%-10s'  ", model->vocab[i].id, model->vocab[i].token);
            if (++col % 5 == 0) printf("\n  ");
        }
    }
    printf("\n");
}
