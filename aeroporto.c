#include "aeroporto.h"
#include <string.h>
#include <stdio.h>

/*
 * ═══════════════════════════════════════════════════════════
 *  MÓDULO: Aeroporto — Implementação
 *
 *  Este arquivo implementa as 5 operações pedidas no enunciado
 *  mais a criação/destruição do grafo.
 *
 *  SEQUÊNCIA GERAL DE EXECUÇÃO:
 *
 *   1. main() chama criarGrafo()
 *        → aloca o Grafo, o vetor de aeroportos e a MatrizEsparsa
 *
 *   2. menu chama cadastrarAeroporto() [operação 1]
 *        → adiciona aeroporto no vetor; se cheio, chama expandirVetor()
 *          que dobra a capacidade (realloc); se ficar com menos de 1/4
 *          da capacidade usada após uma remoção, encolherVetor() reduz pela metade
 *
 *   3. menu chama cadastrarVoo() [operação 2]
 *        → valida os aeroportos com indiceAeroporto()
 *        → chama inserirElemento() da matriz esparsa
 *
 *   4. menu chama removerVoo() [operação 3]
 *        → percorre todas as linhas da matriz procurando o número do voo
 *        → quando acha, chama removerElemento() da matriz esparsa
 *
 *   5. menu chama listarVoos() [operação 4]
 *        → usa primeiroDaLinha() para percorrer as arestas da linha do aeroporto
 *
 *   6. menu chama listarTrajetos() [operação 5]
 *        → chama dfs() recursivamente para encontrar todos os caminhos
 *
 *   7. main() chama destruirGrafo() ao sair
 *        → libera a matriz esparsa e depois o vetor de aeroportos
 * ═══════════════════════════════════════════════════════════
 */

/* ═══════════════════════════════════════════════════════════
 *  Criação / destruição do grafo
 * ═══════════════════════════════════════════════════════════ */

/*
 * criarGrafo — chamada pelo main() antes do menu
 * Aloca o Grafo, inicializa o vetor dinâmico de aeroportos com
 * capacidade inicial CAPACIDADE_INICIAL_AEROPORTOS e cria a
 * matriz esparsa com tamDado = sizeof(Voo).
 * Retorna NULL em qualquer falha de alocação.
 */
Grafo *criarGrafo(void) {
    Grafo *g = (Grafo *)malloc(sizeof(Grafo));
    if (!g) return NULL;

    g->capacidade = CAPACIDADE_INICIAL_AEROPORTOS;
    g->quantidade = 0;
    g->aeroportos = (Aeroporto *)malloc(g->capacidade * sizeof(Aeroporto));
    if (!g->aeroportos) { free(g); return NULL; }

    g->matriz = criarMatriz(sizeof(Voo));
    if (!g->matriz) { free(g->aeroportos); free(g); return NULL; }

    return g;
}

/*
 * destruirGrafo — chamada pelo main() ao sair (opção 0)
 * Libera em ordem: primeiro a matriz esparsa (que libera todos os nós
 * internamente via destruirMatriz()), depois o vetor de aeroportos,
 * depois a própria struct Grafo.
 */
void destruirGrafo(Grafo *g) {
    if (!g) return;
    destruirMatriz(g->matriz);
    free(g->aeroportos);
    free(g);
}

/* ═══════════════════════════════════════════════════════════
 *  Vetor dinâmico de aeroportos
 * ═══════════════════════════════════════════════════════════ */

/*
 * expandirVetor — chamada por cadastrarAeroporto() quando o vetor está cheio
 * Dobra a capacidade usando realloc. Se realloc falhar, o vetor original
 * permanece intacto e retorna 0 (falha).
 */
static int expandirVetor(Grafo *g) {
    int novaCapacidade = g->capacidade * 2;
    Aeroporto *novo = (Aeroporto *)realloc(g->aeroportos,
                                           novaCapacidade * sizeof(Aeroporto));
    if (!novo) return 0;
    g->aeroportos = novo;
    g->capacidade = novaCapacidade;
    return 1;
}

/*
 * encolherVetor — seria chamada se houvesse uma operação de remover aeroporto
 * Reduz a capacidade do vetor pela metade quando a quantidade cai abaixo
 * de 1/4 da capacidade (evita desperdício de memória).
 * Só encolhe se a nova capacidade for suficiente para os aeroportos restantes
 * e maior que a capacidade inicial.
 */
