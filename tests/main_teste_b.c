#include <stdio.h>
#include <string.h>
#include <math.h>
#include "transacao.h"
#include "leitor_csv.h"
#include "tempo.h"
#include "fraude.h"

static int totalTestes = 0;
static int falhas      = 0;

#define CHECK(cond, descricao) do {                                   \
    totalTestes++;                                                    \
    if (cond) { printf("  [ok]    %s\n", descricao); }                \
    else      { printf("  [FALHA] %s\n", descricao); falhas++; }      \
} while (0)


static NoTransacao *add(ListaTransacoes *l, const char *cc, long long epoch, double amt) {
    Transacao t;
    memset(&t, 0, sizeof(t));
    t.trans_datetime = epoch;
    strcpy(t.cc_num, cc);
    t.category = 1;
    t.amt = amt;
    t.gender = 'F';
    t.lat = t.merch_lat = 10.0;       
    t.lon = t.merch_lon = 20.0;
    inserirFinal(l, t);
    return l->fim;
}

#define CARTAO     "1111222233334444"
#define OUTROCARTAO "5555666677778888"
#define T0         1546300818LL        


static void testeTempo(void) {
    int a, m, d;
    printf("\n[tempo] epochParaData / mesmaFatura\n");

    epochParaData(1546300799LL, &a, &m, &d);
    CHECK(a == 2018 && m == 12 && d == 31, "1546300799 = 31/12/2018 (ultimo segundo de 2018)");
    epochParaData(1546300800LL, &a, &m, &d);
    CHECK(a == 2019 && m == 1 && d == 1,   "1546300800 = 01/01/2019 (primeiro segundo de 2019)");
    epochParaData(1546300818LL, &a, &m, &d);
    CHECK(a == 2019 && m == 1 && d == 1,   "1546300818 (1a linha do enunciado) = 01/01/2019");
    epochParaData(1583020799LL, &a, &m, &d);
    CHECK(a == 2020 && m == 2 && d == 29,  "1583020799 = 29/02/2020 (ano bissexto)");
    epochParaData(1583020800LL, &a, &m, &d);
    CHECK(a == 2020 && m == 3 && d == 1,   "1583020800 = 01/03/2020");

    CHECK(mesmaFatura(1546300800LL, 1548979169LL) == 1, "01/01 00:00:00 e 31/01 23:59:29 -> mesma fatura");
    CHECK(mesmaFatura(1546300799LL, 1546300800LL) == 0, "31/12 23:59:59 e 01/01 00:00:00 -> faturas diferentes");
    CHECK(mesmaFatura(1546300800LL, 1577836800LL) == 0, "mesmo mes em anos diferentes (jan/2019 x jan/2020) -> diferentes");
}

static void testeHaversine(void) {
    printf("\n[haversine] distancias conhecidas (R = 6371 km)\n");

    CHECK(haversineKm(10, 20, 10, 20) == 0.0, "mesmo ponto = 0 km");
    CHECK(fabs(haversineKm(0, 0, 1, 0) - 111.1949) < 0.001, "1 grau de latitude = 111,19 km (R*pi/180)");
    CHECK(fabs(haversineKm(0, 0, 0, 90) - 10007.5434) < 0.001, "quarto do equador = pi*R/2 = 10007,54 km");
    CHECK(fabs(haversineKm(0, 0, 0, 180) - 20015.0868) < 0.001, "antipodas = pi*R = 20015,09 km");
    CHECK(fabs(haversineKm(0, 179.5, 0, -179.5) - 111.1949) < 0.001, "cruzando o meridiano 180 = 1 grau = 111,19 km");
    CHECK(fabs(haversineKm(48.8566, 2.3522, 51.5074, -0.1278) - 343.556) < 0.01, "Paris-Londres = 343,56 km");
    CHECK(fabs(haversineKm(10, 20, 30, 40) - haversineKm(30, 40, 10, 20)) < 1e-9, "simetria: d(P,Q) = d(Q,P)");
}

static void testeRegraC(void) {
    const double grau78 = 78.0 / RAIO_TERRA_KM * 180.0 / 3.14159265358979323846;
    Transacao t;
    memset(&t, 0, sizeof(t));
    t.lat = 10.0; t.lon = 20.0; t.merch_lon = 20.0;

    printf("\n[regra C] fronteira de 78 km (78 km = %.9f graus de latitude)\n", grau78);

    t.merch_lat = 10.0 + grau78 - 0.001;
    CHECK(regraC(&t) == 0, "77,9 km (0,001 grau antes do limite) -> N");
    t.merch_lat = 10.0 + grau78 + 0.001;
    CHECK(regraC(&t) == 1, "78,1 km (0,001 grau depois do limite) -> Y");
    t.merch_lat = 10.0;
    CHECK(regraC(&t) == 0, "loja na propria casa (0 km) -> N");
    t.merch_lat = 10.0 - grau78 - 0.001;
    CHECK(regraC(&t) == 1, "78,1 km para o SUL tambem -> Y (distancia, nao direcao)");
}

