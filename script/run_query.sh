#!/bin/bash

set -e

QUERY_NUM=$1
COLUMNAR_FILE=$2
OUTPUT_CSV=$3
LOG_FILE=$4

# Экспортируем переменные, чтобы C++ код их увидел через getenv
export BRICKS_INPUT=$COLUMNAR_FILE
export BRICKS_OUTPUT=$OUTPUT_CSV

# Путь к бинарнику
EXECUTABLE="./build/tests/operators/clickbench"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Binary not found at $EXECUTABLE"
    exit 1
fi

# Запускаем GTest с фильтром по номеру запроса
# Флаг --gtest_filter=ClickBenchTest.Query0 запустит только нужный тест
echo "Running GTest for Query $QUERY_NUM..."
$EXECUTABLE --gtest_filter="ClickBenchTest.Query${QUERY_NUM}" > "$LOG_FILE" 2>&1

# Проверяем, прошел ли тест
if [ $? -eq 0 ]; then
    echo "Query $QUERY_NUM: SUCCESS"
else
    echo "Query $QUERY_NUM: FAILED (check $LOG_FILE)"
    exit 1
fi