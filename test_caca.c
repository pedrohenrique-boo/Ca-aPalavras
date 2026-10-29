/* Teste automatico: gera 200 jogos de cada nivel e confere se todas as palavras
 * podem ser achadas e se os niveis respeitam suas direcoes. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "caca.h"

int main(void)
{
    for (int nv = 0; nv < 3; nv++) {
        for (int rodada = 0; rodada < 200; rodada++) {
            Jogo j = {0};
            if (jogo_novo(&j, (Nivel)nv) != 0) { printf("FALHA ao gerar nivel %d\n", nv); return 1; }
            for (No *p = j.palavras.inicio; p; p = p->prox) {
                int len = (int)strlen(p->palavra);
                if (nv == 0 && !(p->dl == 0 || p->dc == 0) ) { printf("FALHA direcao facil\n"); return 1; }
                if (nv == 0 && (p->dl < 0 || p->dc < 0))      { printf("FALHA facil invertida\n"); return 1; }
                if (nv == 1 && (p->dc < 0 || (p->dl == 0 && p->dc < 0))) { printf("FALHA medio invertida\n"); return 1; }
                int l1 = p->linha + p->dl * (len - 1), c1 = p->coluna + p->dc * (len - 1);
                if (!jogo_tentar(&j, p->linha, p->coluna, l1, c1)) { printf("FALHA nao achou %s\n", p->palavra); return 1; }
            }
            if (!jogo_terminou(&j)) { printf("FALHA terminou\n"); return 1; }
            jogo_liberar(&j);
        }
        printf("Nivel %d OK\n", nv + 1);
    }
    return 0;
}
