# Escalonamento de tarefas críticas de voo (Rate-Monotonic e EDF)

Simulador de escalonamento preemptivo comparando Rate-Monotonic (RM) e
Earliest-Deadline-First (EDF) para tarefas periódicas com deadline
menor que o período.

Implementado e testado em **Windows 10**, com toolchain **MinGW-w64
(gcc)**. O código é C99 padrão, sem dependências de sistema
operacional, e compila da mesma forma em Linux.

## Arquivos

Todo o código-fonte está em `src/`:

- `task.h` — `Task` (configuração estática de uma tarefa: nome,
  período, deadline, burst, posição no arquivo) e `TaskState` (estado
  mutável durante a simulação: rajada restante, deadline absoluto da
  instância atual, próxima chegada, contadores finais).
- `errors.h` — um código de saída (`ErrorCode`) para cada categoria de
  erro exigida na especificação.
- `parser.h` / `parser.c` — leitura e validação do arquivo de entrada:
  tempo total de simulação e a lista de tarefas, com verificação de
  campo faltando, valor não numérico, valor não positivo e violação de
  `C ≤ D ≤ P`.
- `sim.h` / `sim.c` — motor de simulação, compartilhado por RM e EDF.
  É **orientado a eventos** (não avança unidade a unidade): a cada
  iteração processa chegadas periódicas, remove tarefas que perderam o
  deadline aguardando na fila, seleciona a tarefa pronta de maior
  prioridade (critério injetado por função) e calcula até quando ela
  roda sem interrupção. Uma linha de execução só é fechada quando a
  tarefa em execução realmente muda (evita registrar preempções que
  não aconteceram de fato).
- `rate.h` / `rate.c` — critério de prioridade do Rate-Monotonic:
  menor período vence.
- `edf.h` / `edf.c` — critério de prioridade do EDF: menor deadline
  absoluto da instância atual vence.
- `output.h` / `output.c` — grava o arquivo `<algoritmo>_alvs.out`
  com as seções `EXECUTION BY <ALGO>`, `LOST DEADLINES`,
  `COMPLETE EXECUTION` e `KILLED`. Escreve em modo binário para que a
  saída use sempre `\n`, independente da plataforma de compilação.
- `config.h` — define `STUDENT_LOGIN` ("alvs"), usado para nomear o
  arquivo de saída.
- `main.c` — ponto de entrada: valida argumentos, escolhe o algoritmo,
  orquestra parser → simulação → saída, e mapeia cada falha para o
  código de saída correspondente.

Em qualquer erro, uma mensagem é escrita em `stderr`, o programa
encerra com código de saída diferente de zero e **nenhum** arquivo
`.out` é criado. Nada é impresso em `stdout` durante a execução
normal.

### Códigos de saída (`errors.h`)

| Código | Situação |
|---|---|
| 0 | sucesso |
| 1 | número incorreto de argumentos |
| 2 | primeiro argumento diferente de `rate`/`edf` |
| 3 | arquivo de entrada inexistente ou ilegível |
| 4 | arquivo malformado (campo faltando, valor não numérico ou não positivo) |
| 5 | tarefa viola `C ≤ D ≤ P` |
| 6 | falha interna (ex.: falta de memória) |

## Como compilar

Na raiz deste diretório (`alvs/`):

```sh
make
```

Gera o executável `scheduler`. `make clean` remove os objetos e o
executável.

## Como executar

```sh
./scheduler rate caminho/para/arquivo.txt
./scheduler edf  caminho/para/arquivo.txt
```

O resultado é gravado em `rate_alvs.out` ou `edf_alvs.out`, no
diretório onde o comando foi executado.

Formato do arquivo de entrada:

```
[TEMPO TOTAL]
[NOME] [PERIODO] [DEADLINE] [BURST]
...
```

## Como testar

Os arquivos de teste estão em `tests/`: o exemplo do enunciado
(`voo.txt`, com a saída esperada em `expected_rate_voo.out`) e um
arquivo para cada categoria de erro exigida (arquivo vazio, campo
faltando, valor não numérico, valor não positivo, `C > D`, `D > P`,
além de argumentos incorretos, algoritmo inválido e arquivo
inexistente).

```sh
make
sh tests/run_tests.sh
```

O script compara código de saída, garante que nada é impresso em
`stdout` e que nenhum `.out` é criado nos casos de erro, e confere que
a saída de `rate` para `voo.txt` é idêntica, byte a byte, ao trace do
enunciado.
