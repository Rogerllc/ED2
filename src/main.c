#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

/* Bloco para criar pastas automaticamente (Cross-Platform) */
#ifdef _WIN32
    #include <direct.h>
    #define CRIAR_PASTA(dir) _mkdir(dir)
#else
    #include <sys/stat.h>
    #define CRIAR_PASTA(dir) mkdir(dir, 0777)
#endif

#include "hash.h"
#include "bloom.h"

#define MAX_ID     64     
#define NOME_LEN   8      
#define NUM_LEN    3      

typedef struct {
    long consultas_total;      
    long evitadas_bloom;       
    long falsos_positivos;     
    long verdadeiros_positivos;
    double tempo_total_s;      
} Metricas;

static void gerar_id_aleatorio(char *buf) {
    static const char letras[] = "abcdefghijklmnopqrstuvwxyz";
    int i;
    for (i = 0; i < NOME_LEN; i++) {
        buf[i] = letras[rand() % 26];
    }
    for (i = 0; i < NUM_LEN; i++) {
        buf[NOME_LEN + i] = '0' + (rand() % 10);
    }
    buf[NOME_LEN + NUM_LEN] = '\0';
}

static int inserir_usuario(TabelaHash *h, FiltroBloom *f, const char *id) {
    int novo = hash_inserir(h, id);
    if (novo) {
        bloom_inserir(f, id);
    }
    return novo;
}

static int consultar_usuario(TabelaHash *h, FiltroBloom *f, Metricas *m, const char *id) {
    clock_t t0, t1;
    t0 = clock();
    
    m->consultas_total++;
    
    int bloom_res = bloom_consultar(f, id);
    
    if (!bloom_res) {
        m->evitadas_bloom++;
        t1 = clock();
        m->tempo_total_s += (double)(t1 - t0) / CLOCKS_PER_SEC;
        return 0;
    }
    
    int hash_res = hash_buscar(h, id);
    
    t1 = clock();
    m->tempo_total_s += (double)(t1 - t0) / CLOCKS_PER_SEC;
    
    if (hash_res) {
        m->verdadeiros_positivos++;
        return 1;  
    } else {
        m->falsos_positivos++;
        return -1; 
    }
}

static int carregar_arquivo(TabelaHash *h, FiltroBloom *f, const char *caminho) {
    FILE *arq = fopen(caminho, "r");
    if (!arq) {
        printf("[-] Erro: nao foi possivel abrir '%s'\n", caminho);
        return 0;
    }
    
    char linha[MAX_ID + 2];
    int  count = 0;
    
    while (fgets(linha, sizeof(linha), arq)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;
        if (inserir_usuario(h, f, linha)) {
            count++;
        }
    }
    fclose(arq);
    return count;
}

static void exibir_estatisticas(TabelaHash *h, FiltroBloom *f, Metricas *m) {
    printf("\n========================================\n");
    printf("        ESTATISTICAS DO SISTEMA         \n");
    printf("========================================\n");
    printf("  Elementos armazenados : %d\n",   h->quantidade);
    printf("  Consultas realizadas  : %ld\n",  m->consultas_total);
    printf("  Consultas evitadas    : %ld  (bloom disse 'nao')\n", m->evitadas_bloom);
    printf("  Verdadeiros positivos : %ld\n",  m->verdadeiros_positivos);
    printf("  Falsos positivos      : %ld\n",  m->falsos_positivos);
    
    if (m->consultas_total > 0) {
        double taxa_fp = (double)m->falsos_positivos / (double)m->consultas_total * 100.0;
        double t_medio = (m->tempo_total_s * 1e9) / (double)m->consultas_total;
        printf("  Taxa falsos positivos : %.4f%%\n",  taxa_fp);
        printf("  Tempo medio/consulta  : %.2f ns\n", t_medio);
    }
    hash_estatisticas(h);
    bloom_estatisticas(f);
}

