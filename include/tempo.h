#ifndef TEMPO_H
#define TEMPO_H

/* Conversao de epoch para data.  DONO: Integrante B (T04); usado tambem por C
 * (colunas year,month,day do CSV por cartao) e por D.
 * Usar gmtime() (UTC), NAO localtime(): o fuso mudaria o dia/mes. */
void epochParaData(long long epoch, int *ano, int *mes, int *dia);

#endif
