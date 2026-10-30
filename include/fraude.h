#ifndef FRAUDE_H
#define FRAUDE_H

/* Regras de fraude A, B e C (requisito 1).  DONO: Integrante B (T04 a T07)
 * Regra C: distancia por haversine (raio da Terra ~6371 km), limite 78 km.
 * Atencao: "mesma fatura" precisa de uma definicao unica no grupo
 * (sugestao: mesmo ano+mes da transacao). Combinar com C (T11). */
double haversineKm(double lat1, double lon1, double lat2, double lon2);

/* TODO: regra A (<2min na mesma fatura), regra B (>=3x a media das DEMAIS
 * transacoes da fatura) e regra C (loja a mais de 78 km da residencia),
 * cada uma em funcao separada, e a combinacao em fraud_typeA/B/C (Y/N). */

#endif
