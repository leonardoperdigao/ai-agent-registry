#include <stdio.h>
#include <limits.h>
#include "entrada.h"
#include "catalogo.h"
#include "arquivo.h"

/* Coordena o menu e a persistencia. Retorna 0 ao encerrar normalmente
   e 1 quando a abertura ou o salvamento final dos dados falha. */
int main(int argc, char *argv[])
{
    /* Um caminho opcional permite testes sem tocar nos registros de uso real. */
    const char *caminho = argc == 2 ? argv[1] : "dados/agentes.txt";
    if (argc > 2) { fprintf(stderr, "Uso: %s [arquivo-de-dados]\n", argv[0]); return 1; }
    /* Os dados de um agente ocupam o mesmo indice nos quatro vetores.
       Somente as posicoes abaixo de quantidade representam registros ativos. */
    int ids[MAX_AGENTS], quantidade = 0, proximoId = 1;
    char nomes[MAX_AGENTS][TAM_NAME], scripts[MAX_AGENTS][TAM_SCRIPT];
    float avaliacoes[MAX_AGENTS];
    /* pendente permanece ativo se alguma alteracao ainda nao foi salva. */
    int opcao, pendente = 0;
    if (!carregarDados(caminho, ids, nomes, scripts, avaliacoes, &quantidade, &proximoId))
        return 1;

    for (;;) {
        puts("\n=== Catalogo de Agentes ===\n"
             "1 - Cadastrar agente\n2 - Listar agentes\n"
             "3 - Buscar por ID\n4 - Buscar por nome\n0 - Encerrar");
        if (!lerInteiro("Escolha uma opcao: ", 0, 4, &opcao) || opcao == 0) break;
        /* As buscas devolvem a posicao, nao o ID; -1 significa sem selecao. */
        int mudou = 0, posicao = -1, id;
        switch (opcao) {
        case 1:
            mudou = cadastrarAgente(ids, nomes, scripts, avaliacoes, &quantidade, &proximoId);
            break;
        case 2:
            listarAgentes(ids, nomes, avaliacoes, quantidade);
            break;
        case 3:
            if (!lerInteiro("ID do agente: ", 1, INT_MAX, &id)) break;
            posicao = buscarId(ids, quantidade, id);
            if (posicao < 0) puts("Agente nao encontrado.");
            break;
        case 4:
            if (!quantidade) puts("Nenhum agente cadastrado.");
            else posicao = buscarNome(ids, nomes, quantidade);
            break;
        }
        /* As duas formas de busca compartilham o mesmo submenu de detalhes. */
        if (posicao >= 0)
            mudou = gerenciarAgente(posicao, ids, nomes, scripts, avaliacoes, &quantidade);
        if (mudou) pendente = 1;
        /* Salva alteracoes e tenta novamente se houve falha anterior. */
        if (pendente) {
            if (salvarDados(caminho, ids, nomes, scripts, avaliacoes, quantidade, proximoId)) {
                pendente = 0;
                puts("Dados salvos.");
            } else fputs("ATENCAO: alteracoes somente em memoria. Verifique o caminho e permissoes.\n", stderr);
        }
    }
    /* Ultima tentativa de persistencia antes de encerrar, inclusive por EOF. */
    if (pendente && !salvarDados(caminho, ids, nomes, scripts, avaliacoes, quantidade, proximoId)) {
        fputs("Nao foi possivel salvar as alteracoes pendentes.\n", stderr);
        return 1;
    }
    puts("Encerrando o programa.");
    return 0;
}
