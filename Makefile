CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS = -Iinclude
FONTES = src/main.c src/entrada.c src/catalogo.c src/arquivo.c

.PHONY: all run test
all: programa

programa: $(FONTES) $(wildcard include/*.h)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(FONTES) -o $@

run: programa
	./programa

test: programa
	python3 tests/test_fluxos.py ./programa
