#ifndef BLOOM_H
#define BLOOM_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#define BLOOM_BITS    958507   
#define BLOOM_HASHES  7        
#define BIT_SET(arr, i)   ((arr)[(i)/8] |=  (1 << ((i)%8)))
#define BIT_GET(arr, i)   ((arr)[(i)/8] &   (1 << ((i)%8)))
typedef struct {
    uint8_t *bits;        
    int      m;           
    int      k;           
    int      quantidade;  
} FiltroBloom;
FiltroBloom *bloom_criar(int m, int k);
void         bloom_destruir(FiltroBloom *f);
unsigned long bloom_hash1(const char *chave);
unsigned long bloom_hash2(const char *chave);
void         bloom_inserir(FiltroBloom *f, const char *chave);
int          bloom_consultar(FiltroBloom *f, const char *chave);
void         bloom_estatisticas(FiltroBloom *f);
#endif 
