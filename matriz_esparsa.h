#ifndef MATRIZ_ESPARSA_H
#define MATRIZ_ESPARSA_H

#include <stdio.h>
#include <stdlib.h>

/*
 * ═══════════════════════════════════════════════════════════
 *  MÓDULO: Matriz Esparsa Genérica
 *
 *  Este módulo é a base de tudo. Ele implementa uma matriz
 *  esparsa com listas encadeadas cruzadas (uma por linha e
 *  uma por coluna). Cada célula da matriz guarda um dado
 *  genérico (void*), por isso serve para qualquer tipo.
 *
 *  FLUXO GERAL DESTE MÓDULO:
 *  1. criarMatriz()     → aloca e inicializa a estrutura vazia
 *  2. inserirElemento() → cria um nó e o encadeia na linha E na coluna
 *  3. buscarElemento()  → percorre a lista da linha até achar a coluna
 *  4. removerElemento() → desencadeia o nó da linha E da coluna, libera memória
 *  5. destruirMatriz()  → libera tudo (nós pelas linhas, cabeçalhos de coluna separado)
 * ═══════════════════════════════════════════════════════════
 */

/* ─── Nó da lista encadeada (célula da matriz esparsa) ─── */
typedef struct No {
    int linha;
    int coluna;
    void *dado;            /* dado genérico: pode guardar qualquer struct */
    struct No *proxLinha;  /* próximo nó na mesma linha */
    struct No *proxColuna; /* próximo nó na mesma coluna */
} No;

/* ─── Cabeçalho de linha / coluna ─── */
typedef struct Cabecalho {
    int indice;
    struct Cabecalho *prox; /* próximo cabeçalho (linha ou coluna seguinte) */
    No *primeiro;           /* primeiro nó desta linha/coluna */
} Cabecalho;

/* ─── Matriz esparsa ─── */
typedef struct {
    Cabecalho *linhas;   /* lista de cabeçalhos de linha */
    Cabecalho *colunas;  /* lista de cabeçalhos de coluna */
    int numLinhas;
    int numColunas;
    size_t tamDado;      /* tamanho do dado armazenado (usado no memcpy) */
} MatrizEsparsa;

/* ─── Protótipos ─── */
MatrizEsparsa *criarMatriz(size_t tamDado);
void destruirMatriz(MatrizEsparsa *m);

int inserirElemento(MatrizEsparsa *m, int linha, int coluna, void *dado);
int removerElemento(MatrizEsparsa *m, int linha, int coluna);
void *buscarElemento(MatrizEsparsa *m, int linha, int coluna);

/* Retorna o primeiro nó de uma linha; útil para percorrer todas as arestas */
No *primeiroDaLinha(MatrizEsparsa *m, int linha);

#endif /* MATRIZ_ESPARSA_H */
