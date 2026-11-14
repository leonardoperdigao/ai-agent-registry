#ifndef ARQUIVO_H
#define ARQUIVO_H
#include "catalogo.h"
/* Carregar retorna 1 no sucesso (inclusive arquivo inexistente) ou 0 no erro.
   Em erro, nao usar nem salvar os vetores parcialmente carregados. */
int carregarDados(const char *caminho, int ids[], char nomes[][TAM_NAME],char scripts[][TAM_SCRIPT], float avaliacoes[], int *quantidade, int *proximoId);
/* Salvar retorna 1 se a gravacao e substituicao forem concluidas; 0 no erro. */
int salvarDados(const char *caminho, const int ids[], char nomes[][TAM_NAME],char scripts[][TAM_SCRIPT], const float avaliacoes[], int quantidade, int proximoId);
#endif
