#ifndef FRAUDE_H
#define FRAUDE_H

#include "transacao.h"

#define LIMITE_INTERVALO_SEG   120     
#define FATOR_VALOR_ATIPICO    3       
#define LIMITE_DISTANCIA_KM    78.0    
#define RAIO_TERRA_KM          6371.0  

typedef struct {
    char tipoA;
    char tipoB;
    char tipoC;
} ResultadoFraude;

double haversineKm(double lat1, double lon1, double lat2, double lon2);

int regraA(const ListaTransacoes *lista, const NoTransacao *alvo);

int regraB(const ListaTransacoes *lista, const NoTransacao *alvo);

int regraC(const Transacao *t);

ResultadoFraude classificarTransacao(const ListaTransacoes *lista, const NoTransacao *alvo);

int transacaoSuspeita(ResultadoFraude r);

#endif
