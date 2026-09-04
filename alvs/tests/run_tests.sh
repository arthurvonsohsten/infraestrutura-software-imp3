#!/bin/sh
# Executa o scheduler contra o exemplo do enunciado e cada categoria de
# erro exigida, conferindo codigo de saida, ausencia de saida em stdout
# e ausencia de arquivo .out quando a entrada e invalida.
#
# Uso: rode 'make' na raiz do projeto antes, depois execute este
# script a partir de qualquer diretorio: sh tests/run_tests.sh

set -u

SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)
ROOT_DIR=$(dirname "$SCRIPT_DIR")
SCHEDULER="$ROOT_DIR/scheduler"
if [ ! -x "$SCHEDULER" ] && [ -x "$SCHEDULER.exe" ]; then
    SCHEDULER="$SCHEDULER.exe"
fi

if [ ! -x "$SCHEDULER" ]; then
    echo "erro: rode 'make' na raiz do projeto antes de testar" >&2
    exit 1
fi

cd "$SCRIPT_DIR" || exit 1
rm -f rate_alvs.out edf_alvs.out

PASS=0
FAIL=0

check() {
    name="$1"
    expected_exit="$2"
    shift 2

    stdout=$("$@" 2>/tmp/run_tests_stderr.$$)
    actual_exit=$?
    rm -f /tmp/run_tests_stderr.$$

    ok=1

    if [ "$actual_exit" -ne "$expected_exit" ]; then
        echo "FALHOU [$name]: exit esperado=$expected_exit obtido=$actual_exit"
        ok=0
    fi

    if [ -n "$stdout" ]; then
        echo "FALHOU [$name]: stdout deveria estar vazio, obteve: $stdout"
        ok=0
    fi

    if [ "$expected_exit" -ne 0 ]; then
        if [ -f rate_alvs.out ] || [ -f edf_alvs.out ]; then
            echo "FALHOU [$name]: arquivo .out nao deveria ter sido criado"
            ok=0
        fi
    fi

    if [ "$ok" -eq 1 ]; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
    fi
}

# --- numero incorreto de argumentos ---
check "sem argumentos"        1 "$SCHEDULER"
check "argumento a mais"      1 "$SCHEDULER" rate voo.txt extra

# --- algoritmo invalido ---
check "algoritmo invalido"    2 "$SCHEDULER" foo voo.txt

# --- arquivo de entrada ---
check "arquivo inexistente"   3 "$SCHEDULER" rate nao_existe.txt
check "arquivo vazio"         4 "$SCHEDULER" rate erro_vazio.txt
check "campo faltando"        4 "$SCHEDULER" rate erro_campo_faltando.txt
check "valor nao numerico"    4 "$SCHEDULER" rate erro_nao_numerico.txt
check "valor nao positivo"    4 "$SCHEDULER" rate erro_nao_positivo.txt
check "C maior que D"         5 "$SCHEDULER" rate erro_c_maior_d.txt
check "D maior que P"         5 "$SCHEDULER" rate erro_d_maior_p.txt

# --- execucao valida: rate ---
check "rate valido"           0 "$SCHEDULER" rate voo.txt
if [ -f rate_alvs.out ]; then
    if diff -q expected_rate_voo.out rate_alvs.out >/dev/null 2>&1; then
        PASS=$((PASS + 1))
    else
        echo "FALHOU [rate valido]: saida difere de expected_rate_voo.out"
        FAIL=$((FAIL + 1))
    fi
else
    echo "FALHOU [rate valido]: rate_alvs.out nao foi criado"
    FAIL=$((FAIL + 1))
fi
rm -f rate_alvs.out

# --- execucao valida: edf ---
check "edf valido"            0 "$SCHEDULER" edf voo.txt
if [ -f edf_alvs.out ]; then
    PASS=$((PASS + 1))
else
    echo "FALHOU [edf valido]: edf_alvs.out nao foi criado"
    FAIL=$((FAIL + 1))
fi
rm -f edf_alvs.out

echo ""
echo "passou=$PASS falhou=$FAIL"
[ "$FAIL" -eq 0 ]
