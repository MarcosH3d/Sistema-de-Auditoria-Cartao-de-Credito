#ifndef TRANSACAO_H
#define TRANSACAO_H

/* ------------------------------------------------------------------
 * Contrato de dados do projeto "O golpe ta ai!" (Integrante A)
 *
 * Uma Transacao representa UMA linha valida do CSV de entrada:
 *   trans_datetime,cc_num,category,amt,gender,lat,long,merch_lat,merch_long
 *
 * IMPORTANTE p/ a equipe:
 *  - cc_num e guardado como STRING (os cartoes do arquivo tem de 12 a 19
 *    digitos; 19 digitos pode estourar long long). Alem disso o nome do
 *    arquivo de cada cartao e o proprio cc_num, entao string ja serve.
 *  - trans_datetime e epoch em UTC. Para obter ano/mes/dia use gmtime(),
 *    NAO localtime() (senao o fuso horario muda o dia/mes).
 * ------------------------------------------------------------------ */

#define TAM_CC_NUM 21          /* ate 20 digitos + '\0' */

typedef struct {
    long long trans_datetime;  /* epoch time (segundos, UTC)      */
    char      cc_num[TAM_CC_NUM];
    int       category;        /* 1..14                           */
    double    amt;             /* valor da compra (>= 0)          */
    char      gender;          /* 'F' ou 'M'                      */
    double    lat, lon;        /* residencia do cliente           */
    double    merch_lat, merch_lon; /* loja                       */
} Transacao;

/* No da lista encadeada */
typedef struct NoTransacao {
    Transacao dados;
    struct NoTransacao *proximo;
} NoTransacao;

/* Lista com ponteiro para o ultimo no: insercao no final em O(1),
 * importante porque o numero de linhas dos CSVs e indeterminado. */
typedef struct {
    NoTransacao *inicio;
    NoTransacao *fim;
    int          tamanho;
} ListaTransacoes;

/* ---- API da lista (lista.c) ---- */
void iniciarLista(ListaTransacoes *lista);
int  inserirFinal(ListaTransacoes *lista, Transacao t);  /* 1 ok, 0 sem memoria */
void imprimirTransacao(const Transacao *t);
void imprimirLista(const ListaTransacoes *lista, int limite); /* limite<=0: tudo */
int  contarNos(const ListaTransacoes *lista);            /* percorre e conta    */
void liberarLista(ListaTransacoes *lista);

#endif
