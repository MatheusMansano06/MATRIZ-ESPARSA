#include "matriz_esparsa.h"
#include <string.h>

/*
 * ═══════════════════════════════════════════════════════════
 *  MÓDULO: Matriz Esparsa — Implementação
 *
 *  Aqui ficam as funções internas (auxiliares de cabeçalho)
 *  e as funções públicas declaradas em matriz_esparsa.h.
 *
 *  SEQUÊNCIA DE CHAMADAS TÍPICA (vista de aeroporto.c):
 *
 *  criarGrafo()
 *    └─► criarMatriz()          ← inicializa a matriz vazia
 *
 *  cadastrarVoo()
 *    └─► inserirElemento()
 *          ├─► buscarOuCriarCabecalho() [linha]   ← garante que a linha existe
 *          ├─► buscarOuCriarCabecalho() [coluna]  ← garante que a coluna existe
 *          └─► encadeia o novo nó nas duas listas
 *
 *  removerVoo()
 *    └─► removerElemento()
 *          ├─► buscarCabecalho() [linha e coluna]
 *          ├─► desencadeia o nó de ambas as listas
 *          ├─► free(nó)
 *          └─► removerCabecalhoSeVazio() [remove linha/coluna se ficou vazia]
 *
 *  listarVoos()
 *    └─► primeiroDaLinha()      ← pega o primeiro nó da linha e percorre proxLinha
 *
 *  destruirGrafo()
 *    └─► destruirMatriz()       ← libera nós pelas linhas, depois cabeçalhos de coluna
 * ═══════════════════════════════════════════════════════════
 */

/* ═══════════════════════════════════════════════════════════
 *  Funções auxiliares de cabeçalhos (uso interno apenas)
 * ═══════════════════════════════════════════════════════════ */

/*
 * buscarOuCriarCabecalho — chamada por inserirElemento()
 * Percorre a lista de cabeçalhos até encontrar o índice pedido.
 * Se não achar, cria um novo cabeçalho na posição certa (lista ordenada).
 * Retorna o ponteiro para o cabeçalho (existente ou novo).
 */
static Cabecalho *buscarOuCriarCabecalho(Cabecalho **lista, int indice) {
    Cabecalho *ant = NULL, *cur = *lista;
    while (cur && cur->indice < indice) {
        ant = cur;
        cur = cur->prox;
    }
    if (cur && cur->indice == indice)
        return cur;

    Cabecalho *novo = (Cabecalho *)malloc(sizeof(Cabecalho));
    if (!novo) return NULL;
    novo->indice   = indice;
    novo->prox     = cur;
    novo->primeiro = NULL;

    if (ant) ant->prox = novo;
    else     *lista    = novo;

    return novo;
}

/*
 * buscarCabecalho — chamada por removerElemento() e buscarElemento()
 * Só busca, não cria. Retorna NULL se o índice não existir.
 */
static Cabecalho *buscarCabecalho(Cabecalho *lista, int indice) {
    while (lista) {
        if (lista->indice == indice) return lista;
        lista = lista->prox;
    }
    return NULL;
}

/*
 * removerCabecalhoSeVazio — chamada no fim de removerElemento()
 * Depois de remover um nó, verifica se a linha/coluna ficou vazia.
 * Se ficou, remove o próprio cabeçalho para manter a estrutura limpa.
 */
static void removerCabecalhoSeVazio(Cabecalho **lista, int indice) {
    Cabecalho *ant = NULL, *cur = *lista;
    while (cur && cur->indice != indice) { ant = cur; cur = cur->prox; }
    if (!cur || cur->primeiro) return;
    if (ant) ant->prox = cur->prox;
    else     *lista    = cur->prox;
    free(cur);
}

/* ═══════════════════════════════════════════════════════════
 *  Criação / destruição
 * ═══════════════════════════════════════════════════════════ */

/*
 * criarMatriz — chamada por criarGrafo() em aeroporto.c
 * Aloca a estrutura e inicializa tudo como vazio/zero.
 * O parâmetro tamDado define o tamanho do tipo que será guardado
 * (ex: sizeof(Voo)) — isso que torna a matriz genérica.
 * Retorna NULL se não houver memória.
 */
MatrizEsparsa *criarMatriz(size_t tamDado) {
    MatrizEsparsa *m = (MatrizEsparsa *)malloc(sizeof(MatrizEsparsa));
    if (!m) return NULL;
    m->linhas     = NULL;
    m->colunas    = NULL;
    m->numLinhas  = 0;
    m->numColunas = 0;
    m->tamDado    = tamDado;
    return m;
}

/*
 * destruirMatriz — chamada por destruirGrafo() em aeroporto.c
 * Libera memória em duas passagens:
 *   1ª) percorre as linhas → libera cada nó (e seu dado interno)
 *   2ª) percorre os cabeçalhos de coluna → libera só os cabeçalhos
 *       (os nós já foram liberados na 1ª passagem)
 * Por fim libera a própria struct MatrizEsparsa.
 */
void destruirMatriz(MatrizEsparsa *m) {
    if (!m) return;

    /* 1ª passagem: libera nós pelas listas de linha */
    Cabecalho *cabLin = m->linhas;
    while (cabLin) {
        No *no = cabLin->primeiro;
        while (no) {
            No *prox = no->proxLinha;
            free(no->dado);
            free(no);
            no = prox;
        }
        Cabecalho *proxCab = cabLin->prox;
        free(cabLin);
        cabLin = proxCab;
    }

    /* 2ª passagem: libera apenas os cabeçalhos de coluna */
    Cabecalho *cabCol = m->colunas;
    while (cabCol) {
        Cabecalho *proxCab = cabCol->prox;
        free(cabCol);
        cabCol = proxCab;
    }

    free(m);
}

