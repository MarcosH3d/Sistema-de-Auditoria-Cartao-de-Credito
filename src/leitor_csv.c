#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>
#include "leitor_csv.h"

#define NUM_CAMPOS      9
#define TAM_LINHA       512
#define CC_MIN_DIGITOS  12
#define CC_MAX_DIGITOS  19
#define CATEGORIA_MIN   1
#define CATEGORIA_MAX   14

const char *mensagemErro(ErroLinha erro) {
    switch (erro) {
        case ERRO_NENHUM:      return "ok";
        case ERRO_NUM_CAMPOS:  return "numero de campos diferente de 9";
        case ERRO_DATETIME:    return "trans_datetime invalido";
        case ERRO_CARTAO:      return "cc_num invalido (so digitos, 12 a 19)";
        case ERRO_CATEGORIA:   return "category fora de 1..14";
        case ERRO_VALOR:       return "amt invalido (nao numerico ou negativo)";
        case ERRO_GENERO:      return "gender deve ser F ou M";
        case ERRO_COORDENADA:  return "coordenada invalida";
        case ERRO_LINHA_LONGA: return "linha maior que o limite do buffer";
    }
    return "erro desconhecido";
}

/* ---------- conversoes estritas: o campo INTEIRO precisa ser numero ---------- */

static int lerInteiro(const char *s, long long *saida) {
    char *fim;
    if (*s == '\0' || isspace((unsigned char)*s)) return 0;
    errno = 0;
    long long v = strtoll(s, &fim, 10);
    if (errno != 0 || *fim != '\0') return 0;
    *saida = v;
    return 1;
}

static int lerReal(const char *s, double *saida) {
    char *fim;
    if (*s == '\0' || isspace((unsigned char)*s)) return 0;
    errno = 0;
    double v = strtod(s, &fim);
    if (errno != 0 || *fim != '\0' || !isfinite(v)) return 0; /* barra nan/inf */
    *saida = v;
    return 1;
}

int cartaoValido(const char *s) {
    size_t n = strlen(s);
    if (n < CC_MIN_DIGITOS || n > CC_MAX_DIGITOS) return 0;
    for (size_t i = 0; i < n; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

/* Separa a linha em campos preservando campos VAZIOS (strtok nao faz isso:
 * "a,,b" viraria 2 campos). Devolve a quantidade; se passar de 'max',
 * devolve max+1. */
static int dividirCampos(char *linha, char *campos[], int max) {
    int n = 0;
    campos[n++] = linha;
    for (char *p = linha; *p != '\0'; p++) {
        if (*p == ',') {
            if (n >= max) return max + 1;
            *p = '\0';
            campos[n++] = p + 1;
        }
    }
    return n;
}

static int coordenadaValida(const char *s, double limite, double *saida) {
    return lerReal(s, saida) && *saida >= -limite && *saida <= limite;
}

ErroLinha interpretarLinha(char *linha, Transacao *t) {
    char *c[NUM_CAMPOS];
    long long inteiro;
    double real;

    if (dividirCampos(linha, c, NUM_CAMPOS) != NUM_CAMPOS) return ERRO_NUM_CAMPOS;

    /* 0: trans_datetime */
    if (!lerInteiro(c[0], &inteiro) || inteiro <= 0) return ERRO_DATETIME;
    t->trans_datetime = inteiro;

    /* 1: cc_num */
    if (!cartaoValido(c[1])) return ERRO_CARTAO;
    strcpy(t->cc_num, c[1]);

    /* 2: category */
    if (!lerInteiro(c[2], &inteiro) ||
        inteiro < CATEGORIA_MIN || inteiro > CATEGORIA_MAX) return ERRO_CATEGORIA;
    t->category = (int)inteiro;

    /* 3: amt */
    if (!lerReal(c[3], &real) || real < 0) return ERRO_VALOR;
    t->amt = real;

    /* 4: gender */
    if (strlen(c[4]) != 1 || (c[4][0] != 'F' && c[4][0] != 'M')) return ERRO_GENERO;
    t->gender = c[4][0];

    /* 5..8: lat, long, merch_lat, merch_long */
    if (!coordenadaValida(c[5], 90.0,  &t->lat))       return ERRO_COORDENADA;
    if (!coordenadaValida(c[6], 180.0, &t->lon))       return ERRO_COORDENADA;
    if (!coordenadaValida(c[7], 90.0,  &t->merch_lat)) return ERRO_COORDENADA;
    if (!coordenadaValida(c[8], 180.0, &t->merch_lon)) return ERRO_COORDENADA;

    return ERRO_NENHUM;
}

/* descarta o resto de uma linha que nao coube no buffer */
static void descartarRestoDaLinha(FILE *f) {
    int ch;
    while ((ch = fgetc(f)) != EOF && ch != '\n') { }
}

int carregarCSV(const char *caminho, ListaTransacoes *lista, ResumoCarga *resumo) {
    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir \"%s\".\n", caminho);
        return 0;
    }

    char linha[TAM_LINHA], original[TAM_LINHA];
    int numLinha = 0;
    int ok = 1;

    resumo->linhasLidas = resumo->validas = resumo->invalidas = 0;

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        numLinha++;

        /* linha maior que o buffer: nao terminou em \n e ainda nao e o fim do arquivo */
        int longa = (strchr(linha, '\n') == NULL && !feof(arquivo));
        if (longa) descartarRestoDaLinha(arquivo);

        linha[strcspn(linha, "\r\n")] = '\0';   /* aceita \n (Unix) e \r\n (Windows) */

        if (linha[0] == '\0') continue;                              /* linha em branco */
        /* cabecalho: o PDF diz "trans_datetime", mas o arquivo real usa
           "trans_date_trans_time" -- o prefixo "trans_date" cobre os dois */
        if (numLinha == 1 && strncmp(linha, "trans_date", 10) == 0) continue;

        resumo->linhasLidas++;
        strcpy(original, linha);                /* copia p/ mensagem de erro */

        Transacao t;
        ErroLinha erro = longa ? ERRO_LINHA_LONGA : interpretarLinha(linha, &t);

        if (erro != ERRO_NENHUM) {
            resumo->invalidas++;
            fprintf(stderr, "[%s] linha %d ignorada: %s -> \"%.60s\"\n",
                    caminho, numLinha, mensagemErro(erro), original);
            continue;
        }

        if (!inserirFinal(lista, t)) {
            fprintf(stderr, "Erro: sem memoria na linha %d.\n", numLinha);
            ok = 0;
            break;
        }
        resumo->validas++;
    }

    fclose(arquivo);
    return ok;
}
