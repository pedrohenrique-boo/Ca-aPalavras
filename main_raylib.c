/**
 * main_raylib.c - Interface grafica do caca-palavras usando a biblioteca raylib (>= 4.0).
 *
 * Controles:
 *   Menu : clique em um nivel ou tecle 1, 2 ou 3
 *   Jogo : segure o botao esquerdo no inicio da palavra, arraste ate o fim e solte
 *          R = novo jogo no mesmo nivel | ESC = volta ao menu
 */
#include "raylib.h"
#include "caca.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define LARGURA   1020
#define ALTURA    700
#define CELULA    36      /* lado de cada quadradinho, em pixels */
#define GRADE_X   30      /* canto superior esquerdo da grade    */
#define GRADE_Y   90
#define TEXTO_X   (GRADE_X + MAX_N * CELULA + 40)
#define TEXTO_W   (LARGURA - TEXTO_X - 25)

typedef enum { TELA_MENU, TELA_JOGO } Tela;

/* Uma cor por palavra encontrada (repete se acabarem). */
static const Color PALETA[] = { SKYBLUE, LIME, PINK, GOLD, ORANGE, VIOLET, BEIGE, GREEN, MAROON, PURPLE };
#define QTD_CORES ((int)(sizeof PALETA / sizeof PALETA[0]))

/* Converte a posicao do mouse em (linha, coluna). Retorna 0 se estiver fora da grade. */
static int celula_do_mouse(const Jogo *j, Vector2 m, int *l, int *c)
{
    if (m.x < GRADE_X || m.y < GRADE_Y) return 0;
    *c = (int)(m.x - GRADE_X) / CELULA;
    *l = (int)(m.y - GRADE_Y) / CELULA;
    return *l < j->n && *c < j->n;
}

/* Diz se (l0,c0)->(l1,c1) forma reta/diagonal e devolve o passo (sl,sc) e o tamanho. */
static int alinhado(int l0, int c0, int l1, int c1, int *sl, int *sc, int *len)
{
    int dl = l1 - l0, dc = c1 - c0;
    int adl = dl < 0 ? -dl : dl, adc = dc < 0 ? -dc : dc;
    if (!(dl == 0 || dc == 0 || adl == adc)) return 0;
    *sl = (dl > 0) - (dl < 0);
    *sc = (dc > 0) - (dc < 0);
    *len = (adl > adc ? adl : adc) + 1;
    return 1;
}

static void desenhar_grade(const Jogo *j, int selecionando, int l0, int c0, int l1, int c1)
{
    /* Cor de fundo das celulas ja achadas */
    for (int l = 0; l < j->n; l++)
        for (int c = 0; c < j->n; c++) {
            Rectangle r = { GRADE_X + c * CELULA, GRADE_Y + l * CELULA, CELULA, CELULA };
            DrawRectangleRec(r, RAYWHITE);
            if (j->marcada[l][c])
                DrawRectangleRec(r, Fade(PALETA[(j->marcada[l][c] - 1) % QTD_CORES], 0.75f));
        }

    /* Pre-visualizacao da selecao em andamento */
    int sl, sc, len;
    if (selecionando && alinhado(l0, c0, l1, c1, &sl, &sc, &len))
        for (int i = 0; i < len; i++)
            DrawRectangle(GRADE_X + (c0 + sc * i) * CELULA, GRADE_Y + (l0 + sl * i) * CELULA,
                          CELULA, CELULA, Fade(YELLOW, 0.7f));

    /* Linhas e letras */
    for (int l = 0; l < j->n; l++)
        for (int c = 0; c < j->n; c++) {
            Rectangle r = { GRADE_X + c * CELULA, GRADE_Y + l * CELULA, CELULA, CELULA };
            DrawRectangleLinesEx(r, 1, LIGHTGRAY);
            char s[2] = { j->letras[l][c], '\0' };
            int w = MeasureText(s, 24);
            DrawText(s, (int)r.x + (CELULA - w) / 2, (int)r.y + 6, 24, DARKGRAY);
        }
    DrawRectangleLines(GRADE_X, GRADE_Y, j->n * CELULA, j->n * CELULA, DARKGRAY);
}

/* Desenha o texto quebrando linhas. Palavras da lista ficam laranja; achadas, verdes e riscadas. */
static void desenhar_texto(const Jogo *j, int x, int y, int largura)
{
    const int fs = 20;
    const int espaco = MeasureText(" ", fs);
    const char *p = j->texto;
    int cx = x, cy = y;

    while (*p) {
        while (*p == ' ') p++;
        char tok[64];
        int n = 0;
        while (*p && *p != ' ' && n < 63) tok[n++] = *p++;
        tok[n] = '\0';
        if (n == 0) break;

        /* tira pontuacao e poe em maiuscula para comparar com a lista */
        char limpo[64];
        int k = 0;
        for (int i = 0; i < n; i++)
            if (isalpha((unsigned char)tok[i])) limpo[k++] = (char)toupper((unsigned char)tok[i]);
        limpo[k] = '\0';
        No *no = lista_buscar(&j->palavras, limpo);

        Color cor = DARKGRAY;
        if (no) cor = no->achada ? DARKGREEN : (Color){ 210, 100, 0, 255 };

        int w = MeasureText(tok, fs);
        if (cx + w > x + largura) { cx = x; cy += fs + 8; }
        DrawText(tok, cx, cy, fs, cor);
        if (no && no->achada) DrawLine(cx, cy + fs / 2 + 2, cx + w, cy + fs / 2 + 2, cor);
        cx += w + espaco;
    }
}

