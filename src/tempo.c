#include <time.h>
#include "tempo.h"

void epochParaData(long long epoch, int *ano, int *mes, int *dia) {
    time_t instante = (time_t)epoch;
    struct tm *dh = gmtime(&instante);

    if (dh == NULL) {
        *ano = *mes = *dia = 0;
        return;
    }
    *ano = dh->tm_year + 1900;
    *mes = dh->tm_mon + 1;
    *dia = dh->tm_mday;
}

int mesmaFatura(long long epochA, long long epochB) {
    int anoA, mesA, diaA, anoB, mesB, diaB;

    epochParaData(epochA, &anoA, &mesA, &diaA);
    epochParaData(epochB, &anoB, &mesB, &diaB);

    if (anoA == 0 || anoB == 0) return 0;
    return anoA == anoB && mesA == mesB;
}
