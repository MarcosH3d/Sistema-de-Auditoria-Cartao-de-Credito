CC     = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC    = $(wildcard src/*.c)
HDR    = $(wildcard include/*.h)

golpe: $(SRC) $(HDR)
	$(CC) $(CFLAGS) -o golpe $(SRC)

# teste isolado do Integrante A (leitura + lista)
teste_a: tests/main_teste_a.c src/lista.c src/leitor_csv.c $(HDR)
	$(CC) $(CFLAGS) -o teste_a tests/main_teste_a.c src/lista.c src/leitor_csv.c

clean:
	rm -f golpe teste_a golpe.exe teste_a.exe

.PHONY: clean
