#!/bin/bash

# Compilar si no existe
if [ ! -f shell ]; then
    echo "🔧 Compilando shell..."
    gcc -Wall -Wextra -std=c11 -o shell shell.c || exit 1
fi

echo "🧪 Ejecutando tests..."

# Archivo temporal para la salida
OUT=$(mktemp)

# Función para correr un test
run_test() {
    local input="$1"
    local expected="$2"
    local label="$3"

    # Usamos un heredoc para emular entrada por stdin al shell
    echo -e "$input\nexit" | ./shell > "$OUT" 2>/dev/null

    if grep -q "^$expected$" "$OUT"; then
        echo "✅ $label"
    else
        echo "❌ $label"
        echo "   Esperado: '$expected'"
        echo "   Salida:"
        cat "$OUT"
        echo "-------------"
    fi
}

# Tests
run_test "echo hola mundo | wc -w" "2" "Test básico: echo → wc"
run_test "ls | grep .c | wc -l" "$(ls | grep .c | wc -l)" "Test con grep y pipe"
run_test "echo \"hola mundo\"" "hola mundo" "Test de comillas simples"
run_test "echo uno dos tres | wc -w" "3" "Echo con 3 palabras"

# Limpiar
rm "$OUT"
