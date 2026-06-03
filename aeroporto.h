#ifndef AEROPORTO_H
#define AEROPORTO_H

#include "matriz_esparsa.h"

/*
 * ═══════════════════════════════════════════════════════════
 *  MÓDULO: Aeroporto / Grafo
 *
 *  Este módulo usa a matriz esparsa (matriz_esparsa.h) para
 *  representar um grafo de aeroportos (vértices) e voos (arestas).
 *
 *  ESTRUTURA DO GRAFO:
 *    - aeroportos[]: vetor dinâmico de Aeroporto (vértices)
 *    - matriz:       MatrizEsparsa de Voo (arestas)
 *    - linha i / coluna j da matriz = voo saindo de aeroportos[i]
 *                                     chegando em aeroportos[j]
 *
 *  ORDEM DE CHAMADAS (do ponto de vista do main.c):
 *
 *    main()
 *      │
 *      ├─ criarGrafo()           ← cria vetor + matriz esparsa
 *      │
 *      ├─ cadastrarAeroporto()   ← adiciona vértice ao vetor
 *      │     └─ expandirVetor()       (se vetor cheio)
 *      │
 *      ├─ cadastrarVoo()         ← adiciona aresta na matriz
 *      │     └─ inserirElemento()     (de matriz_esparsa.c)
 *      │
 *      ├─ removerVoo()           ← remove aresta pelo número do voo
 *      │     └─ removerElemento()     (de matriz_esparsa.c)
 *      │
 *      ├─ listarVoos()           ← lista saídas de um aeroporto
 *      │     └─ primeiroDaLinha()     (de matriz_esparsa.c)
 *      │
 *      ├─ listarTrajetos()       ← DFS: todos os caminhos entre dois aeroportos
 *      │     └─ dfs() [interno]       percorre recursivamente
 *      │
 *      └─ destruirGrafo()        ← libera vetor + matriz esparsa
 *            └─ destruirMatriz()      (de matriz_esparsa.c)
 * ═══════════════════════════════════════════════════════════
 */

#define MAX_CODIGO                  8
#define MAX_CIDADE                 64
#define CAPACIDADE_INICIAL_AEROPORTOS 10

/* ─── Estrutura do aeroporto (vértice do grafo) ─── */
typedef struct {
    char codigo[MAX_CODIGO];  /* ex.: "GRU" */
    char cidade[MAX_CIDADE];  /* ex.: "Sao Paulo" */
} Aeroporto;

/* ─── Dado armazenado na aresta (voo) ─── */
typedef struct {
    int numero;  /* número do voo */
} Voo;

/* ─── Grafo completo ─── */
typedef struct {
    Aeroporto    *aeroportos;  /* vetor dinâmico de vértices */
    int           quantidade;  /* quantos aeroportos estão cadastrados */
    int           capacidade;  /* tamanho atual do vetor alocado */
    MatrizEsparsa *matriz;     /* matriz esparsa de arestas (voos) */
} Grafo;

/* ─── Protótipos ─── */
Grafo *criarGrafo(void);
void   destruirGrafo(Grafo *g);

int  cadastrarAeroporto(Grafo *g, const char *codigo, const char *cidade);
int  indiceAeroporto(Grafo *g, const char *codigo);
int  encolherVetor(Grafo *g);

int  cadastrarVoo(Grafo *g, int numeroVoo,
                  const char *codOrigem, const char *codDestino);
int  removerVoo(Grafo *g, int numeroVoo);

void listarVoos(Grafo *g, const char *codOrigem);
void listarTrajetos(Grafo *g, const char *codOrigem, const char *codDestino);

#endif /* AEROPORTO_H */
