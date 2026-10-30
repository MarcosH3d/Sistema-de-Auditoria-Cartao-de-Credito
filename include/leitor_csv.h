#ifndef LEITOR_CSV_H
#define LEITOR_CSV_H

#include "transacao.h"

/* Motivos pelos quais uma linha do CSV e rejeitada */
typedef enum {
    ERRO_NENHUM = 0,
    ERRO_NUM_CAMPOS,     /* diferente de 9 campos         */
    ERRO_DATETIME,
    ERRO_CARTAO,
    ERRO_CATEGORIA,      /* fora de 1..14                 */
    ERRO_VALOR,          /* nao numerico ou negativo      */
    ERRO_GENERO,         /* diferente de F/M              */
    ERRO_COORDENADA,     /* nao numerica ou fora da faixa */
    ERRO_LINHA_LONGA     /* linha maior que o buffer      */
} ErroLinha;

typedef struct {
    int linhasLidas;   /* linhas de dados (sem cabecalho e sem linhas em branco) */
    int validas;       /* viraram Transacao e foram inseridas na lista            */
    int invalidas;     /* rejeitadas (motivo impresso em stderr)                  */
} ResumoCarga;

const char *mensagemErro(ErroLinha erro);

/* 1 se for so digitos e tiver de 12 a 19 digitos (reutilizavel p/ validar entrada do usuario) */
int cartaoValido(const char *s);

/* Interpreta UMA linha (sem \n). ATENCAO: modifica 'linha' (troca ',' por '\0').
 * Retorna ERRO_NENHUM e preenche *t se a linha for valida. */
ErroLinha interpretarLinha(char *linha, Transacao *t);

/* Le um CSV inteiro e ANEXA as transacoes validas ao final de 'lista'
 * (pode ser chamada varias vezes com arquivos diferentes = carga incremental).
 * Retorna 1 se processou o arquivo, 0 se nao abriu o arquivo ou faltou memoria. */
int carregarCSV(const char *caminho, ListaTransacoes *lista, ResumoCarga *resumo);

#endif
