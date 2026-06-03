#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "aeroporto.h"

/*
 * ═══════════════════════════════════════════════════════════
 *  ARQUIVO: main.c — Ponto de entrada e interface com o usuário
 *
 *  Este arquivo contém:
 *    - Funções auxiliares de leitura (lerString, lerInteiro)
 *    - Handlers de cada opção do menu (op_*)
 *    - O menu principal (imprimirMenu)
 *    - main(): inicializa o grafo, roda o loop do menu, finaliza
 *
 *  FLUXO GERAL DO PROGRAMA:
 *
 *   main()
 *     │
 *     ├─ criarGrafo()          ← cria a estrutura do grafo (aeroporto.c)
 *     │
 *     └─ loop do menu
 *           │
 *           ├─ opcao 1 → op_cadastrarAeroporto() → cadastrarAeroporto()
 *           ├─ opcao 2 → op_cadastrarVoo()       → cadastrarVoo()
 *           ├─ opcao 3 → op_removerVoo()         → removerVoo()
 *           ├─ opcao 4 → op_listarVoos()         → listarVoos()
 *           ├─ opcao 5 → op_listarTrajetos()     → listarTrajetos()
 *           └─ opcao 0 → sai do loop → destruirGrafo() → fim
 *
 *  As funções op_* só lêem os dados do usuário e repassam para
 *  as funções de negócio em aeroporto.c.
 * ═══════════════════════════════════════════════════════════
 */

/* ═══════════════════════════════════════════════════════════
 *  Funções auxiliares de leitura
 * ═══════════════════════════════════════════════════════════ */

/*
 * lerString — lê uma string do teclado com exibição de prompt
 * Usa fgets para leitura segura (sem estouro de buffer).
 * Remove o '\n' que fgets deixa no final.
 * Se ocorrer EOF ou erro, dest fica como string vazia.
 * Chamada por todas as funções op_* que precisam de código ou cidade.
 */
static void lerString(const char *prompt, char *dest, int tam) {
    printf("%s", prompt);
    if (fgets(dest, tam, stdin)) {
        dest[strcspn(dest, "\n")] = '\0';
    } else {
        dest[0] = '\0';
    }
}

/*
 * lerInteiro — lê um número inteiro do teclado com exibição de prompt
 * Usa fgets + sscanf para evitar problemas com o buffer de stdin.
 * Retorna -1 se a entrada não for um número válido.
 * Chamada por op_cadastrarVoo() e op_removerVoo().
 */
static int lerInteiro(const char *prompt) {
    int v;
    char buf[32];
    printf("%s", prompt);
    if (fgets(buf, sizeof(buf), stdin))
        if (sscanf(buf, "%d", &v) == 1) return v;
    return -1;
}

/*
 * maiusculas — converte a string s para maiúsculas in-place
 * Chamada depois de ler o código IATA, pois os códigos são sempre
 * em maiúsculas (ex: "gru" → "GRU").
 */