int encolherVetor(Grafo *g) {
    int novaCapacidade = g->capacidade / 2;
    if (novaCapacidade < CAPACIDADE_INICIAL_AEROPORTOS) return 1;
    if (g->quantidade > novaCapacidade) return 1;
    Aeroporto *novo = (Aeroporto *)realloc(g->aeroportos,
                                           novaCapacidade * sizeof(Aeroporto));
    if (!novo) return 0;
    g->aeroportos = novo;
    g->capacidade = novaCapacidade;
    return 1;
}

/*
 * indiceAeroporto — chamada por cadastrarVoo(), removerVoo(),
 *                   listarVoos() e listarTrajetos()
 * Faz busca linear no vetor de aeroportos comparando o código IATA.
 * Retorna o índice (0, 1, 2...) ou -1 se não encontrar.
 * O índice retornado é usado como linha/coluna na matriz esparsa.
 */
int indiceAeroporto(Grafo *g, const char *codigo) {
    for (int i = 0; i < g->quantidade; i++)
        if (strcmp(g->aeroportos[i].codigo, codigo) == 0)
            return i;
    return -1;
}

/* ═══════════════════════════════════════════════════════════
 *  Operação 1 – Cadastrar aeroporto
 * ═══════════════════════════════════════════════════════════ */

/*
 * cadastrarAeroporto — operação 1 do menu
 *
 * Fluxo:
 *   1. indiceAeroporto() → verifica se o código já existe (duplicata)
 *   2. Se o vetor estiver cheio, chama expandirVetor() para dobrar a capacidade
 *   3. Copia código e cidade no próximo slot do vetor
 *   4. Incrementa g->quantidade
 *
 * Retorna 1 = sucesso, 0 = falha (duplicata ou sem memória)
 */
int cadastrarAeroporto(Grafo *g, const char *codigo, const char *cidade) {
    if (indiceAeroporto(g, codigo) >= 0) {
        printf("[FALHA] Aeroporto '%s' ja esta cadastrado.\n", codigo);
        return 0;
    }
    if (g->quantidade == g->capacidade && !expandirVetor(g)) {
        printf("[FALHA] Memoria insuficiente para novo aeroporto.\n");
        return 0;
    }

    Aeroporto *a = &g->aeroportos[g->quantidade];
    strncpy(a->codigo, codigo, MAX_CODIGO - 1);
    a->codigo[MAX_CODIGO - 1] = '\0';
    strncpy(a->cidade, cidade, MAX_CIDADE - 1);
    a->cidade[MAX_CIDADE - 1] = '\0';

    printf("[OK] Aeroporto %s (%s) cadastrado.\n", codigo, cidade);
    g->quantidade++;
    return 1;
}

/* ═══════════════════════════════════════════════════════════
 *  Operação 2 – Cadastrar voo
 * ═══════════════════════════════════════════════════════════ */

/*
 * cadastrarVoo — operação 2 do menu
 *
 * Fluxo:
 *   1. indiceAeroporto() para origem e destino → valida que existem
 *   2. Verifica que origem != destino e que o número é positivo
 *   3. buscarElemento() → verifica se já existe voo nessa rota
 *   4. inserirElemento() → insere o Voo na célula (orig, dest) da matriz
 *
 * Retorna 1 = sucesso, 0 = falha
 */
int cadastrarVoo(Grafo *g, int numeroVoo,
                 const char *codOrigem, const char *codDestino) {
    int orig = indiceAeroporto(g, codOrigem);
    int dest = indiceAeroporto(g, codDestino);

    if (orig < 0) {
        printf("[FALHA] Aeroporto de origem '%s' nao encontrado.\n", codOrigem);
        return 0;
    }
    if (dest < 0) {
        printf("[FALHA] Aeroporto de destino '%s' nao encontrado.\n", codDestino);
        return 0;
    }
    if (orig == dest) {
        printf("[FALHA] Origem e destino nao podem ser o mesmo aeroporto.\n");
        return 0;
    }
    if (numeroVoo <= 0) {
        printf("[FALHA] Numero de voo invalido.\n");
        return 0;
    }

    /* Verifica se já existe qualquer voo nessa rota */
    Voo *existente = (Voo *)buscarElemento(g->matriz, orig, dest);
    if (existente) {
        if (existente->numero == numeroVoo)
            printf("[FALHA] Voo %d ja existe nessa rota.\n", numeroVoo);
        else
            printf("[FALHA] Ja existe o voo %d nessa rota. "
                   "Remova-o antes de inserir outro.\n", existente->numero);
        return 0;
    }

    /* Tudo certo: insere o voo na matriz esparsa */
    Voo voo = { numeroVoo };
    int res = inserirElemento(g->matriz, orig, dest, &voo);
    if (res == 1) {
        printf("[OK] Voo %d cadastrado: %s (%s) --> %s (%s).\n",
               numeroVoo,
               g->aeroportos[orig].codigo, g->aeroportos[orig].cidade,
               g->aeroportos[dest].codigo, g->aeroportos[dest].cidade);
        return 1;
    }
    printf("[FALHA] Erro de memoria ao cadastrar voo.\n");
    return 0;
}

