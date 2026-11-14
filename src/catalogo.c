#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "catalogo.h"
#include "entrada.h"

/* Recebe os vetores paralelos e os contadores por endereco.
   Retorna 1 somente ao concluir o cadastro; 0 indica limite ou interrupcao. */
int cadastrarAgente(int ids[], char nomes[][TAM_NAME], char scripts[][TAM_SCRIPT],
                    float avaliacoes[], int *quantidade, int *proximoId)
{
    if (*quantidade >= MAX_AGENTS) {
        puts("Catalogo cheio.");
        return 0;
    }
    /* Impede estouro ao incrementar o contador de identificadores. */
    if (*proximoId == INT_MAX) {
        puts("Limite de identificadores atingido.");
        return 0;
    }
    /* Le em variaveis temporarias: falhas nao confirmam registros parciais. */
    char nome[TAM_NAME], script[TAM_SCRIPT];
    float nota;
    if (!lerTextoLimitado("Nome do agente: ", nome, sizeof nome, LIM_NAME) ||
        !lerTextoLimitado("Instrucoes do agente (uma linha): ", script, sizeof script, LIM_SCRIPT) ||
        !lerNota("Nota de 0 a 5 (use ponto): ", &nota)) {
        puts("Cadastro cancelado: leitura interrompida.");
        return 0;
    }
    /* Todos os campos foram validados; confirma na proxima posicao livre. */
    int posicao = *quantidade;
    ids[posicao] = *proximoId;
    strcpy(nomes[posicao], nome);
    strcpy(scripts[posicao], script);
    avaliacoes[posicao] = nota;
    ++(*quantidade);
    ++(*proximoId);
    printf("Agente cadastrado. ID: %d\n", ids[posicao]);
    return 1;
}

/* Mostra um resumo dos registros ativos, sem modificar os vetores. */
void listarAgentes(const int ids[], char nomes[][TAM_NAME],
                   const float avaliacoes[], int quantidade)
{
    if (!quantidade) { puts("Nenhum agente cadastrado."); return; }
    printf("\nTotal: %d agente(s)\n", quantidade);
    for (int i = 0; i < quantidade; ++i)
        printf("ID: %d | Nome: %s | Nota: %.2f\n", ids[i], nomes[i], avaliacoes[i]);
}

/* Busca sequencial: devolve o indice do ID ou -1 quando nao o encontra. */
int buscarId(const int ids[], int quantidade, int id)
{
    for (int i = 0; i < quantidade; ++i)
        if (ids[i] == id) return i;
    return -1;
}

/* Busca parcial sensivel a maiusculas e acentos. Nomes podem se repetir.
   Retorna o indice escolhido entre os resultados, ou -1 para voltar/falhar. */
int buscarNome(const int ids[], char nomes[][TAM_NAME], int quantidade)
{
    char termo[TAM_NAME];
    int encontrados = 0, escolhido;
    if (!lerTextoLimitado("Nome ou parte do nome: ", termo, sizeof termo, LIM_NAME))
        return -1;
    for (int i = 0; i < quantidade; ++i) {
        if (strstr(nomes[i], termo)) {
            printf("ID: %d | Nome: %s\n", ids[i], nomes[i]);
            ++encontrados;
        }
    }
    if (!encontrados) { puts("Nenhum agente encontrado."); return -1; }
    while (lerInteiro("ID do resultado para abrir (0 volta): ", 0, INT_MAX, &escolhido)) {
        if (!escolhido) return -1;
        int posicao = buscarId(ids, quantidade, escolhido);
        if (posicao >= 0 && strstr(nomes[posicao], termo)) return posicao;
        puts("Escolha um ID apresentado nos resultados.");
    }
    return -1;
}

/* Recebe um indice valido obtido na busca e oferece detalhes/edicao/exclusao.
   Retorna 1 quando houve alteracao, para a main solicitar o salvamento. */
int gerenciarAgente(int posicao, int ids[], char nomes[][TAM_NAME],
                    char scripts[][TAM_SCRIPT], float avaliacoes[], int *quantidade)
{
    int opcao, confirmar;
    printf("\nID: %d\nNome: %s\nInstrucoes: %s\nNota: %.2f\n",
           ids[posicao], nomes[posicao], scripts[posicao], avaliacoes[posicao]);
    if (!lerInteiro("1 - Editar | 2 - Excluir | 0 - Voltar: ", 0, 2, &opcao) || !opcao)
        return 0;
    if (opcao == 1) {
        /* Preserva o registro antigo ate todas as novas entradas serem validas. */
        char nome[TAM_NAME], script[TAM_SCRIPT];
        float nota;
        puts("Informe os novos dados. O ID sera mantido.");
        if (!lerTextoLimitado("Novo nome: ", nome, sizeof nome, LIM_NAME) ||
            !lerTextoLimitado("Novas instrucoes: ", script, sizeof script, LIM_SCRIPT) ||
            !lerNota("Nova nota (0 a 5): ", &nota)) {
            puts("Edicao cancelada. Dados anteriores preservados.");
            return 0;
        }
        strcpy(nomes[posicao], nome);
        strcpy(scripts[posicao], script);
        avaliacoes[posicao] = nota;
        puts("Agente atualizado.");
        return 1;
    }
    if (!lerInteiro("Confirmar exclusao? 1 - Sim | 0 - Nao: ", 0, 1, &confirmar) ||
        !confirmar) { puts("Exclusao cancelada."); return 0; }
    /* Desloca TODOS os vetores juntos, sem alterar os IDs dos sobreviventes.
       proximoId nao diminui, portanto uma exclusao nao permite reutilizar IDs. */
    for (int i = posicao; i < *quantidade - 1; ++i) {
        ids[i] = ids[i + 1];
        strcpy(nomes[i], nomes[i + 1]);
        strcpy(scripts[i], scripts[i + 1]);
        avaliacoes[i] = avaliacoes[i + 1];
    }
    --(*quantidade);
    puts("Agente excluido.");
    return 1;
}
