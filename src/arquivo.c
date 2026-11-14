#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include "arquivo.h"
#include "entrada.h"

/* Funcao interna: exige uma linha completa terminada em Enter no arquivo.
   Retorna 0 para linha truncada, excessiva, fim de arquivo ou erro. */
static int lerLinha(FILE *arquivo, char *linha, int tamanho)
{
    if (!fgets(linha, tamanho, arquivo)) return 0;
    char *quebra = strchr(linha, '\n');
    if (!quebra) return 0;
    *quebra = '\0';
    return 1;
}

/* Le inteiro nao negativo sem estouro. resto aponta para o separador seguinte,
   permitindo ao chamador verificar a estrutura da linha. */
static int inteiroArquivo(const char *texto, const char **resto, int *valor)
{
    char *fim;
    errno = 0;
    long numero = strtol(texto, &fim, 10);
    if (texto == fim || errno || numero < 0 || numero > INT_MAX) return 0;
    *valor = (int)numero;
    *resto = fim;
    return 1;
}

/* Formato: "PIM2 1", linha com quantidade/proximo ID e tres linhas por agente:
   ID/nota, nome e instrucoes. Arquivo inexistente inicia um catalogo vazio.
   Retorna 1 no sucesso; em erro retorna 0 e o chamador deve encerrar sem salvar.
   Os vetores podem ter sido parcialmente preenchidos antes de detectar erro. */
int carregarDados(const char *caminho, int ids[], char nomes[][TAM_NAME],
                  char scripts[][TAM_SCRIPT], float avaliacoes[], int *quantidade, int *proximoId)
{
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) {
        if (errno == ENOENT) { *quantidade = 0; *proximoId = 1; return 1; }
        perror("Nao foi possivel abrir os dados");
        return 0;
    }
    char linha[TAM_SCRIPT + 2];
    int total, proximo, ok = 0;
    const char *resto;
    if (!lerLinha(arquivo, linha, sizeof linha) || strcmp(linha, "PIM2 1")) goto fim;
    if (!lerLinha(arquivo, linha, sizeof linha) ||
        !inteiroArquivo(linha, &resto, &total) || *resto != ' ' ||
        !inteiroArquivo(resto + 1, &resto, &proximo) || *resto ||
        total > MAX_AGENTS || proximo < 1) goto fim;
    /* Valida limites, notas, textos e unicidade antes de confirmar contadores. */
    for (int i = 0; i < total; ++i) {
        if (!lerLinha(arquivo, linha, sizeof linha) ||
            !inteiroArquivo(linha, &resto, &ids[i]) || *resto != ' ' ||
            ids[i] < 1 || ids[i] >= proximo) goto fim;
        char *final;
        const char *inicio = resto + 1;
        errno = 0;
        avaliacoes[i] = strtof(inicio, &final);
        if (inicio == final || *final || errno || !isfinite(avaliacoes[i]) ||
            avaliacoes[i] < 0 || avaliacoes[i] > 5) goto fim;
        for (int j = 0; j < i; ++j) if (ids[i] == ids[j]) goto fim;
        if (!lerLinha(arquivo, linha, sizeof linha) || !textoValido(linha, LIM_NAME))
            goto fim;
        strcpy(nomes[i], linha);
        if (!lerLinha(arquivo, linha, sizeof linha) || !textoValido(linha, LIM_SCRIPT))
            goto fim;
        strcpy(scripts[i], linha);
    }
    /* Conteudo extra apos os registros tambem indica formato invalido. */
    if (fgetc(arquivo) != EOF || ferror(arquivo)) goto fim;
    *quantidade = total;
    *proximoId = proximo;
    ok = 1;
/* Centraliza o fechamento do arquivo nos caminhos de sucesso e falha. */
fim:
    fclose(arquivo);
    if (!ok) fputs("Arquivo de dados invalido. Ele nao sera sobrescrito.\n", stderr);
    return ok;
}

/* Grava vetores e contadores em arquivo temporario no mesmo diretorio.
   Retorna 1 apos substituir o arquivo anterior; 0 sinaliza falha.
   Requer registros validos e uso de apenas uma instancia por arquivo. */
int salvarDados(const char *caminho, const int ids[], char nomes[][TAM_NAME],
                char scripts[][TAM_SCRIPT], const float avaliacoes[], int quantidade, int proximoId)
{
    /* Reserva o caminho original, o sufixo ".tmp" e o terminador. */
    size_t tamanho = strlen(caminho) + 5;
    char *temporario = malloc(tamanho);
    if (!temporario) return 0;
    snprintf(temporario, tamanho, "%s.tmp", caminho);
    FILE *arquivo = fopen(temporario, "w");
    if (!arquivo) { perror("Falha ao salvar"); free(temporario); return 0; }
    int ok = fprintf(arquivo, "PIM2 1\n%d %d\n", quantidade, proximoId) >= 0;
    for (int i = 0; ok && i < quantidade; ++i)
        ok = fprintf(arquivo, "%d %.9g\n%s\n%s\n",
                     ids[i], (double)avaliacoes[i], nomes[i], scripts[i]) >= 0;
    /* fclose tambem pode detectar falha ao descarregar os dados no arquivo. */
    if (fclose(arquivo) != 0) ok = 0;
    
    /* Nao substitui os dados anteriores se a escrita/fechamento falhou.
       A troca nao equivale a backup nem garante durabilidade contra queda de energia. */
    if (ok && rename(temporario, caminho) != 0) ok = 0;
    if (!ok) {
        perror("Falha ao concluir salvamento");
        remove(temporario);
    }
    free(temporario);
    return ok;
}