/* ═══════════════════════════════════════════════════════════
 *  Operação 3 – Remover voo (pelo número do voo)
 * ═══════════════════════════════════════════════════════════ */

/*
 * removerVoo — operação 3 do menu
 *
 * O enunciado pede remoção pelo número do voo apenas (sem precisar
 * informar origem e destino). Por isso, percorremos todas as linhas
 * da matriz procurando o voo com esse número.
 *
 * Fluxo:
 *   1. Para cada aeroporto i (linha da matriz):
 *        a. primeiroDaLinha() → percorre nós da linha i
 *        b. Compara o número do Voo de cada nó com numeroVoo
 *        c. Se achou: removerElemento(orig=i, dest=no->coluna)
 *           e retorna 1
 *   2. Se percorreu tudo e não achou: mensagem de falha, retorna 0
 *
 * Retorna 1 = removeu, 0 = não encontrado
 */
int removerVoo(Grafo *g, int numeroVoo) {
    for (int i = 0; i < g->quantidade; i++) {
        No *no = primeiroDaLinha(g->matriz, i);
        while (no) {
            Voo *voo = (Voo *)no->dado;
            if (voo->numero == numeroVoo) {
                int dest = no->coluna;
                /* guarda destino antes de remover (no será liberado) */
                removerElemento(g->matriz, i, dest);
                printf("[OK] Voo %d (%s -> %s) removido.\n",
                       numeroVoo,
                       g->aeroportos[i].codigo,
                       g->aeroportos[dest].codigo);
                return 1;
            }
            no = no->proxLinha;
        }
    }
    printf("[FALHA] Voo %d nao encontrado.\n", numeroVoo);
    return 0;
}

/* ═══════════════════════════════════════════════════════════
 *  Operação 4 – Listar voos de um aeroporto
 * ═══════════════════════════════════════════════════════════ */

/*
 * listarVoos — operação 4 do menu
 *
 * Fluxo:
 *   1. indiceAeroporto() → valida que o aeroporto existe
 *   2. primeiroDaLinha() → pega o primeiro nó da linha do aeroporto
 *   3. Percorre a lista (no->proxLinha) imprimindo número do voo e cidade destino
 *      (conforme enunciado: "número e nome da cidade destino")
 */
