# Sistema de Verificação de Cadastro de Usuários

Sistema eficiente de consulta combinando **Tabela Hash** e **Filtro de Bloom**, desenvolvido em C.

---

## Estrutura do Projeto

```
projeto/
├── src/
│   ├── hash.c       # Tabela Hash com encadeamento externo
│   ├── hash.h
│   ├── bloom.c      # Filtro de Bloom com double hashing
│   ├── bloom.h
│   └── main.c       # Menu, métricas e experimentos
├── data/
│   └── usuarios.txt # Arquivo de exemplo
├── testes/
└── README.md
```

---

## Compilação

### Pré-requisitos
- GCC (versão 7 ou superior)
- Biblioteca matemática `libm`

### Comando

```bash
gcc -O2 -Wall -Wextra -o sistema src/hash.c src/bloom.c src/main.c -lm
```

Para debug (com símbolos):
```bash
gcc -g -Wall -o sistema src/hash.c src/bloom.c src/main.c -lm
```

---

## Execução

```bash
./sistema
```

O programa exibe um menu interativo.

---

## Formato de Entrada

### Identificador de usuário
String alfanumérica, sem espaços.

**Formato gerado automaticamente pelos experimentos:**
```
[8 letras minúsculas][3 dígitos]
```
Exemplos:
```
islaifda122
djskalsa297
fjkldsaf881
```

### Arquivo de usuários (`data/usuarios.txt`)
Um identificador por linha:
```
joao123
maria98
pedro45
```

---

## Exemplos de Execução

### Inserção (opção 1)
```
ID do usuário: joao123
✔ Usuário 'joao123' cadastrado com sucesso.
```

### Consulta (opção 2)
```
ID a consultar: joao123
✔ Usuário encontrado.

ID a consultar: ana777
✘ Usuário inexistente (filtro descartou).
```

### Carregar arquivo (opção 4)
```
Caminho do arquivo: data/usuarios.txt
✔ 3 registros carregados de 'data/usuarios.txt'.
```

### Experimentos automatizados (opção 5)
Gera arquivos com 1.000, 10.000 e 100.000 registros, executa os
experimentos e exibe a tabela comparativa:

```
Gerando arquivos de teste...
Arquivo 'data/usuarios_1k.txt' gerado com 1000 registros.
...
--- Experimento: 1000 registros ---
  Tempo SEM Bloom      : 0.31 ms
  Tempo COM Bloom      : 0.18 ms
  Speedup              : 1.72x
  Falsos positivos     : 2
  Taxa FP (real)       : 0.2000%
```

---

## Decisões de Projeto

### Tabela Hash
| Parâmetro | Valor | Justificativa |
|-----------|-------|---------------|
| Tamanho   | 142.909 (primo) | Fator de carga ≈ 0,70 para 100k elementos |
| Colisão   | Encadeamento externo | Robusto para variações de carga |
| Função    | djb2 com XOR | Boa distribuição, baixo custo |

### Filtro de Bloom
| Parâmetro | Valor | Justificativa |
|-----------|-------|---------------|
| Tamanho (m) | 958.507 bits | Fórmula: m = -(n·ln p)/(ln 2)² com n=100k, p=1% |
| Funções hash (k) | 7 | k = (m/n)·ln 2 ≈ 6,64 → arredondado para 7 |
| Técnica | Double hashing | Kirsch & Mitzenmacher (2006) |

---

## Referências

- KNUTH, D. E. *The Art of Computer Programming*, Vol. 3. Addison-Wesley, 1998.
- BLOOM, B. H. Space/time trade-offs in hash coding with allowable errors. *CACM*, 1970.
- KIRSCH, A.; MITZENMACHER, M. Less hashing, same performance. *ESA*, 2006.
- RAMAKRISHNA, M. V. Practical performance of bloom filters. *CACM*, 1989.
