# Sistema de Auditoria de Cartão de Crédito — "O golpe tá aí!"

Exercício Escolar 2 · Programação 1 · Sistemas de Informação · UPE Caruaru  
Prof. Raphael Dourado · Entrega: 11/11/2026

Sistema em C que carrega transações de cartão de crédito a partir de CSVs, sinaliza
suspeitas de fraude, gera um CSV por cartão e emite relatório de perfil de compras.

## Equipe e responsabilidades

| Integrante | Responsável por | Tarefas | Arquivos |
|---|---|---|---|
| A — (Marcos) | Leitura dos CSVs, validação de campos e listas dinâmicas (base do sistema); apoia a integração com a persistência | T01, T02, T03 | `src/leitor_csv.c`, `src/lista.c`, `include/transacao.h`, `include/leitor_csv.h` |
| B — (Pedro) | Regras de fraude A, B e C, conversão de timestamp, haversine; reclassificação após novas cargas (com C) | T04, T05, T06, T07 (+T11 com C) | `src/fraude.c`, `src/tempo.c`, `include/fraude.h`, `include/tempo.h` |
| C — (Laura) | CSV por cartão, carga incremental, ordem cronológica, dados brutos para reclassificação | T08, T09, T10, T23 (+T11 com B) | `src/persistencia.c`, `include/persistencia.h` |
| D — (Ycaro) | Relatório de perfil por categoria, validação de entradas, testes de aceitação do grupo | requisito 3 | `src/relatorio.c`, `include/relatorio.h`, `src/main.c` |

Requisito 4 (modularização, sem globais, sem duplicação de código) é de todos.

### Como os módulos se conectam

```
CSV de entrada -> [A] leitor_csv + lista -> lista de Transacao
                                              |
                  [B] fraude/tempo  <-------->  [C] persistencia  -> saida/<cc_num>.csv
                                                                          |
                                                      [D] relatorio  <----+
```

D lê o CSV do cartão gerado por C; por isso o formato desse arquivo
(`year,month,day,trans_datetime,category,amt,fraud_typeA,fraud_typeB,fraud_typeC`)
é o contrato entre C e D.

### Decisões pendentes (fechar no grupo)

- **Definição de "fatura"**: sugestão = mesmo ano+mês da transação (o extra pede fatura por mês/ano). Afeta as regras A e B (B), a reclassificação (B e C) e o PDF (D).
- **Dados brutos para reclassificar**: o CSV por cartão não guarda latitude/longitude. C precisa decidir onde manter esses dados.

## Estrutura

```
include/   cabeçalhos (.h)
src/       código (.c); main.c = menu principal
data/      CSVs de exemplo
tests/     testes isolados de cada módulo e CSVs de teste
saida/     CSVs por cartão gerados pelo programa (não versionado)
```

## Compilar e executar

```bash
make            # gera ./golpe
./golpe

make teste_a    # teste isolado de leitura + lista
./teste_a data/1_jan2019.csv
```

Sem `make` (ex.: Code::Blocks): adicione todos os `src/*.c` ao projeto e configure
`include` como diretório de cabeçalhos (*Project → Build options → Search directories*).

## Contrato de dados

`include/transacao.h` é compartilhado por todos. **Não altere a `struct Transacao` sozinho**:
avise o grupo, abra um PR só para essa mudança e todos façam `git pull` em seguida.

- `cc_num` é string (cartões de até 19 dígitos).
- `trans_datetime` é epoch em UTC: use `gmtime()`, não `localtime()`.
- Os arquivos reais usam o cabeçalho `trans_date_trans_time` (o enunciado diz `trans_datetime`); o leitor aceita os dois.

## Fluxo de trabalho (Git)

1. Ninguém commita direto na `main`. Cada requisito tem sua branch: `feature/<assunto>`.
2. Antes de começar: `git checkout main && git pull`.
3. Commits pequenos, mensagem clara (`Adiciona regra B de fraude`).
4. Abra Pull Request e peça revisão de pelo menos 1 colega antes do merge.
5. Cada um é dono dos seus arquivos; mudou arquivo de outro? Avise antes.

## Regras do enunciado (lembrete)

- Listas dinâmicas obrigatórias; **sem vetores nem ordenação** durante carga/processamento.
- Vetores só na geração de relatórios, sem desperdício de memória.
- Entradas do teclado validadas; sem variáveis globais.