static void testeRegraA(void) {
    ListaTransacoes l; iniciarLista(&l);
    printf("\n[regra A] intervalo < 120 s na mesma fatura\n");

    NoTransacao *a1 = add(&l, CARTAO, T0, 10);
    NoTransacao *a2 = add(&l, CARTAO, T0 + 119, 10);
    CHECK(regraA(&l, a1) == 1 && regraA(&l, a2) == 1, "119 s -> as DUAS do par ficam Y");
    liberarLista(&l);

    NoTransacao *b1 = add(&l, CARTAO, T0, 10);
    NoTransacao *b2 = add(&l, CARTAO, T0 + 120, 10);
    CHECK(regraA(&l, b1) == 0 && regraA(&l, b2) == 0, "120 s exatos -> N (a regra e 'menor que' 2 min)");
    liberarLista(&l);

    NoTransacao *c1 = add(&l, CARTAO, T0, 10);
    NoTransacao *c2 = add(&l, CARTAO, T0 + 121, 10);
    CHECK(regraA(&l, c1) == 0 && regraA(&l, c2) == 0, "121 s -> N");
    liberarLista(&l);

    NoTransacao *d1 = add(&l, CARTAO, T0, 10);
    NoTransacao *d2 = add(&l, CARTAO, T0 + 5, 10);
    NoTransacao *d3 = add(&l, OUTROCARTAO, T0 + 1, 10);
    CHECK(regraA(&l, d1) == 1 && regraA(&l, d2) == 1, "par a 5 s no mesmo cartao -> Y");
    CHECK(regraA(&l, d3) == 0, "outro cartao a 1 s de distancia nao conta -> N");
    liberarLista(&l);

    NoTransacao *e1 = add(&l, CARTAO, T0, 10);
    NoTransacao *e2 = add(&l, CARTAO, T0, 10);
    CHECK(regraA(&l, e1) == 1 && regraA(&l, e2) == 1, "duas compras IDENTICAS (diferenca 0 s) -> as duas Y");
    liberarLista(&l);

    NoTransacao *f1 = add(&l, CARTAO, T0, 10);
    CHECK(regraA(&l, f1) == 0, "transacao sozinha nao e comparada com ela mesma -> N");
    liberarLista(&l);

    NoTransacao *g1 = add(&l, CARTAO, T0, 10);
    NoTransacao *g2 = add(&l, CARTAO, T0 + 100, 10);
    NoTransacao *g3 = add(&l, CARTAO, T0 + 250, 10);
    CHECK(regraA(&l, g1) == 1 && regraA(&l, g2) == 1 && regraA(&l, g3) == 0, "cadeia 0/100/250 s -> Y, Y, N");
    liberarLista(&l);

    NoTransacao *h2 = add(&l, CARTAO, T0 + 50, 10);
    NoTransacao *h1 = add(&l, CARTAO, T0, 10);
    CHECK(regraA(&l, h1) == 1 && regraA(&l, h2) == 1, "lista fora de ordem cronologica -> mesmo resultado");
    liberarLista(&l);

    NoTransacao *i1 = add(&l, CARTAO, 1548979170LL, 10);
    NoTransacao *i2 = add(&l, CARTAO, 1548979210LL, 10);
    CHECK(regraA(&l, i1) == 0 && regraA(&l, i2) == 0, "40 s entre 31/01 e 01/02 (faturas diferentes) -> N");
    liberarLista(&l);
}

