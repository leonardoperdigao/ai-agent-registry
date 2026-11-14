#ifndef CATALOGO_H
#define CATALOGO_H

/* Limites de registros e de caracteres definidos para o catalogo. */
#define MAX_AGENTS 50
#define LIM_NAME 80
#define LIM_SCRIPT 1000
/* Reserva ate quatro bytes UTF-8 por ponto de codigo, mais o terminador. */
#define TAM_NAME (LIM_NAME * 4 + 1)
#define TAM_SCRIPT (LIM_SCRIPT * 4 + 1)

/* Vetores paralelos compartilham o indice de cada agente.
   Cadastro/gerenciamento retornam 1 se alteraram dados, 0 caso contrario. */
int cadastrarAgente(int ids[], char nomes[][TAM_NAME], char scripts[][TAM_SCRIPT],float avaliacoes[], int *quantidade, int *proximoId);
void listarAgentes(const int ids[], char nomes[][TAM_NAME],const float avaliacoes[], int quantidade);
/* As buscas retornam posicoes nos vetores; -1 indica ausencia ou cancelamento. */
int buscarId(const int ids[], int quantidade, int id);
int buscarNome(const int ids[], char nomes[][TAM_NAME], int quantidade);
int gerenciarAgente(int posicao, int ids[], char nomes[][TAM_NAME],char scripts[][TAM_SCRIPT], float avaliacoes[], int *quantidade);

#endif
