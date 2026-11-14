#ifndef ENTRADA_H
#define ENTRADA_H

/* Leituras interativas: retornam 1 com valor valido ou 0 em fim/erro de entrada.
   O chamador fornece buffers e ponteiros validos; capacidade e medida em bytes. */
int lerTexto(const char *mensagem, char *destino, int capacidade);
/* limite restringe pontos de codigo UTF-8, independentemente dos bytes. */
int lerTextoLimitado(const char *mensagem, char *destino, int capacidade, int limite);
int lerInteiro(const char *mensagem, int minimo, int maximo, int *destino);
int lerNota(const char *mensagem, float *destino);
/* Valida uma string sem ler o teclado nem modificar seu conteudo. */
int textoValido(const char *texto, int limite);

#endif