static void testeRegraB(void) {
    ListaTransacoes l; iniciarLista(&l);
    printf("\n[regra B] valor >= 3 x media das DEMAIS da fatura\n");

    add(&l, CARTAO, T0, 10);
    add(&l, CARTAO, T0 + 10000, 20);
    NoTransacao *x = add(&l, CARTAO, T0 + 20000, 45.00);
    CHECK(regraB(&l, x) == 1, "45,00 com media das demais 15,00 (exatamente 3x) -> Y");
    liberarLista(&l);

    add(&l, CARTAO, T0, 10);
    add(&l, CARTAO, T0 + 10000, 20);
    x = add(&l, CARTAO, T0 + 20000, 44.99);
    CHECK(regraB(&l, x) == 0, "44,99 (um centavo abaixo de 3x) -> N");
    liberarLista(&l);

    add(&l, CARTAO, T0, 10);
    add(&l, CARTAO, T0 + 10000, 20);
    x = add(&l, CARTAO, T0 + 20000, 45.01);
    CHECK(regraB(&l, x) == 1, "45,01 (um centavo acima de 3x) -> Y");
    liberarLista(&l);

    add(&l, CARTAO, T0, 0.10);
    add(&l, CARTAO, T0 + 10000, 0.20);
    x = add(&l, CARTAO, T0 + 20000, 0.45);
    CHECK(regraB(&l, x) == 1, "fronteira com centavos (0,10 / 0,20 / 0,45) -> Y (sem erro de double)");
    liberarLista(&l);

    add(&l, CARTAO, T0, 10);
    add(&l, CARTAO, T0 + 10000, 10);
    x = add(&l, CARTAO, T0 + 20000, 30);
    CHECK(regraB(&l, x) == 1, "10,10,30: media das DEMAIS = 10, 30 = 3x -> Y (com a propria na media daria N)");
    liberarLista(&l);

    x = add(&l, CARTAO, T0, 500);
    CHECK(regraB(&l, x) == 0, "unica transacao da fatura (sem 'demais') -> N");
    liberarLista(&l);

    add(&l, CARTAO, 1548979210LL, 5);            
    add(&l, OUTROCARTAO, T0 + 1000, 5);          
    x = add(&l, CARTAO, T0, 40);                 
    CHECK(regraB(&l, x) == 0, "outros meses/cartoes nao entram na media -> N");
    liberarLista(&l);

    add(&l, CARTAO, T0, 0);
    x = add(&l, CARTAO, T0 + 10000, 0);
    CHECK(regraB(&l, x) == 0, "valor 0 com demais 0 -> N (0 nao e '3x maior')");
    liberarLista(&l);

    add(&l, CARTAO, T0, 0);
    x = add(&l, CARTAO, T0 + 10000, 1.00);
    CHECK(regraB(&l, x) == 1, "valor 1,00 com demais 0 (media 0) -> Y");
    liberarLista(&l);
}

static void testeCombinacao(void) {
    ListaTransacoes l; iniciarLista(&l);
    ResultadoFraude r;
    printf("\n[combinacao] classificarTransacao\n");

    NoTransacao *legit = add(&l, CARTAO, T0, 25.0);
    r = classificarTransacao(&l, legit);
    CHECK(r.tipoA == 'N' && r.tipoB == 'N' && r.tipoC == 'N' && !transacaoSuspeita(r), "compra legitima -> N N N");
    liberarLista(&l);

    add(&l, CARTAO, T0, 10.0);
    NoTransacao *golpe = add(&l, CARTAO, T0 + 30, 200.0);
    golpe->dados.merch_lat = golpe->dados.lat + 1.0;
    r = classificarTransacao(&l, golpe);
    CHECK(r.tipoA == 'Y' && r.tipoB == 'Y' && r.tipoC == 'Y' && transacaoSuspeita(r), "golpe classico -> Y Y Y");
    liberarLista(&l);

    NoTransacao *longe = add(&l, CARTAO, T0, 20.0);
    longe->dados.merch_lat = longe->dados.lat + 2.0;
    r = classificarTransacao(&l, longe);
    CHECK(r.tipoA == 'N' && r.tipoB == 'N' && r.tipoC == 'Y', "so loja distante -> N N Y");
    liberarLista(&l);
}

static void classificarArquivo(const char *caminho, int tudo) {
    ListaTransacoes lista; ResumoCarga resumo;
    int nA = 0, nB = 0, nC = 0, nSusp = 0, mostradas = 0;

    iniciarLista(&lista);
    printf("\n=== Arquivo real: %s ===\n", caminho);
    if (!carregarCSV(caminho, &lista, &resumo)) { liberarLista(&lista); return; }
    printf("Linhas: %d | validas: %d | invalidas: %d\n", resumo.linhasLidas, resumo.validas, resumo.invalidas);

    for (const NoTransacao *no = lista.inicio; no != NULL; no = no->proximo) {
        ResultadoFraude r = classificarTransacao(&lista, no);
        nA += r.tipoA == 'Y'; nB += r.tipoB == 'Y'; nC += r.tipoC == 'Y';
        if (transacaoSuspeita(r)) nSusp++;

        if (tudo || (transacaoSuspeita(r) && mostradas < 12)) {
            if (!tudo) mostradas++;
            printf("%lld,%s,%.2f,%c,%c,%c,dist=%.2fkm\n", no->dados.trans_datetime, no->dados.cc_num,
                   no->dados.amt, r.tipoA, r.tipoB, r.tipoC,
                   haversineKm(no->dados.lat, no->dados.lon, no->dados.merch_lat, no->dados.merch_lon));
        }
    }
    printf("RESUMO total=%d A=%d B=%d C=%d suspeitas=%d\n", lista.tamanho, nA, nB, nC, nSusp);
    liberarLista(&lista);
}

int main(int argc, char *argv[]) {
    printf("===== Testes de Pedu =====\n");
    testeTempo();
    testeHaversine();
    testeRegraC();
    testeRegraA();
    testeRegraB();
    testeCombinacao();

    printf("\n%d testes, %d falha(s).\n", totalTestes, falhas);

    if (argc >= 2) classificarArquivo(argv[1], argc >= 3 && strcmp(argv[2], "--tudo") == 0);

    return falhas == 0 ? 0 : 1;
}
