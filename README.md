# AI Agent Registry — PIM II

Catálogo de configurações de agentes de IA, desenvolvido em C11 e executado no terminal. O programa armazena configurações: não executa IA nem se conecta a serviços externos.

## Compilar e executar

Requisitos: GCC (ou compilador C11), Make e terminal UTF-8. Python 3 é necessário apenas para executar os testes automatizados; o sistema é inteiramente em C.

Abra o terminal na raiz do projeto, onde estão o Makefile e as pastas src, include e dados:

```bash
make
./programa
```

Alternativa sem Make:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -I include src/main.c src/entrada.c src/catalogo.c src/arquivo.c -o programa
./programa
```

Execute a partir da raiz do projeto para localizar a pasta dados. Ela deve existir e permitir escrita. O arquivo de registros será criado no primeiro salvamento.

## Manual de uso

| Opção | Operação |
|---|---|
| 1 | Cadastrar: informar nome, instruções em uma única linha e nota |
| 2 | Listar IDs, nomes e notas |
| 3 | Buscar pelo ID e mostrar todos os campos |
| 4 | Buscar por parte do nome, escolher um ID entre os resultados e abrir detalhes |
| 0 | Encerrar |

Depois de uma busca, o programa apresenta ID, nome, instruções e nota, com as opções **1 — Editar**, **2 — Excluir** e **0 — Voltar**.

Na edição, informe novamente nome, instruções e nota; o ID não muda. Os dados antigos só são substituídos depois que todos os novos campos forem aceitos. Na exclusão, confirme com 1 ou cancele com 0. Depois de editar ou excluir, o programa retorna ao menu principal.

Entradas inválidas repetem a pergunta do campo atual. Use **ponto** em notas decimais, por exemplo 4.5. No Linux, Ctrl+D em uma linha vazia indica fim da entrada: uma operação incompleta não é confirmada e o programa encerra. Prefira a opção 0 para sair normalmente.

## Regras de negócio

- No máximo 50 agentes ativos.
- ID inteiro positivo, automático, único e imutável. O próximo ID é salvo no arquivo; excluir um agente não libera seu ID para reutilização.
- Nome: até 80 caracteres; instruções: até 1.000, incluindo espaços.
- Nome e instruções não podem ser vazios, conter somente espaços ou caracteres de controle.
- Os limites contam pontos de código Unicode em UTF-8. Um caractere visual formado por letra e acento combinante pode contar como dois pontos de código.
- Nota numérica finita entre 0 e 5, inclusive; entradas como 4abc, nan e inf são rejeitadas.
- Nomes repetidos são permitidos. A busca por nome é parcial e diferencia maiúsculas, minúsculas e acentos.
- Os registros são organizados em vetores paralelos: a mesma posição em IDs, nomes, scripts e avaliações representa um agente.
- Uma exclusão desloca os quatro vetores em conjunto.

## Persistência e recuperação

O caminho padrão é `dados/agentes.txt`. O arquivo contém uma identificação de versão, a quantidade de agentes, o próximo ID e os registros. Os dados são carregados na abertura e salvos após cada cadastro, edição ou exclusão concluído.

O salvamento escreve primeiro em um arquivo temporário e só depois substitui o anterior. Isso reduz o risco de deixar um arquivo parcial; não é um mecanismo de backup nem uma garantia contra falha física de energia/disco.

Se o arquivo existente for inválido, o programa avisa e encerra sem sobrescrevê-lo. Preserve uma cópia e investigue o conteúdo; não apague o arquivo para tentar corrigir sem antes guardar seus dados.

Se não for possível salvar, o programa avisa que as alterações estão somente em memória, tenta novamente nas próximas operações e ao encerrar. Se a última tentativa falhar, termina com código de erro. Não ignore esse aviso.

É possível indicar outro arquivo:

```bash
./programa dados/demonstracao.txt
```


## Organização

| Arquivo | Responsabilidade |
|---|---|
| src/main.c | Menu, coordenação das operações, carregamento e salvamento |
| src/entrada.c | Leitura de linhas, limites UTF-8 e conversão/validação numérica |
| src/catalogo.c | Cadastro, listagem, buscas, edição e exclusão |
| src/arquivo.c | Leitura, validação e gravação dos registros em arquivo |
| include/*.h | Protótipos e limites compartilhados |
| tests/test_fluxos.py | Testes externos que executam o programa compilado |
| dados/.gitkeep | Mantém a pasta de dados no repositório |

## Testes automatizados

```bash
make test
```

A suíte usa arquivos em uma pasta temporária, sem alterar os registros de uso. Verifica:

- Menu vazio e opções inválidas, incluindo número seguido de letras.
- Cadastro, nota inválida, limites 0 e 5, notas não finitas e erro de conversão.
- Busca por ID, busca parcial, múltiplos resultados e ausência de resultados.
- Edição, interrupção de edição/cadastro e cancelamento de exclusão.
- Exclusão com deslocamento dos vetores e remoção do último agente.
- Persistência entre processos e não reutilização de IDs.
- Limites de 80/1.000 caracteres UTF-8, excesso de texto e catálogo cheio.
- Arquivo inválido, IDs duplicados e falha de salvamento.




