#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transacao.h"
#include "leitor_csv.h"
#include "persistencia.h"
#include "relatorio.h"

#define TAM_ENTRADA 256
#define PASTA_SAIDA "saida"

/* Le uma linha do teclado sem \n. Retorna 0 em EOF. */
static int lerTexto(const char *prompt, char *buf, int tam) {
    printf("%s", prompt);
    if (fgets(buf, tam, stdin) == NULL) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 1;
}

/* Repete ate o usuario digitar um inteiro em [min, max]. -1 em EOF. */
static int lerOpcao(int min, int max) {
    char buf[TAM_ENTRADA], *fim;
    while (lerTexto("Escolha: ", buf, sizeof(buf))) {
        long v = strtol(buf, &fim, 10);
        if (buf[0] != '\0' && *fim == '\0' && v >= min && v <= max) return (int)v;
        printf("Opcao invalida. Digite um numero de %d a %d.\n", min, max);
    }
    return -1;
}

static void menuCarregar(ListaTransacoes *lista) {
    char caminho[TAM_ENTRADA];
    ResumoCarga resumo;

    if (!lerTexto("Caminho do CSV: ", caminho, sizeof(caminho)) || caminho[0] == '\0') {
        printf("Caminho vazio.\n");
        return;
    }
    if (carregarCSV(caminho, lista, &resumo)) {
        printf("Linhas: %d | validas: %d | invalidas: %d | total em memoria: %d\n",
               resumo.linhasLidas, resumo.validas, resumo.invalidas, lista->tamanho);
    }
}

static void menuPerfil(void) {
    char cc[TAM_ENTRADA];

    if (!lerTexto("Numero do cartao: ", cc, sizeof(cc))) return;
    if (!cartaoValido(cc)) {
        printf("Numero de cartao invalido (somente digitos, 12 a 19).\n");
        return;
    }
    if (!imprimirPerfilCliente(PASTA_SAIDA, cc)) {
        printf("Nao foi possivel gerar o relatorio para o cartao %s.\n", cc);
    }
}

int main(void) {
    ListaTransacoes lista;
    int opcao;

    iniciarLista(&lista);

    do {
        printf("\n===== O GOLPE TA AI! =====\n");
        printf("1 - Carregar arquivo CSV\n");
        printf("2 - Gerar CSVs por cartao\n");
        printf("3 - Relatorio de perfil de um cartao\n");
        printf("0 - Sair\n");

        opcao = lerOpcao(0, 3);
        switch (opcao) {
            case 1: menuCarregar(&lista); break;
            case 2: persistirCartoes(&lista, PASTA_SAIDA); break;
            case 3: menuPerfil(); break;
            default: break;
        }
    } while (opcao > 0);

    liberarLista(&lista);
    return 0;
}
