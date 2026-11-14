#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include "entrada.h"

/* Retorna 1 para texto UTF-8 preenchido e dentro do limite de pontos de codigo.
   Retorna 0 para codificacao invalida, excesso, controles ou somente espacos.
   O argumento deve ser uma string terminada em zero. */
int textoValido(const char *texto, int limite)
{
    const unsigned char *p = (const unsigned char*)texto;
    int quantidade = 0, conteudo = 0;
    while (*p) {
        unsigned long codigo;
        int extras;
        /* O primeiro byte determina quantos bytes de continuacao devem seguir. */
        if (*p < 0x80) { codigo = *p; extras = 0; }
        else if (*p >= 0xC2 && *p <= 0xDF) { codigo = *p & 0x1F; extras = 1; }
        else if (*p >= 0xE0 && *p <= 0xEF) { codigo = *p & 0x0F; extras = 2; }
        else if (*p >= 0xF0 && *p <= 0xF4) { codigo = *p & 0x07; extras = 3; }
        else return 0;
        ++p;
        for (int i = 0; i < extras; ++i) {
            if ((*p & 0xC0) != 0x80) return 0;
            codigo = (codigo << 6) | (*p & 0x3F);
            ++p;
        }
        /* Rejeita sequencias longas demais para o valor, substitutos UTF-16
           e valores fora da faixa Unicode. */
        if ((extras == 1 && codigo < 0x80) ||
            (extras == 2 && codigo < 0x800) ||
            (extras == 3 && codigo < 0x10000) ||
            codigo > 0x10FFFF || (codigo >= 0xD800 && codigo <= 0xDFFF))
            return 0;
        if (codigo < 32 || (codigo >= 127 && codigo <= 159)) return 0;
        /* Espacos Unicode tambem nao contam como conteudo do campo. */
        if (!(codigo == 32 || codigo == 160 || codigo == 0x1680 ||
              (codigo >= 0x2000 && codigo <= 0x200A) ||
              codigo == 0x2028 || codigo == 0x2029 || codigo == 0x202F ||
              codigo == 0x205F || codigo == 0x3000)) conteudo = 1;
        if (++quantidade > limite) return 0;
    }
    return conteudo;
}

/* Le uma linha no buffer de capacidade bytes, incluindo o terminador.
   Repete entradas invalidas; retorna 1 no sucesso e 0 em EOF/erro de leitura. */
int lerTexto(const char *mensagem, char *destino, int capacidade)
{
    if (capacidade < 2) return 0;
    for (;;) {
        int caractere;
        printf("%s", mensagem);
        fflush(stdout);
        if (!fgets(destino, capacidade, stdin)) return 0;
        /* Sem Enter no buffer, examina o proximo byte: pode ser apenas o Enter
           de uma linha que coube exatamente. Havendo excesso, descarta-o. */
        if (!strchr(destino, '\n')) {
            caractere = getchar();
            if (caractere != '\n' && caractere != EOF) {
                while ((caractere = getchar()) != '\n' && caractere != EOF) {}
                puts("Texto muito longo. Tente novamente.");
                continue;
            }
        }
        destino[strcspn(destino, "\n")] = '\0';
        /* Aceita CRLF no fim da linha, mas preserva controles internos
           para que textoValido os rejeite sem truncar silenciosamente. */
        size_t tamanho = strlen(destino);
        if (tamanho > 0 && destino[tamanho - 1] == '\r')
            destino[tamanho - 1] = '\0';
        if (!textoValido(destino, capacidade - 1)) {
            puts("Digite um texto preenchido, em UTF-8 e sem caracteres de controle.");
            continue;
        }
        return 1;
    }
}

/* Separa capacidade em bytes do limite de caracteres exigido para o campo. */
int lerTextoLimitado(const char *mensagem, char *destino, int capacidade, int limite)
{
    while (lerTexto(mensagem, destino, capacidade)) {
        if (textoValido(destino, limite)) return 1;
        printf("O limite e de %d caracteres. Tente novamente.\n", limite);
    }
    return 0;
}

/* Converte a linha inteira em inteiro dentro do intervalo inclusivo.
   destino so e alterado quando o valor e aceito; retorna 1 ou 0 (fim/erro). */
int lerInteiro(const char *mensagem, int minimo, int maximo, int *destino)
{
    char entrada[64], *fim;
    while (lerTexto(mensagem, entrada, sizeof entrada)) {
        errno = 0;
        /* fim permite rejeitar sufixos como "2abc"; errno detecta estouro. */
        long numero = strtol(entrada, &fim, 10);
        if (fim == entrada || errno == ERANGE) {
            puts("Digite um numero inteiro valido.");
            continue;
        }
        while (isspace((unsigned char)*fim)) ++fim;
        if (*fim || numero < minimo || numero > maximo) {
            printf("Digite um inteiro entre %d e %d.\n", minimo, maximo);
            continue;
        }
        *destino = (int)numero;
        return 1;
    }
    return 0;
}

/* Aceita nota finita entre 0 e 5. Repete ate sucesso (1) ou fim/erro (0).
   Usa ponto decimal no locale C padrao do programa. */
int lerNota(const char *mensagem, float *destino)
{
    char entrada[64], *fim;
    while (lerTexto(mensagem, entrada, sizeof entrada)) {
        errno = 0;
        float nota = strtof(entrada, &fim);
        if (fim == entrada || errno == ERANGE) {
            puts("Digite uma nota numerica representavel.");
            continue;
        }
        while (isspace((unsigned char)*fim)) ++fim;
        /* strtof reconhece nan e inf; essas representacoes nao sao notas. */
        if (*fim || !isfinite(nota) || nota < 0 || nota > 5) {
            puts("A nota deve ser um numero de 0 a 5. Use ponto para decimais.");
            continue;
        }
        *destino = nota;
        return 1;
    }
    return 0;
}