static void gerar_arquivo_teste(const char *caminho, int n) {
    FILE *f = fopen(caminho, "w");
    if (!f) {
        printf("[-] Erro ao criar '%s'\n", caminho);
        return;
    }
    char id[MAX_ID];
    for (int i = 0; i < n; i++) {
        gerar_id_aleatorio(id);
        fprintf(f, "%s\n", id);
    }
    fclose(f);
    printf("[+] Arquivo '%s' gerado com %d registros.\n", caminho, n);
}

static void experimento(const char *caminho, int n) {
    printf("\n--- Experimento: %d registros (%s) ---\n", n, caminho);
    
    TabelaHash  *h = hash_criar(HASH_SIZE);
    FiltroBloom *f = bloom_criar(BLOOM_BITS, BLOOM_HASHES);
    Metricas     m = {0, 0, 0, 0, 0.0};
    
    int carregados = carregar_arquivo(h, f, caminho);
    if (carregados == 0) {
        hash_destruir(h); bloom_destruir(f); return;
    }
    printf("  Registros carregados : %d\n", carregados);
    
    FILE *arq = fopen(caminho, "r");
    if (!arq) { hash_destruir(h); bloom_destruir(f); return; }
    
    char **ids = (char **)malloc(n * sizeof(char *));
    if (!ids) { fclose(arq); hash_destruir(h); bloom_destruir(f); return; }
    
    char linha[MAX_ID + 2];
    int  total_ids = 0;
    
    while (total_ids < n && fgets(linha, sizeof(linha), arq)) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (strlen(linha) == 0) continue;
        ids[total_ids++] = strdup(linha);
    }
    fclose(arq);
    
    int consultas_total_exp = total_ids * 2;
    char **consultas = (char **)malloc(consultas_total_exp * sizeof(char *));
    if (!consultas) {
        for (int i = 0; i < total_ids; i++) free(ids[i]);
        free(ids);
        hash_destruir(h);
        bloom_destruir(f);
        return;
    }
    
    for (int i = 0; i < total_ids; i++) {
        consultas[i] = strdup(ids[i]);
    }
    for (int i = total_ids; i < consultas_total_exp; i++) {
        char id_novo[MAX_ID];
        do { gerar_id_aleatorio(id_novo); }
        while (hash_buscar(h, id_novo)); 
        consultas[i] = strdup(id_novo);
    }
    
    clock_t t0, t1;
    t0 = clock();
    
    for (int i = 0; i < consultas_total_exp; i++) {
        hash_buscar(h, consultas[i]);
    }
    
    t1 = clock();
    double tempo_sem_s = (double)(t1 - t0) / CLOCKS_PER_SEC;
    
    long fp_real = 0, evitadas = 0;
    t0 = clock();
    
    for (int i = 0; i < consultas_total_exp; i++) {
        int bloom_res = bloom_consultar(f, consultas[i]);
        if (!bloom_res) {
            evitadas++; 
        } else {
            int hr = hash_buscar(h, consultas[i]);
            if (!hr) fp_real++; 
        }
    }
    
    t1 = clock();
    double tempo_com_s = (double)(t1 - t0) / CLOCKS_PER_SEC;
    
    long ausentes = (long)total_ids;
    double taxa_fp_real = (ausentes > 0) ? (double)fp_real / ausentes * 100.0 : 0.0;
    
    printf("  Total de consultas   : %d (50%% presentes, 50%% ausentes)\n", consultas_total_exp);
    printf("  Tempo SEM Bloom      : %.2f ms\n", tempo_sem_s * 1000.0);
    printf("  Tempo COM Bloom      : %.2f ms\n", tempo_com_s * 1000.0);
    printf("  Speedup              : %.2fx\n", (tempo_com_s > 0) ? tempo_sem_s / tempo_com_s : 0.0);
    printf("  Consultas evitadas   : %ld / %d  (%.1f%%)\n", evitadas, consultas_total_exp, (double)evitadas / consultas_total_exp * 100.0);
    printf("  Falsos positivos     : %ld\n", fp_real);
    printf("  Taxa FP (real)       : %.4f%%\n", taxa_fp_real);
    
    for (int i = 0; i < consultas_total_exp; i++) free(consultas[i]);
    free(consultas);
    for (int i = 0; i < total_ids; i++) free(ids[i]);
    free(ids);
    
    hash_destruir(h);
    bloom_destruir(f);
    (void)m; 
}

