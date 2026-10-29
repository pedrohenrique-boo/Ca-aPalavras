/**
 * caca.c - Implementacao da logica do caca-palavras. Veja caca.h.
 */
#include "caca.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Todas as 8 direcoes possiveis {delta_linha, delta_coluna}.
 * A ORDEM IMPORTA: o nivel usa so os primeiros N itens.
 *   0..1 : horizontal e vertical           (FACIL)
 *   2..3 : diagonais                       (MEDIO)
 *   4..7 : as mesmas direcoes, invertidas  (DIFICIL) */
static const int DIRECOES[8][2] = {
    { 0,  1}, { 1,  0},
    { 1,  1}, {-1,  1},
    { 0, -1}, {-1,  0}, {-1, -1}, { 1, -1}
};
static const int QTD_DIRECOES[3] = { 2, 4, 8 };
static const int TAM_GRADE[3]    = { 12, 14, 16 };
static const char *NOMES[3]      = { "Facil", "Medio", "Dificil" };

/* Textos SEM acento: a fonte padrao da raylib so desenha ASCII com seguranca. */
static const char *TEXTOS[3] = {
    "O computador processa dados usando um programa. A memoria guarda "
    "informacoes e o teclado envia comandos para a tela mostrar o resultado.",

    "Todo sistema operacional controla processos e gerencia a memoria. "
    "O kernel conversa com o hardware enquanto o escalonador decide qual "
    "tarefa usa o processador.",

    "Em banco de dados, uma tabela guarda registros ligados por chaves. "
    "A consulta usa SQL para filtrar, ordenar e juntar informacoes, "
    "e o indice acelera a busca."
};

/* Palavras de cada nivel (maiusculas, sem acento, terminadas em NULL).
 * Cada uma delas aparece no texto do nivel correspondente. */
static const char *PALAVRAS[3][12] = {
    { "COMPUTADOR", "PROGRAMA", "MEMORIA", "TECLADO", "COMANDOS",
      "TELA", "DADOS", "RESULTADO", NULL },
    { "SISTEMA", "OPERACIONAL", "PROCESSOS", "KERNEL", "HARDWARE",
      "ESCALONADOR", "TAREFA", "PROCESSADOR", NULL },
    { "BANCO", "TABELA", "REGISTROS", "CHAVES", "CONSULTA", "INDICE",
      "INFORMACOES", "ACELERA", "FILTRAR", "ORDENAR", NULL }
};

/* ===================== LISTA ENCADEADA ===================== */

void lista_iniciar(Lista *l)
{
    l->inicio = l->fim = NULL;
    l->tamanho = l->achadas = 0;
}

/* Insere no FIM da lista (O(1) porque guardamos o ponteiro 'fim'). */
int lista_inserir(Lista *l, const char *palavra)
{
    if (strlen(palavra) >= MAX_PAL) return -1;        /* evita estourar o vetor */
    No *novo = calloc(1, sizeof(No));                 /* calloc zera tudo (prox = NULL) */
    if (!novo) return -1;                             /* sem memoria */
    strcpy(novo->palavra, palavra);                   /* seguro: tamanho ja checado */
    if (l->fim) l->fim->prox = novo; else l->inicio = novo;
    l->fim = novo;
    l->tamanho++;
    return 0;
}

No *lista_buscar(const Lista *l, const char *palavra)
{
    for (No *p = l->inicio; p; p = p->prox)
        if (strcmp(p->palavra, palavra) == 0) return p;
    return NULL;
}

/* Guarda o 'prox' ANTES de dar free, senao perdemos o resto da lista. */
void lista_liberar(Lista *l)
{
    No *p = l->inicio;
    while (p) {
        No *prox = p->prox;
        free(p);
        p = prox;
    }
    lista_iniciar(l);
}

/* ===================== GRADE ===================== */

/* Verifica se a palavra cabe a partir de (l,c) na direcao (dl,dc).
 * Aceita celula vazia (0) ou com a MESMA letra (cruzamento de palavras). */
static int cabe(const Jogo *j, const char *pal, int l, int c, int dl, int dc)
{
    int len = (int)strlen(pal);
    int lf = l + dl * (len - 1), cf = c + dc * (len - 1);
    if (lf < 0 || lf >= j->n || cf < 0 || cf >= j->n) return 0;   /* sai da grade */
    for (int i = 0; i < len; i++) {
        char atual = j->letras[l + dl * i][c + dc * i];
        if (atual != 0 && atual != pal[i]) return 0;               /* conflito */
    }
    return 1;
}