/* ═══════════════════════════════════════════════════════════
 *  Inserção
 * ═══════════════════════════════════════════════════════════ */

/*
 * inserirElemento — chamada por cadastrarVoo() em aeroporto.c
 *
 * Fluxo interno:
 *   1. Checa duplicata via buscarElemento() — retorna 0 se já existe
 *   2. buscarOuCriarCabecalho() para linha e coluna
 *   3. Aloca o nó e copia o dado (memcpy com tamDado bytes)
 *   4. Encadeia o nó na lista da linha (ordenado por coluna)
 *   5. Encadeia o nó na lista da coluna (ordenado por linha)
 *   6. Atualiza numLinhas / numColunas se necessário
 *
 * Retorna: 1 = inseriu, 0 = já existia, -1 = erro de memória
 */
int inserirElemento(MatrizEsparsa *m, int linha, int coluna, void *dado) {
    if (buscarElemento(m, linha, coluna)) return 0;

    Cabecalho *cabLin = buscarOuCriarCabecalho(&m->linhas,  linha);
    Cabecalho *cabCol = buscarOuCriarCabecalho(&m->colunas, coluna);
    if (!cabLin || !cabCol) return -1;

    No *novo = (No *)malloc(sizeof(No));
    if (!novo) return -1;
    novo->linha   = linha;
    novo->coluna  = coluna;
    novo->dado    = malloc(m->tamDado);
    if (!novo->dado) { free(novo); return -1; }
    memcpy(novo->dado, dado, m->tamDado);
    novo->proxLinha  = NULL;
    novo->proxColuna = NULL;

    /* Encadeia na lista da linha, mantendo ordem por coluna */
    No *ant = NULL, *cur = cabLin->primeiro;
    while (cur && cur->coluna < coluna) { ant = cur; cur = cur->proxLinha; }
    novo->proxLinha = cur;
    if (ant) ant->proxLinha   = novo;
    else     cabLin->primeiro = novo;

    /* Encadeia na lista da coluna, mantendo ordem por linha */
    ant = NULL; cur = cabCol->primeiro;
    while (cur && cur->linha < linha) { ant = cur; cur = cur->proxColuna; }
    novo->proxColuna = cur;
    if (ant) ant->proxColuna  = novo;
    else     cabCol->primeiro = novo;

    if (linha  >= m->numLinhas)  m->numLinhas  = linha  + 1;
    if (coluna >= m->numColunas) m->numColunas = coluna + 1;

    return 1;
}

/* ═══════════════════════════════════════════════════════════
 *  Remoção
 * ═══════════════════════════════════════════════════════════ */

/*
 * removerElemento — chamada por removerVoo() em aeroporto.c
 *
 * Fluxo interno:
 *   1. buscarCabecalho() para linha e coluna — retorna 0 se não existem
 *   2. Percorre lista da linha até achar o nó com a coluna certa
 *   3. Desencadeia o nó da lista da linha
 *   4. Percorre lista da coluna e desencadeia o mesmo nó
 *   5. Libera dado e nó
 *   6. removerCabecalhoSeVazio() para linha e coluna
 *
 * Retorna: 1 = removeu, 0 = não encontrou
 */
int removerElemento(MatrizEsparsa *m, int linha, int coluna) {
    Cabecalho *cabLin = buscarCabecalho(m->linhas,  linha);
    Cabecalho *cabCol = buscarCabecalho(m->colunas, coluna);
    if (!cabLin || !cabCol) return 0;

    /* Desencadeia da lista da linha */
    No *ant = NULL, *cur = cabLin->primeiro;
    while (cur && cur->coluna != coluna) { ant = cur; cur = cur->proxLinha; }
    if (!cur) return 0;
    if (ant) ant->proxLinha   = cur->proxLinha;
    else     cabLin->primeiro = cur->proxLinha;

    /* Desencadeia da lista da coluna */
    No *antC = NULL, *curC = cabCol->primeiro;
    while (curC && curC->linha != linha) { antC = curC; curC = curC->proxColuna; }
    if (curC) {
        if (antC) antC->proxColuna  = curC->proxColuna;
        else      cabCol->primeiro  = curC->proxColuna;
    }

    free(cur->dado);
    free(cur);

    removerCabecalhoSeVazio(&m->linhas,  linha);
    removerCabecalhoSeVazio(&m->colunas, coluna);

    return 1;
}

/* ═══════════════════════════════════════════════════════════
 *  Busca
 * ═══════════════════════════════════════════════════════════ */

/*
 * buscarElemento — chamada por inserirElemento() (checa duplicata)
 *                  e por removerVoo() (checa se existe antes de remover)
 *
 * Fluxo: acha o cabeçalho da linha → percorre nós até achar a coluna.
 * Retorna ponteiro para o dado, ou NULL se não encontrar.
 */
void *buscarElemento(MatrizEsparsa *m, int linha, int coluna) {
    Cabecalho *cabLin = buscarCabecalho(m->linhas, linha);
    if (!cabLin) return NULL;
    No *cur = cabLin->primeiro;
    while (cur) {
        if (cur->coluna == coluna) return cur->dado;
        cur = cur->proxLinha;
    }
    return NULL;
}

/*
 * primeiroDaLinha — chamada por listarVoos() e pela DFS em aeroporto.c
 * Retorna o primeiro nó da lista da linha pedida.
 * Quem chamar percorre o restante usando no->proxLinha.
 * Retorna NULL se a linha não tiver nenhum elemento.
 */
No *primeiroDaLinha(MatrizEsparsa *m, int linha) {
    Cabecalho *cabLin = buscarCabecalho(m->linhas, linha);
    if (!cabLin) return NULL;
    return cabLin->primeiro;
}