static void menu_principal(void) {
    TabelaHash  *h = hash_criar(HASH_SIZE);
    FiltroBloom *f = bloom_criar(BLOOM_BITS, BLOOM_HASHES);
    Metricas     m = {0, 0, 0, 0, 0.0};
    char opcao[16];
    char id[MAX_ID];
    
    printf("\n========================================\n");
    printf("  Sistema de Verificacao de Usuarios  \n");
    printf("========================================\n");
    
    while (1) {
        printf("\n[1] Inserir usuario\n");
        printf("[2] Consultar usuario\n");
        printf("[3] Estatisticas\n");
        printf("[4] Carregar arquivo\n");
        printf("[5] Experimentos automatizados\n");
        printf("[0] Sair\n");
        printf("Opcao: ");
        fflush(stdout);
        
        if (!fgets(opcao, sizeof(opcao), stdin)) break;
        opcao[strcspn(opcao, "\r\n")] = '\0';
        
        if (strcmp(opcao, "1") == 0) {
            printf("ID do usuario: ");
            fflush(stdout);
            if (!fgets(id, sizeof(id), stdin)) continue;
            id[strcspn(id, "\r\n")] = '\0';
            if (strlen(id) == 0) { printf("ID invalido.\n"); continue; }
            
            if (inserir_usuario(h, f, id)) {
                printf("[+] Usuario '%s' cadastrado com sucesso.\n", id);
            } else {
                printf("[-] Usuario '%s' ja existe.\n", id);
            }
        } else if (strcmp(opcao, "2") == 0) {
            printf("ID a consultar: ");
            fflush(stdout);
            if (!fgets(id, sizeof(id), stdin)) continue;
            id[strcspn(id, "\r\n")] = '\0';
            if (strlen(id) == 0) { printf("ID invalido.\n"); continue; }
            
            int res = consultar_usuario(h, f, &m, id);
            if (res == 1) {
                printf("[+] Usuario encontrado.\n");
            } else if (res == 0) {
                printf("[-] Usuario inexistente (filtro descartou).\n");
            } else {
                printf("[-] Usuario inexistente (falso positivo detectado).\n");
            }
        } else if (strcmp(opcao, "3") == 0) {
            exibir_estatisticas(h, f, &m);
        } else if (strcmp(opcao, "4") == 0) {
            char caminho[256];
            printf("Caminho do arquivo: ");
            fflush(stdout);
            if (!fgets(caminho, sizeof(caminho), stdin)) continue;
            caminho[strcspn(caminho, "\r\n")] = '\0';
            
            int count = carregar_arquivo(h, f, caminho);
            printf("[+] %d registros carregados de '%s'.\n", count, caminho);
        } else if (strcmp(opcao, "5") == 0) {
            printf("\nPreparando pasta e gerando arquivos de teste...\n");
            CRIAR_PASTA("data");
            srand((unsigned)time(NULL));
            
            gerar_arquivo_teste("data/usuarios_1k.txt",    1000);
            gerar_arquivo_teste("data/usuarios_10k.txt",  10000);
            gerar_arquivo_teste("data/usuarios_100k.txt",100000);
            
            printf("\n====== TABELA DE EXPERIMENTOS ======\n");
            printf("%-10s %-15s %-15s %-12s\n", "N", "Tempo s/ Bloom", "Tempo c/ Bloom", "FP (%)");
            printf("%-10s %-15s %-15s %-12s\n", "----------","---------------","---------------","----------");
            
            experimento("data/usuarios_1k.txt",    1000);
            experimento("data/usuarios_10k.txt",  10000);
            experimento("data/usuarios_100k.txt",100000);
        } else if (strcmp(opcao, "0") == 0) {
            printf("Encerrando sistema.\n");
            break;
        } else {
            printf("Opcao invalida.\n");
        }
    }
    hash_destruir(h);
    bloom_destruir(f);
}

int main(void) {
    srand((unsigned)time(NULL));
    menu_principal();
    return 0;
}