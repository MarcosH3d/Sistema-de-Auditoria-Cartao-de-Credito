#ifndef PERSISTENCIA_H
#define PERSISTENCIA_H
#include "transacao.h"

/* Requisitos 1 e 2: CSV por cartao ("<cc_num>.csv"), carga incremental e ordem
 * cronologica.  DONO: Integrante C (T08, T09, T10, T23; compartilha T11 com B).
 *
 * Retorna 1 em sucesso, 0 em erro.
 *
 * Atencao: o CSV por cartao NAO tem lat/long; a regra C e a reclassificacao de
 * faturas antigas precisam desses dados. Decidir onde guardar os dados brutos
 * (dica: gravar no mesmo formato do CSV de entrada, que o leitor_csv.c do
 * Integrante A ja sabe interpretar). */
int persistirCartoes(const ListaTransacoes *lista, const char *pastaSaida);

#endif
