/**
 * main_terminal.c - Versao em modo texto (nao precisa de nenhuma biblioteca).
 * Compilar: gcc main_terminal.c caca.c -o caca_terminal
 */
#include <stdio.h>
#include "caca.h"

static void mostrar(const Jogo *j)
{
    printf("\n    ");
    for (int c = 0; c < j->n; c++) printf("%2d ", c + 1);
    printf("\n");
    for (int l = 0; l < j->n; l++) {
        printf("%2d  ", l + 1);
        for (int c = 0; c < j->n; c++)
            printf(" %c%s", j->letras[l][c], j->marcada[l][c] ? "*" : " ");
        printf("\n");
    }
    printf("\n%s\n\nPalavras:\n", j->texto);
    for (const No *p = j->palavras.inicio; p; p = p->prox)
        printf("  [%c] %s\n", p->achada ? 'x' : ' ', p->palavra);
}

int main(void)
{
    Jogo jogo = {0};
    int nivel = 0;
    printf("Nivel (1 = Facil, 2 = Medio, 3 = Dificil): ");
    if (scanf("%d", &nivel) != 1 || nivel < 1 || nivel > 3) {
        printf("Nivel invalido.\n");
        return 1;
    }
    if (jogo_novo(&jogo, (Nivel)(nivel - 1)) != 0) {
        printf("Erro ao gerar o jogo.\n");
        return 1;
    }
    while (!jogo_terminou(&jogo)) {
        mostrar(&jogo);
        int l0, c0, l1, c1;
        printf("\nDigite linha e coluna do INICIO e do FIM (ou 0 0 0 0 para sair): ");
        if (scanf("%d %d %d %d", &l0, &c0, &l1, &c1) != 4) break;   /* entrada invalida ou EOF */
        if (l0 == 0 && c0 == 0 && l1 == 0 && c1 == 0) break;
        printf(jogo_tentar(&jogo, l0 - 1, c0 - 1, l1 - 1, c1 - 1)
               ? "Achou!\n" : "Nao e uma palavra da lista.\n");
    }
    if (jogo_terminou(&jogo)) { mostrar(&jogo); printf("\nParabens, voce achou todas!\n"); }
    jogo_liberar(&jogo);
    return 0;
}
