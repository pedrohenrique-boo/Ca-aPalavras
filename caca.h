/**
 * caca.h - Logica do jogo de caca-palavras (sem nenhuma parte grafica).
 *
 * Esta biblioteca cuida de: lista encadeada de palavras, geracao da grade,
 * colocacao das palavras conforme o nivel e verificacao das selecoes.
 * Assim a mesma logica serve para a versao grafica (raylib) e para a de terminal.
 */
#ifndef CACA_H
#define CACA_H

#define MAX_N   16   /* tamanho maximo da grade (nivel dificil)          */
#define MAX_PAL 24   /* tamanho maximo de uma palavra (incluindo o '\0') */

typedef enum { FACIL = 0, MEDIO = 1, DIFICIL = 2 } Nivel;

/* No da lista encadeada: guarda uma palavra e onde ela foi colocada. */
typedef struct No {
    char palavra[MAX_PAL];
    int  achada;            /* 0 = ainda nao achada, 1 = achada            */
    int  linha, coluna;     /* onde a palavra comeca na grade              */
    int  dl, dc;            /* direcao usada (ex.: dl=0, dc=1 = horizontal) */
    struct No *prox;        /* proximo no da lista                         */
} No;

/* Lista encadeada simples com ponteiro para o inicio e para o fim. */
typedef struct {
    No *inicio;
    No *fim;
    int tamanho;            /* quantas palavras existem na lista           */
    int achadas;            /* quantas ja foram encontradas                */
} Lista;

/* Estado completo de uma partida. */
typedef struct {
    int         n;                        /* lado da grade (n x n)         */
    char        letras[MAX_N][MAX_N];     /* letras da grade               */
    int         marcada[MAX_N][MAX_N];    /* 0 = livre; k>0 = achada na k-esima palavra */
    Lista       palavras;                 /* palavras a encontrar          */
    Nivel       nivel;
    const char *texto;                    /* texto que contem as palavras  */
} Jogo;

/* ---- Lista ---- */
void lista_iniciar(Lista *l);
int  lista_inserir(Lista *l, const char *palavra);          /* 0 = ok, -1 = erro */
No  *lista_buscar(const Lista *l, const char *palavra);     /* NULL se nao achar */
void lista_liberar(Lista *l);

/* ---- Jogo ---- */
int  jogo_novo(Jogo *j, Nivel nivel);   /* 0 = ok, -1 = erro. Chame jogo_liberar antes de repetir */
void jogo_liberar(Jogo *j);
int  jogo_tentar(Jogo *j, int l0, int c0, int l1, int c1);  /* 1 = achou uma palavra, 0 = nao */
int  jogo_terminou(const Jogo *j);
const char *jogo_nome_nivel(Nivel nivel);

#endif
