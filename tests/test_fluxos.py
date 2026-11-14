from pathlib import Path
import subprocess
import sys
import tempfile

programa = str(Path(sys.argv[1] if len(sys.argv) > 1 else './programa').resolve())
executados = 0


def executar(arquivo, entrada, codigo=0):
    global executados
    resultado = subprocess.run([programa, str(arquivo)], input=entrada,
                               text=True, capture_output=True, timeout=10)
    assert resultado.returncode == codigo, resultado.stdout + resultado.stderr
    executados += 1
    return resultado.stdout + resultado.stderr


with tempfile.TemporaryDirectory(prefix='pim-testes-') as pasta:
    pasta = Path(pasta)
    dados = pasta / 'agentes.txt'
    saida = executar(dados, '2\nabc\n2abc\n9\n0\n')
    assert 'Nenhum agente cadastrado' in saida
    assert not dados.exists()

    saida = executar(dados, '1\n\n   \nAssistente financeiro\nOrganizar despesas mensais.\n'
                     'abc\n4abc\n-1\n6\nnan\ninf\n1e9999\n4.5\n2\n0\n')
    assert 'ID: 1' in saida and 'Nota: 4.50' in saida
    assert 'Dados salvos' in saida
    saida = executar(dados, '3\n1\n0\n4\nfinanceiro\n1\n0\n3\n99\n0\n')
    assert 'Organizar despesas mensais.' in saida and 'Agente nao encontrado' in saida

    executar(dados, '1\nAssistente financeiro\nOutro script\n0\n0\n')
    saida = executar(dados, '4\nfinanceiro\n99\n2\n1\nNovo nome\nNovas instrucoes\n5\n0\n')
    assert 'Escolha um ID apresentado' in saida and 'Agente atualizado' in saida
    saida = executar(dados, '3\n2\n0\n0\n')
    assert 'Novo nome' in saida and 'Novas instrucoes' in saida and 'Nota: 5.00' in saida

    anterior = dados.read_bytes()
    executar(dados, '3\n2\n1\nEdicao interrompida\n')
    assert dados.read_bytes() == anterior
    executar(dados, '1\nCadastro interrompido\n')
    assert dados.read_bytes() == anterior
    executar(dados, '3\n1\n2\n0\n0\n')
    assert dados.read_bytes() == anterior
    executar(dados, '3\n1\n2\n1\n0\n')
    saida = executar(dados, '2\n1\nTerceiro\nScript terceiro\n3\n0\n')
    assert 'Nome: Novo nome | Nota: 5.00' in saida and 'Agente cadastrado. ID: 3' in saida

    unicode = pasta / 'unicode.txt'
    nome = 'á' * 80
    script = '😀' * 1000
    saida = executar(unicode, '1\n' + 'a' * 81 + '\n' + nome + '\n' +
                     '😀' * 1001 + '\n' + script + '\n5\n0\n')
    assert 'limite e de 80' in saida and 'Texto muito longo' in saida
    saida = executar(unicode, '3\n1\n0\n0\n')
    assert nome in saida and script in saida

    cheio = pasta / 'cheio.txt'
    entrada = ''.join(f'1\nNome {i}\nScript {i}\n{i % 6}\n' for i in range(50))
    saida = executar(cheio, entrada + '1\n2\n0\n')
    assert 'Catalogo cheio' in saida and 'Total: 50' in saida
    saida = executar(cheio, '3\n25\n2\n1\n1\nReposicao\nScript novo\n2\n2\n0\n')
    assert 'Agente cadastrado. ID: 51' in saida and 'Total: 50' in saida

    vazio = pasta / 'vazio.txt'
    executar(vazio, '1\nUnico\nInstrucoes\n0\n3\n1\n2\n1\n0\n')
    saida = executar(vazio, '2\n1\nOutro\nInstrucoes\n5\n0\n')
    assert 'Nenhum agente cadastrado' in saida and 'Agente cadastrado. ID: 2' in saida
    saida = executar(vazio, '4\nInexistente\n4\nOutro\n0\n0\n')
    assert 'Nenhum agente encontrado' in saida

    corrompido = pasta / 'corrompido.txt'
    for conteudo in ['invalido\n', 'PIM2 1\n51 52\n',
                     'PIM2 1\n1 2\n1 nan\nNome\nScript\n',
                     'PIM2 1\n2 2\n1 2\nNome\nScript\n1 3\nOutro\nScript\n']:
        corrompido.write_text(conteudo)
        executar(corrompido, '0\n', codigo=1)
        assert corrompido.read_text() == conteudo

    saida = executar(pasta / 'inexistente' / 'dados.txt', '1\nNome\nScript\n2\n0\n', codigo=1)
    assert 'alteracoes somente em memoria' in saida

print(f'OK: {executados} execucoes; cadastro, entradas, buscas, edicao, exclusao, limites e arquivos.')
