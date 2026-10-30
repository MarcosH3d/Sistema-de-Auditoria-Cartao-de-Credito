#include <stdio.h>
#include <stdlib.h>
#include "transacao.h"

void iniciarLista(ListaTransacoes *lista) {
    lista->inicio  = NULL;
    lista->fim     = NULL;
    lista->tamanho = 0;
}

int inserirFinal(ListaTransacoes *lista, Transacao t) {
    NoTransacao *novo = malloc(sizeof(NoTransacao));
    if (novo == NULL) {
        return 0; /* sem memoria */
    }

    novo->dados   = t;
    novo->proximo = NULL;

    if (lista->inicio == NULL) {      /* lista vazia */
        lista->inicio = novo;
    } else {                          /* encadeia depois do ultimo */
        lista->fim->proximo = novo;
    }
    lista->fim = novo;
    lista->tamanho++;

    return 1;
}

void imprimirTransacao(const Transacao *t) {
    printf("%lld | cartao %s | cat %2d | R$ %9.2f | %c | casa(%.4f, %.4f) | loja(%.6f, %.6f)\n",
           t->trans_datetime, t->cc_num, t->category, t->amt, t->gender,
           t->lat, t->lon, t->merch_lat, t->merch_lon);
}

void imprimirLista(const ListaTransacoes *lista, int limite) {
    const NoTransacao *no = lista->inicio;
    int i = 0;

    while (no != NULL && (limite <= 0 || i < limite)) {
        printf("%3d: ", i + 1);
        imprimirTransacao(&no->dados);
        no = no->proximo;
        i++;
    }
    if (no != NULL) {
        printf("... (%d transacoes omitidas)\n", lista->tamanho - i);
    }
}

int contarNos(const ListaTransacoes *lista) {
    int n = 0;
    const NoTransacao *no = lista->inicio;
    while (no != NULL) {
        n++;
        no = no->proximo;
    }
    return n;
}

void liberarLista(ListaTransacoes *lista) {
    NoTransacao *no = lista->inicio;
    while (no != NULL) {
        NoTransacao *proximo = no->proximo; /* guarda antes do free */
        free(no);
        no = proximo;
    }
    lista->inicio  = NULL;
    lista->fim     = NULL;
    lista->tamanho = 0;
}