static void maiusculas(char *s) {
    for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

/* ═══════════════════════════════════════════════════════════
 *  Handlers do menu (funções op_*)
 *  Cada uma lê os dados necessários e chama a função em aeroporto.c
 * ═══════════════════════════════════════════════════════════ */

/*
 * op_cadastrarAeroporto — handler da opção 1
 * Lê código IATA e nome da cidade, converte código para maiúsculas,
 * valida que nenhum campo está vazio e chama cadastrarAeroporto().
 * Depois de cadastrarAeroporto(), o aeroporto está no vetor do grafo.
 */
static void op_cadastrarAeroporto(Grafo *g) {
    char codigo[MAX_CODIGO], cidade[MAX_CIDADE];
    lerString("  Codigo IATA (ex: GRU): ", codigo, MAX_CODIGO);
    maiusculas(codigo);
    lerString("  Nome da cidade       : ", cidade, MAX_CIDADE);
    if (strlen(codigo) == 0 || strlen(cidade) == 0) {
        printf("[FALHA] Campos nao podem ser vazios.\n");
        return;
    }
    /* vai para cadastrarAeroporto() em aeroporto.c */
    cadastrarAeroporto(g, codigo, cidade);
}

/*
 * op_cadastrarVoo — handler da opção 2
 * Lê código de origem, código de destino e número do voo.
 * Converte os códigos para maiúsculas e chama cadastrarVoo().
 * Dentro de cadastrarVoo() os aeroportos são localizados por índice
 * e o voo é inserido na matriz esparsa.
 */
static void op_cadastrarVoo(Grafo *g) {
    char orig[MAX_CODIGO], dest[MAX_CODIGO];
    int  num;

    lerString("  Codigo origem  : ", orig, MAX_CODIGO); maiusculas(orig);
    lerString("  Codigo destino : ", dest, MAX_CODIGO); maiusculas(dest);
    num = lerInteiro("  Numero do voo  : ");

    /* vai para cadastrarVoo() em aeroporto.c */
    cadastrarVoo(g, num, orig, dest);
}

/*
 * op_removerVoo — handler da opção 3
 * Lê apenas o número do voo (o enunciado pede remoção pelo número).
 * Chama removerVoo() que percorre a matriz inteira procurando o voo.
 */
static void op_removerVoo(Grafo *g) {
    int num = lerInteiro("  Numero do voo a remover: ");
    /* vai para removerVoo() em aeroporto.c, que busca o voo na matriz */
    removerVoo(g, num);
}

/*
 * op_listarVoos — handler da opção 4
 * Lê o código do aeroporto de origem e chama listarVoos().
 * listarVoos() percorre a linha do aeroporto na matriz esparsa
 * e imprime número do voo + cidade destino de cada aresta.
 */
static void op_listarVoos(Grafo *g) {
    char codigo[MAX_CODIGO];
    lerString("  Codigo do aeroporto de origem: ", codigo, MAX_CODIGO);
    maiusculas(codigo);
    /* vai para listarVoos() em aeroporto.c */
    listarVoos(g, codigo);
}

/*
 * op_listarTrajetos — handler da opção 5
 * Lê o código de origem e de destino e chama listarTrajetos().
 * listarTrajetos() executa uma DFS e imprime todos os caminhos
 * possíveis entre os dois aeroportos (diretos e com escalas).
 */
static void op_listarTrajetos(Grafo *g) {
    char orig[MAX_CODIGO], dest[MAX_CODIGO];
    lerString("  Codigo de origem : ", orig, MAX_CODIGO); maiusculas(orig);
    lerString("  Codigo de destino: ", dest, MAX_CODIGO); maiusculas(dest);
    /* vai para listarTrajetos() em aeroporto.c */
    listarTrajetos(g, orig, dest);
}

/* ═══════════════════════════════════════════════════════════
 *  Menu principal
 * ═══════════════════════════════════════════════════════════ */

/*
 * imprimirMenu — exibe as 5 operações do enunciado + opção de saída
 * Chamada no início de cada iteração do loop em main().
 */
static void imprimirMenu(void) {
    printf("+--------------------------------------------------+\n");
    printf("|        SISTEMA DE MALHA AEREA - ANAC             |\n");
    printf("+--------------------------------------------------+\n");
    printf("| 1. Cadastrar aeroporto                           |\n");
    printf("| 2. Cadastrar voo                                 |\n");
    printf("| 3. Remover voo                                   |\n");
    printf("| 4. Listar voos de um aeroporto                   |\n");
    printf("| 5. Listar trajetos entre dois aeroportos         |\n");
    printf("| 0. Sair                                          |\n");
    printf("+--------------------------------------------------+\n");
    printf("  Opcao: ");
}

/* ═══════════════════════════════════════════════════════════
 *  Função principal
 * ═══════════════════════════════════════════════════════════ */

/*
 * main — ponto de entrada do programa
 *
 * Fluxo completo:
 *   1. criarGrafo()     → aloca toda a estrutura de dados
 *   2. loop do menu:
 *        - imprimirMenu() exibe as opções
 *        - lê a opção com fgets + sscanf
 *        - switch direciona para a função op_* correspondente
 *        - cada op_* lê dados do usuário e chama aeroporto.c
 *   3. ao sair (opcao 0):
 *        - destruirGrafo() libera toda a memória alocada
 *   4. retorna EXIT_SUCCESS
 */
int main(void) {
    /* passo 1: inicializa a estrutura do grafo */
    Grafo *g = criarGrafo();
    if (!g) {
        fprintf(stderr, "Erro fatal: nao foi possivel alocar o grafo.\n");
        return EXIT_FAILURE;
    }

    /*
     * Pré-carrega os 5 aeroportos iniciais conforme o enunciado.
     * O sistema já começa com eles cadastrados — o usuário pode
     * adicionar mais aeroportos e cadastrar os voos pelo menu.
     */
    printf("  [SISTEMA] Carregando aeroportos iniciais...\n");
    cadastrarAeroporto(g, "BSB", "Brasilia");
    cadastrarAeroporto(g, "CNF", "Belo Horizonte");
    cadastrarAeroporto(g, "GIG", "Rio de Janeiro");
    cadastrarAeroporto(g, "GRU", "Sao Paulo");
    cadastrarAeroporto(g, "SSA", "Salvador");
    printf("  [SISTEMA] Pronto!\n\n");

    int  opcao;
    char buf[8];

    /* passo 2: loop principal do menu */
    do {
        imprimirMenu();
        if (!fgets(buf, sizeof(buf), stdin)) break;
        if (sscanf(buf, "%d", &opcao) != 1) { opcao = -1; }

        printf("\n");
        switch (opcao) {
            case 1: op_cadastrarAeroporto(g); break;  /* → cadastrarAeroporto() */
            case 2: op_cadastrarVoo(g);       break;  /* → cadastrarVoo()       */
            case 3: op_removerVoo(g);         break;  /* → removerVoo()         */
            case 4: op_listarVoos(g);         break;  /* → listarVoos()         */
            case 5: op_listarTrajetos(g);     break;  /* → listarTrajetos()+DFS */
            case 0: printf("  Encerrando sistema. Ate logo!\n\n"); break;
            default:
                printf("  [AVISO] Opcao invalida. Tente novamente.\n\n");
        }
    } while (opcao != 0);

    /* passo 3: libera toda a memória antes de encerrar */
    destruirGrafo(g);
    return EXIT_SUCCESS;
}
