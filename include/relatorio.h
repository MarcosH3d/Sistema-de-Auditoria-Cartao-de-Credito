#ifndef RELATORIO_H
#define RELATORIO_H

/* Requisito 3: perfil de compras por categoria.  DONO: Integrante D
 *
 * Abre "<pastaSaida>/<cc_num>.csv" (gerado por C), soma compras e valor das
 * 14 categorias e imprime a tabela.  Retorna 1 em sucesso, 0 se o cartao nao
 * existe (arquivo ausente) ou houve erro de leitura. */
int imprimirPerfilCliente(const char *pastaSaida, const char *cc_num);

#endif
