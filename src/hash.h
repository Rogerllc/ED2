#ifndef HASH_H
#define HASH_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASH_SIZE 142909
typedef struct No {
    char      *chave;     
    struct No *proximo;   
} No;
typedef struct {
    No         **tabela;    
    int          tamanho;   
    int          quantidade;
    long         colisoes;  
} TabelaHash;
TabelaHash *hash_criar(int tamanho);
void        hash_destruir(TabelaHash *h);
unsigned int hash_funcao(TabelaHash *h, const char *chave);
int         hash_inserir(TabelaHash *h, const char *chave);
int         hash_buscar(TabelaHash *h, const char *chave);
void        hash_estatisticas(TabelaHash *h);
#endif 