void listarVoos(Grafo *g, const char *codOrigem) {
    int orig = indiceAeroporto(g, codOrigem);
    if (orig < 0) {
        printf("[FALHA] Aeroporto '%s' nao encontrado.\n", codOrigem);
        return;
    }

    No *no = primeiroDaLinha(g->matriz, orig);
    if (!no) {
        printf("[INFO] Nenhum voo partindo de %s (%s).\n",
               codOrigem, g->aeroportos[orig].cidade);
        return;
    }

    printf("\n  Voos partindo de %s (%s):\n",
           codOrigem, g->aeroportos[orig].cidade);
    printf("  %-10s %-8s %s\n", "Voo", "Cod.", "Cidade destino");
    printf("  %-10s %-8s %s\n", "----------", "--------",
           "------------------------------");

    /* percorre a lista da linha — cada nó é um voo saindo daqui */
    while (no) {
        Voo *voo = (Voo *)no->dado;
        int  dest = no->coluna;
        printf("  %-10d %-8s %s\n",
               voo->numero,
               g->aeroportos[dest].codigo,
               g->aeroportos[dest].cidade);
        no = no->proxLinha;   /* vai para o próximo voo da linha */
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  Operação 5 – Listar trajetos (DFS)
 * ═══════════════════════════════════════════════════════════ */

/*
 * EstadoDFS — struct que agrupa o estado da busca em profundidade
 * Usamos struct em vez de variáveis globais para evitar problemas
 * se a função for chamada mais de uma vez.
 *
 * Campos:
 *   visitados[]  → marca quais aeroportos já foram visitados neste caminho
 *   caminho[]    → guarda a sequência de índices do trajeto atual
 *   profundidade → tamanho atual do caminho (índice do próximo slot)
 *   encontrou    → flag: ficará 1 se pelo menos um trajeto foi achado
 *   n            → g->quantidade no momento da busca (tamanho dos arrays)
 */
typedef struct {
    int *visitados;
    int *caminho;
    int  profundidade;
    int  encontrou;
    int  n;
} EstadoDFS;

/*
 * dfs — função recursiva; chamada por listarTrajetos()
 *
 * Fluxo:
 *   1. Se atual == destino: imprime o caminho acumulado em st->caminho[]
 *      e marca st->encontrou = 1, então retorna (caso base)
 *   2. Caso contrário: para cada vizinho (no->coluna) da linha "atual":
 *        a. Se o vizinho ainda não foi visitado neste caminho:
 *           - marca visitados[vizinho] = 1
 *           - adiciona vizinho ao caminho (caminho[profundidade++])
 *           - chama dfs() recursivamente para o vizinho  ← desce mais fundo
 *           - ao voltar da recursão: profundidade-- e visitados[vizinho] = 0
 *             (backtracking: desfaz a escolha para tentar outro caminho)
 */
static void dfs(Grafo *g, EstadoDFS *st, int atual, int destino) {
    /* caso base: chegamos ao destino → imprime o trajeto */
    if (atual == destino) {
        st->encontrou = 1;
        printf("  Trajeto: ");
        for (int i = 0; i < st->profundidade; i++) {
            printf("%s", g->aeroportos[st->caminho[i]].codigo);
            if (i < st->profundidade - 1) printf(" -> ");
        }
        printf("\n");
        return;
    }

    /* percorre os voos que saem do aeroporto "atual" */
    No *no = primeiroDaLinha(g->matriz, atual);
    while (no) {
        int vizinho = no->coluna;
        if (vizinho < st->n && !st->visitados[vizinho]) {
            st->visitados[vizinho]           = 1;
            st->caminho[st->profundidade++]  = vizinho;
            dfs(g, st, vizinho, destino);   /* ← chamada recursiva */
            st->profundidade--;             /* backtracking */
            st->visitados[vizinho]           = 0;
        }
        no = no->proxLinha;
    }
}

/*
 * listarTrajetos — operação 5 do menu
 *
 * Fluxo:
 *   1. indiceAeroporto() para origem e destino → valida existência
 *   2. Aloca visitados[] e caminho[] com tamanho g->quantidade
 *      (alocação dinâmica: evita overflow independente do número de aeroportos)
 *   3. Marca origem como visitada, insere no caminho
 *   4. Chama dfs() → imprime todos os trajetos encontrados
 *   5. Se nenhum trajeto: informa ao usuário
 *   6. Libera visitados[] e caminho[] antes de retornar
 */
void listarTrajetos(Grafo *g, const char *codOrigem, const char *codDestino) {
    int orig = indiceAeroporto(g, codOrigem);
    int dest = indiceAeroporto(g, codDestino);

    if (orig < 0) {
        printf("[FALHA] Aeroporto de origem '%s' nao encontrado.\n", codOrigem);
        return;
    }
    if (dest < 0) {
        printf("[FALHA] Aeroporto de destino '%s' nao encontrado.\n", codDestino);
        return;
    }
    if (orig == dest) {
        printf("[FALHA] Origem e destino sao o mesmo aeroporto.\n");
        return;
    }

    int n = g->quantidade;
    EstadoDFS st;
    st.n            = n;
    st.profundidade = 0;
    st.encontrou    = 0;
    st.visitados    = (int *)calloc(n, sizeof(int));  /* inicializa com zeros */
    st.caminho      = (int *)malloc(n * sizeof(int));
    if (!st.visitados || !st.caminho) {
        printf("[FALHA] Memoria insuficiente para busca de trajetos.\n");
        free(st.visitados);
        free(st.caminho);
        return;
    }

    /* começa o caminho pela origem */
    st.visitados[orig]              = 1;
    st.caminho[st.profundidade++]   = orig;

    printf("\n  Trajetos possiveis de %s (%s) ate %s (%s):\n",
           codOrigem, g->aeroportos[orig].cidade,
           codDestino, g->aeroportos[dest].cidade);

    dfs(g, &st, orig, dest);   /* ← dispara a busca em profundidade */

    if (!st.encontrou)
        printf("  [INFO] Nenhum trajeto encontrado.\n");
    printf("\n");

    /* libera memória alocada para a DFS */
    free(st.visitados);
    free(st.caminho);
}
