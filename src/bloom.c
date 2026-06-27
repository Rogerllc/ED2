#include "bloom.h"
FiltroBloom *bloom_criar(int m, int k) {
    FiltroBloom *f = (FiltroBloom *)malloc(sizeof(FiltroBloom));
    if (!f) {
        fprintf(stderr, "Erro: falha ao alocar FiltroBloom\n");
        exit(EXIT_FAILURE);
    }
    int bytes = (m + 7) / 8; 
    f->bits = (uint8_t *)calloc(bytes, sizeof(uint8_t));
    if (!f->bits) {
        fprintf(stderr, "Erro: falha ao alocar vetor de bits\n");
        free(f);
        exit(EXIT_FAILURE);
    }
    f->m          = m;
    f->k          = k;
    f->quantidade = 0;
    return f;
}
void bloom_destruir(FiltroBloom *f) {
    if (!f) return;
    free(f->bits);
    free(f);
}
unsigned long bloom_hash1(const char *chave) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*chave++)) {
        hash = ((hash << 5) + hash) ^ (unsigned long)c;
    }
    return hash;
}
unsigned long bloom_hash2(const char *chave) {
    unsigned long hash = 0;
    int c;
    while ((c = (unsigned char)*chave++)) {
        hash = (unsigned long)c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
void bloom_inserir(FiltroBloom *f, const char *chave) {
    unsigned long h1 = bloom_hash1(chave);
    unsigned long h2 = bloom_hash2(chave);
    for (int i = 0; i < f->k; i++) {
        unsigned long pos = (h1 + (unsigned long)i * h2) % (unsigned long)f->m;
        BIT_SET(f->bits, pos);
    }
    f->quantidade++;
}
int bloom_consultar(FiltroBloom *f, const char *chave) {
    unsigned long h1 = bloom_hash1(chave);
    unsigned long h2 = bloom_hash2(chave);
    for (int i = 0; i < f->k; i++) {
        unsigned long pos = (h1 + (unsigned long)i * h2) % (unsigned long)f->m;
        if (!BIT_GET(f->bits, pos)) {
            return 0; 
        }
    }
    return 1; 
}
void bloom_estatisticas(FiltroBloom *f) {
    long bits_setados = 0;
    int bytes = (f->m + 7) / 8;
    for (int i = 0; i < bytes; i++) {
        bits_setados += __builtin_popcount(f->bits[i]);
    }
    double saturacao = (double)bits_setados / f->m * 100.0;
    double p_fp = 1.0;
    for (int i = 0; i < f->k; i++) {
        p_fp *= (double)bits_setados / f->m;
    }
    printf("\n=== Estatísticas do Filtro de Bloom ===\n");
    printf("  Tamanho do vetor (m): %d bits (%.2f KB)\n",
           f->m, (double)((f->m+7)/8) / 1024.0);
    printf("  Funções hash (k)    : %d\n",      f->k);
    printf("  Elementos inseridos : %d\n",      f->quantidade);
    printf("  Bits setados        : %ld\n",     bits_setados);
    printf("  Saturação do vetor  : %.2f%%\n",  saturacao);
    printf("  FP estimado (teórico): %.4f%%\n", p_fp * 100.0);
}
