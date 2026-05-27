#!/bin/bash

set -e

# $1 — это первый аргумент (INPUT_Csv)
# $2 — это второй аргумент (COLUMNAR)
INPUT_FILE=$1
OUTPUT_FILE=$2
SCHEME_FILE="./additional_files/schema.csv"
CONVERTER="./build/tests/transform/csv_to_columnar_test"

echo "Converting $INPUT_FILE with $SCHEME_FILE to $OUTPUT_FILE."

# Проверяем, существует ли бинарник (на случай, если билд упал ранее)
if [ ! -f "$CONVERTER" ]; then
    echo "Error: Converter binary not found at $CONVERTER"
    exit 1
fi

# Запуск конвертации. 
# Ввод параметров обычно делается через аргументы командной строки.
$CONVERTER "$INPUT_FILE" "$SCHEME_FILE" "$OUTPUT_FILE"

echo "Conversion completed!"