/* Sorteia posicao e direcao ate caber (maximo 500 tentativas). */
static int colocar(Jogo *j, No *no, int ndir)
{
    int len = (int)strlen(no->palavra);
    for (int t = 0; t < 500; t++) {
        int d  = rand() % ndir;
        int dl = DIRECOES[d][0], dc = DIRECOES[d][1];
        int l  = rand() % j->n,  c  = rand() % j->n;
        if (!cabe(j, no->palavra, l, c, dl, dc)) continue;
        for (int i = 0; i < len; i++)
            j->letras[l + dl * i][c + dc * i] = no->palavra[i];
        no->linha = l; no->coluna = c; no->dl = dl; no->dc = dc;
        return 1;
    }
    return 0;
}

int jogo_novo(Jogo *j, Nivel nivel)
{
    static int semeado = 0;
    if (!semeado) { srand((unsigned)time(NULL)); semeado = 1; }

    memset(j, 0, sizeof *j);
    j->nivel = nivel;
    j->n     = TAM_GRADE[nivel];
    j->texto = TEXTOS[nivel];
    lista_iniciar(&j->palavras);

    /* 1) Carrega as palavras do nivel na lista. */
    for (int i = 0; PALAVRAS[nivel][i]; i++) {
        if ((int)strlen(PALAVRAS[nivel][i]) > j->n ||
            lista_inserir(&j->palavras, PALAVRAS[nivel][i]) < 0) {
            lista_liberar(&j->palavras);
            return -1;
        }
    }

    /* 2) Tenta posicionar todas; se alguma falhar, limpa a grade e recomeca. */
    int ok = 0;
    for (int tent = 0; tent < 100 && !ok; tent++) {
        memset(j->letras, 0, sizeof j->letras);
        ok = 1;
        for (No *no = j->palavras.inicio; no; no = no->prox)
            if (!colocar(j, no, QTD_DIRECOES[nivel])) { ok = 0; break; }
    }
    if (!ok) { lista_liberar(&j->palavras); return -1; }

    /* 3) Preenche o resto com letras aleatorias. */
    for (int l = 0; l < j->n; l++)
        for (int c = 0; c < j->n; c++)
            if (j->letras[l][c] == 0) j->letras[l][c] = (char)('A' + rand() % 26);
    return 0;
}

void jogo_liberar(Jogo *j)
{
    lista_liberar(&j->palavras);
}

/* Confere a selecao de (l0,c0) ate (l1,c1). Aceita o sentido normal ou inverso. */
int jogo_tentar(Jogo *j, int l0, int c0, int l1, int c1)
{
    if (l0 < 0 || c0 < 0 || l1 < 0 || c1 < 0 ||
        l0 >= j->n || c0 >= j->n || l1 >= j->n || c1 >= j->n) return 0;

    int dl = l1 - l0, dc = c1 - c0;
    int adl = abs(dl), adc = abs(dc);
    if (!(dl == 0 || dc == 0 || adl == adc)) return 0;   /* nao e reta nem diagonal */

    int len = (adl > adc ? adl : adc) + 1;
    if (len < 2) return 0;                               /* clique sem arrastar */
    int sl = (dl > 0) - (dl < 0), sc = (dc > 0) - (dc < 0);   /* sinais: -1, 0 ou 1 */

    char frente[MAX_N + 1], tras[MAX_N + 1];
    for (int i = 0; i < len; i++) frente[i] = j->letras[l0 + sl * i][c0 + sc * i];
    frente[len] = '\0';
    for (int i = 0; i < len; i++) tras[i] = frente[len - 1 - i];
    tras[len] = '\0';

    for (No *no = j->palavras.inicio; no; no = no->prox) {
        if (no->achada) continue;
        if (strcmp(frente, no->palavra) == 0 || strcmp(tras, no->palavra) == 0) {
            no->achada = 1;
            j->palavras.achadas++;
            for (int i = 0; i < len; i++)
                j->marcada[l0 + sl * i][c0 + sc * i] = j->palavras.achadas;
            return 1;
        }
    }
    return 0;
}

int jogo_terminou(const Jogo *j)
{
    return j->palavras.tamanho > 0 && j->palavras.achadas == j->palavras.tamanho;
}

const char *jogo_nome_nivel(Nivel nivel) { return NOMES[nivel]; }
