#!/bin/bash

# Compilar con símbolos de depuración
gcc -Wall -Wextra -std=c11 -g -o ring ring.c || { echo "❌ Falló la compilación"; exit 1; }

echo "🧪 Iniciando tests con Valgrind..."

TEMP_OUT=$(mktemp)
VALG_LOG=$(mktemp)

run_valgrind_test() {
    local n="$1"
    local c="$2"
    local s="$3"
    local expected="$4"
    local label="$5"

    echo -e "\n🔸 Test: $label"
    
    timeout 5 valgrind --leak-check=full --error-exitcode=123 \
        --log-file="$VALG_LOG" ./ring "$n" "$c" "$s" > "$TEMP_OUT" 2>/dev/null
    EXIT_CODE=$?

    if [ $EXIT_CODE -eq 124 ]; then
        echo "❌ TIMEOUT - El programa tardó demasiado"
        return
    elif [ $EXIT_CODE -eq 123 ]; then
        echo "❌ MEMORY LEAK - Detectado por Valgrind"
        cat "$VALG_LOG"
        return
    fi

    if grep -q "Resultado final: $expected" "$TEMP_OUT"; then
        echo "✅ OK - Resultado esperado: $expected"
    else
        echo "❌ ERROR - Se esperaba: $expected"
        echo "   Salida obtenida:"
        cat "$TEMP_OUT"
    fi
}

# Casos de prueba
run_valgrind_test 3 10 0 13 "3 procesos, inicio en 0"
run_valgrind_test 4 20 2 24 "4 procesos, inicio en 2"
run_valgrind_test 5 100 2 105 "5 procesos, inicio en 2"
run_valgrind_test 6 0 5 6 "6 procesos, inicio en 5"
run_valgrind_test 10 1 0 11 "10 procesos, inicio en 0"

rm "$TEMP_OUT" "$VALG_LOG"