static int botao(Rectangle r, const char *txt)
{
    int sobre = CheckCollisionPointRec(GetMousePosition(), r);
    DrawRectangleRec(r, sobre ? SKYBLUE : LIGHTGRAY);
    DrawRectangleLinesEx(r, 2, DARKGRAY);
    int w = MeasureText(txt, 28);
    DrawText(txt, (int)(r.x + (r.width - w) / 2), (int)(r.y + 16), 28, BLACK);
    return sobre && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

int main(void)
{
    InitWindow(LARGURA, ALTURA, "Caca-Palavras");
    SetExitKey(KEY_NULL);            /* ESC nao fecha a janela; usamos ESC para voltar ao menu */
    SetTargetFPS(60);

    Jogo jogo = {0};
    Tela tela = TELA_MENU;
    Nivel nivel = FACIL;
    int selecionando = 0, l0 = 0, c0 = 0, l1 = 0, c1 = 0;
    char msg[64] = "";

    while (!WindowShouldClose()) {
        int iniciar = -1;            /* nivel a iniciar neste quadro (-1 = nenhum) */

        /* ---------- ATUALIZACAO ---------- */
        if (tela == TELA_MENU) {
            if (IsKeyPressed(KEY_ONE))   iniciar = FACIL;
            if (IsKeyPressed(KEY_TWO))   iniciar = MEDIO;
            if (IsKeyPressed(KEY_THREE)) iniciar = DIFICIL;
        } else {
            if (IsKeyPressed(KEY_ESCAPE)) { tela = TELA_MENU; selecionando = 0; }
            if (IsKeyPressed(KEY_R)) iniciar = nivel;

            if (!jogo_terminou(&jogo)) {
                Vector2 m = GetMousePosition();
                int l, c;
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && celula_do_mouse(&jogo, m, &l, &c)) {
                    selecionando = 1; l0 = l1 = l; c0 = c1 = c;
                }
                if (selecionando && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && celula_do_mouse(&jogo, m, &l, &c)) {
                    l1 = l; c1 = c;     /* fora da grade: mantem a ultima celula valida */
                }
                if (selecionando && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                    selecionando = 0;
                    if (l0 != l1 || c0 != c1)
                        snprintf(msg, sizeof msg, "%s",
                                 jogo_tentar(&jogo, l0, c0, l1, c1)
                                 ? "Palavra encontrada!" : "Essa nao esta na lista.");
                }
            }
        }

        if (iniciar >= 0) {
            jogo_liberar(&jogo);                       /* evita vazamento de memoria */
            if (jogo_novo(&jogo, (Nivel)iniciar) == 0) {
                nivel = (Nivel)iniciar;
                tela = TELA_JOGO;
                selecionando = 0;
                msg[0] = '\0';
            } else {
                tela = TELA_MENU;
                snprintf(msg, sizeof msg, "Erro ao gerar o jogo. Tente de novo.");
            }
        }

        /* ---------- DESENHO ---------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (tela == TELA_MENU) {
            const char *titulo = "CACA-PALAVRAS";
            DrawText(titulo, (LARGURA - MeasureText(titulo, 56)) / 2, 110, 56, DARKBLUE);
            DrawText("Escolha o nivel", (LARGURA - MeasureText("Escolha o nivel", 28)) / 2, 200, 28, DARKGRAY);
            if (botao((Rectangle){ 360, 260, 300, 60 }, "1 - Facil"))   iniciar = FACIL;
            if (botao((Rectangle){ 360, 340, 300, 60 }, "2 - Medio"))   iniciar = MEDIO;
            if (botao((Rectangle){ 360, 420, 300, 60 }, "3 - Dificil")) iniciar = DIFICIL;
            DrawText("Facil: horizontal e vertical", 360, 510, 20, GRAY);
            DrawText("Medio: + diagonais", 360, 535, 20, GRAY);
            DrawText("Dificil: + palavras de tras para frente", 360, 560, 20, GRAY);
            if (msg[0]) DrawText(msg, 360, 610, 20, RED);
        } else {
            char topo[96];
            snprintf(topo, sizeof topo, "Nivel %s   |   Palavras: %d / %d",
                     jogo_nome_nivel(nivel), jogo.palavras.achadas, jogo.palavras.tamanho);
            DrawText(topo, GRADE_X, 25, 28, DARKBLUE);
            DrawText(msg, GRADE_X, 58, 20, MAROON);

            desenhar_grade(&jogo, selecionando, l0, c0, l1, c1);

            DrawText("Leia o texto e encontre as palavras em laranja:", TEXTO_X, GRADE_Y, 20, BLACK);
            desenhar_texto(&jogo, TEXTO_X, GRADE_Y + 40, TEXTO_W);
            DrawText("Arraste o mouse sobre a palavra.", TEXTO_X, ALTURA - 90, 18, GRAY);
            DrawText("R = novo jogo   |   ESC = menu", TEXTO_X, ALTURA - 65, 18, GRAY);

            if (jogo_terminou(&jogo)) {
                DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.55f));
                const char *t = "PARABENS! Voce achou todas!";
                DrawText(t, (LARGURA - MeasureText(t, 44)) / 2, 280, 44, WHITE);
                const char *d = "R = jogar de novo   |   ESC = menu";
                DrawText(d, (LARGURA - MeasureText(d, 24)) / 2, 350, 24, LIGHTGRAY);
            }
        }
        EndDrawing();

        if (iniciar >= 0 && tela == TELA_MENU) {       /* clique de botao do menu */
            jogo_liberar(&jogo);
            if (jogo_novo(&jogo, (Nivel)iniciar) == 0) {
                nivel = (Nivel)iniciar; tela = TELA_JOGO; selecionando = 0; msg[0] = '\0';
            } else {
                snprintf(msg, sizeof msg, "Erro ao gerar o jogo. Tente de novo.");
            }
        }
    }

    jogo_liberar(&jogo);
    CloseWindow();
    return 0;
}
