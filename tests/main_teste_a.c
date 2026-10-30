/* main_teste_a.c - demonstracao do Integrante A
 * Uso: ./teste_a arquivo1.csv [arquivo2.csv ...]
 * Cada arquivo e ANEXADO na mesma lista (simula a carga incremental). */
#include <stdio.h>
#include "transacao.h"
#include "leitor_csv.h"

int main(int argc, char *argv[]) {
    ListaTransacoes lista;
    ResumoCarga resumo;

    if (argc < 2) {
        printf("Uso: %s arquivo1.csv [arquivo2.csv ...]\n", argv[0]);
        return 1;
    }

    iniciarLista(&lista);

    for (int i = 1; i < argc; i++) {
        printf("\n=== Carregando %s ===\n", argv[i]);
        if (carregarCSV(argv[i], &lista, &resumo)) {
            printf("Linhas de dados: %d | validas: %d | invalidas: %d\n",
                   resumo.linhasLidas, resumo.validas, resumo.invalidas);
        }
    }

    printf("\n=== Percorrendo a lista (primeiras 10) ===\n");
    imprimirLista(&lista, 10);

    int contados = contarNos(&lista);
    printf("\nTamanho registrado: %d | nos percorridos: %d | %s\n",
           lista.tamanho, contados,
           lista.tamanho == contados ? "CONSISTENTE" : "INCONSISTENTE!");

    liberarLista(&lista);
    printf("Memoria liberada (tamanho agora: %d).\n", lista.tamanho);
    return 0;
}
