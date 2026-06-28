#include "hash.h"
TabelaHash *hash_criar(int tamanho) {
    TabelaHash *h = (TabelaHash *)malloc(sizeof(TabelaHash));
    if (!h) {
        fprintf(stderr, "Erro: falha ao alocar TabelaHash\n");
        exit(EXIT_FAILURE);
    }
    h->tabela = (No **)calloc(tamanho, sizeof(No *)); 
    if (!h->tabela) {
        fprintf(stderr, "Erro: falha ao alocar vetor da tabela\n");
        free(h);
        exit(EXIT_FAILURE);
    }
    h->tamanho   = tamanho;
    h->quantidade = 0;
    h->colisoes  = 0;
    return h;
}
void hash_destruir(TabelaHash *h) {
    if (!h) return;
    for (int i = 0; i < h->tamanho; i++) {
        No *atual = h->tabela[i];
        while (atual) {
            No *prox = atual->proximo;
            free(atual->chave);
            free(atual);
            atual = prox;
        }
    }
    free(h->tabela);
    free(h);
}
unsigned int hash_funcao(TabelaHash *h, const char *chave) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*chave++)) {
        hash = ((hash << 5) + hash) ^ c; 
    }
    return (unsigned int)(hash % (unsigned long)h->tamanho);
}
int hash_inserir(TabelaHash *h, const char *chave) {
    unsigned int idx = hash_funcao(h, chave);
    No *atual = h->tabela[idx];
    if (atual != NULL) {
        h->colisoes++;
    }
    while (atual) {
        if (strcmp(atual->chave, chave) == 0) {
            return 0; 
        }
        atual = atual->proximo;
    }
    No *novo = (No *)malloc(sizeof( No));
    if (!novo) {
        fprintf(stderr, "Erro: falha ao alocar No\n");
        exit(EXIT_FAILURE);
    }
    novo->chave    = strdup(chave);
    novo->proximo  = h->tabela[idx];
    h->tabela[idx] = novo;
    h->quantidade++;
    return 1;
}
int hash_buscar(TabelaHash *h, const char *chave) {
    unsigned int idx = hash_funcao(h, chave);
    No *atual = h->tabela[idx];
    while (atual) {
        if (strcmp(atual->chave, chave) == 0) {
            return 1; 
        }
        atual = atual->proximo;
    }
    return 0; 
}
void hash_estatisticas(TabelaHash *h) {
    int ocupados = 0;
    int max_cadeia = 0;
    for (int i = 0; i < h->tamanho; i++) {
        if (h->tabela[i]) {
            ocupados++;
            int len = 0;
            No *n = h->tabela[i];
            while (n) { len++; n = n->proximo; }
            if (len > max_cadeia) max_cadeia = len;
        }
    }
    double fator_carga = (double)h->quantidade / h->tamanho;
    printf("\n=== Estatísticas da Tabela Hash ===\n");
    printf("  Tamanho da tabela   : %d\n",   h->tamanho);
    printf("  Elementos inseridos : %d\n",   h->quantidade);
    printf("  Buckets ocupados    : %d\n",   ocupados);
    printf("  Fator de carga      : %.4f\n", fator_carga);
    printf("  Colisões registradas: %ld\n",  h->colisoes);
    printf("  Maior cadeia        : %d nós\n", max_cadeia);
}
