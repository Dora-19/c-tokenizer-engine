//
//  engine.c
//  C_project
//
//  Created by Dora Alkan on 5.06.2026.
//
// engine.c
#include "engine.h"

// 1. LEXICAL ANALYZER (Sözcük Analizörü): Cümleyi okur ve etiketli token'lara böler
int tokenize_input(const char *input, Token tokens[]) {
    int count = 0;
    int i = 0;
    
    while (input[i] != '\0' && count < MAX_TOKENS) {
        
        // 1. Boşlukları atla (isspace)
        if (isspace(input[i])) {
            i++;
            continue;
        }
        
        int j = 0;
        
        // 2. Sayı Yakalama (isdigit) - Ondalıklı noktaları da kabul eder
        if (isdigit(input[i])) {
            tokens[count].type = TOKEN_NUMBER;
            while ((isdigit(input[i]) || input[i] == '.') && j < MAX_TOKEN_LEN - 1) {
                tokens[count].value[j++] = input[i++];
            }
            tokens[count].value[j] = '\0'; // String sonlandırıcı
            count++;
        }
        // 3. Kelime Yakalama (isalpha) - Hemen küçük harfe çevirir (tolower)
        else if (isalpha(input[i])) {
            tokens[count].type = TOKEN_WORD;
            while (isalpha(input[i]) && j < MAX_TOKEN_LEN - 1) {
                tokens[count].value[j++] = tolower(input[i++]);
            }
            tokens[count].value[j] = '\0';
            count++;
        }
        // 4. Noktalama İşareti Yakalama (ispunct)
        else if (ispunct(input[i])) {
            tokens[count].type = TOKEN_PUNCT;
            tokens[count].value[j++] = input[i++];
            tokens[count].value[j] = '\0';
            count++;
        }
        // 5. Bilinmeyen (Emoji vb.)
        else {
            tokens[count].type = TOKEN_UNKNOWN;
            tokens[count].value[j++] = input[i++];
            tokens[count].value[j] = '\0';
            count++;
        }
    }
    return count; // Toplam oluşturulan token sayısını döndür
}

// 2. DEBUG FONKSİYONU: Arka planda token'ların nasıl etiketlendiğini gösterir
void print_tokens(Token tokens[], int count) {
    printf("\n--- TOKENIZATION STAGE (LEXER OUTPUT) ---\n");
    for (int i = 0; i < count; i++) {
        char *type_name = "";
        switch(tokens[i].type) {
            case TOKEN_WORD:   type_name = "WORD"; break;
            case TOKEN_NUMBER: type_name = "NUMBER"; break;
            case TOKEN_PUNCT:  type_name = "PUNCTUATION"; break;
            default:           type_name = "UNKNOWN"; break;
        }
        printf("Token %02d: [%-12s] -> Type: %s\n", i+1, tokens[i].value, type_name);
    }
}

// 3. PARSER (Ayrıştırıcı): Token listesini gezer ve mantıklı veri çıkarır
void parse_tokens(Token tokens[], int count) {
    char action[20] = "Unknown";
    int quantity = 0;
    double price = 0.0;
    char item[50] = "";

    // Sadece kategorize edilmiş token'lar üzerinde mantık yürütüyoruz
    for (int i = 0; i < count; i++) {
        
        // Eylem (Intent) Tespiti
        if (tokens[i].type == TOKEN_WORD) {
            if (strcmp(tokens[i].value, "buy") == 0 || strcmp(tokens[i].value, "order") == 0) {
                strcpy(action, "PURCHASE");
            }
            else if (strcmp(tokens[i].value, "cancel") == 0) {
                strcpy(action, "CANCEL");
            }
            // Bağlaç değilse, Eşya ismine ekle
            else if (strcmp(tokens[i].value, "for") != 0 && strcmp(tokens[i].value, "dollars") != 0 && strcmp(action, "Unknown") != 0) {
                strncat(item, tokens[i].value, 15);
                strcat(item, " ");
            }
        }
        
        // Sayısal Veri Tespiti (Rakam mı Fiyat mı?)
        if (tokens[i].type == TOKEN_NUMBER) {
            if (strchr(tokens[i].value, '.') != NULL) {
                // İçinde nokta varsa bu bir fiyattır (atof)
                price = atof(tokens[i].value);
            } else {
                // Yoksa bu bir adettir (atoi)
                quantity = atoi(tokens[i].value);
            }
        }
        
        // Ekstra: Yanındaki işareti kontrol et (Örn: $ işareti varsa direkt fiyat yap)
        if (tokens[i].type == TOKEN_PUNCT && strcmp(tokens[i].value, "$") == 0) {
            if (i + 1 < count && tokens[i+1].type == TOKEN_NUMBER) {
                price = atof(tokens[i+1].value);
            }
        }
    }

    printf("\n--- NLP PARSER EXTRACTION RESULTS ---\n");
    printf("Intent (Action) : %s\n", action);
    printf("Target (Item)   : %s\n", item);
    printf("Entity (Amount) : %d\n", quantity);
    printf("Entity (Price)  : $%.2f\n", price);
    printf("-------------------------------------\n\n");
}
