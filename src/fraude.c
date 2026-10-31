#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "fraude.h"
#include "tempo.h"

#define PI 3.14159265358979323846

static double grausParaRad(double graus) {
    return graus * PI / 180.0;
}

double haversineKm(double lat1, double lon1, double lat2, double lon2) {
    double fi1 = grausParaRad(lat1);
    double fi2 = grausParaRad(lat2);
    double dFi = grausParaRad(lat2 - lat1);
    double dLa = grausParaRad(lon2 - lon1);

    double s1 = sin(dFi / 2.0);
    double s2 = sin(dLa / 2.0);
    double a  = s1 * s1 + cos(fi1) * cos(fi2) * s2 * s2;

    if (a < 0.0) a = 0.0;
    if (a > 1.0) a = 1.0;

    return 2.0 * RAIO_TERRA_KM * atan2(sqrt(a), sqrt(1.0 - a));
}

static int ehOutraDaMesmaFatura(const NoTransacao *no, const NoTransacao *alvo) {
    return no != alvo
        && strcmp(no->dados.cc_num, alvo->dados.cc_num) == 0
        && mesmaFatura(no->dados.trans_datetime, alvo->dados.trans_datetime);
}

static long long emCentavos(double valor) {
    return llround(valor * 100.0);
}

int regraA(const ListaTransacoes *lista, const NoTransacao *alvo) {
    for (const NoTransacao *no = lista->inicio; no != NULL; no = no->proximo) {
        if (!ehOutraDaMesmaFatura(no, alvo)) continue;

        long long diferenca = no->dados.trans_datetime - alvo->dados.trans_datetime;
        if (llabs(diferenca) < LIMITE_INTERVALO_SEG) return 1;
    }
    return 0;
}

int regraB(const ListaTransacoes *lista, const NoTransacao *alvo) {
    long long somaOutras = 0;
    long long qtdOutras  = 0;

    for (const NoTransacao *no = lista->inicio; no != NULL; no = no->proximo) {
        if (!ehOutraDaMesmaFatura(no, alvo)) continue;
        somaOutras += emCentavos(no->dados.amt);
        qtdOutras++;
    }

    if (qtdOutras == 0) return 0;

    long long valor = emCentavos(alvo->dados.amt);
    if (valor <= 0) return 0;

    return valor * qtdOutras >= FATOR_VALOR_ATIPICO * somaOutras;
}

int regraC(const Transacao *t) {
    return haversineKm(t->lat, t->lon, t->merch_lat, t->merch_lon) > LIMITE_DISTANCIA_KM;
}

static char simNao(int condicao) {
    return condicao ? 'Y' : 'N';
}

ResultadoFraude classificarTransacao(const ListaTransacoes *lista, const NoTransacao *alvo) {
    ResultadoFraude r;
    r.tipoA = simNao(regraA(lista, alvo));
    r.tipoB = simNao(regraB(lista, alvo));
    r.tipoC = simNao(regraC(&alvo->dados));
    return r;
}

int transacaoSuspeita(ResultadoFraude r) {
    return r.tipoA == 'Y' || r.tipoB == 'Y' || r.tipoC == 'Y';
}
