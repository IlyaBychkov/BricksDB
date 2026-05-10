#!/bin/bash

# Прерывать выполнение при ошибке
set -e

echo "Starting build process..."

# 1. Очищаем и создаем папку для сборки
# Если папка build существует, удаляем её рекурсивно
if [ -d "build" ]; then
    echo "Cleaning up existing build directory..."
    rm -rf build
fi

# 2. Создаем папку для сборки
mkdir build
cd build

# 3. Конфигурация проекта через CMake
# Мы явно указываем тип сборки Release для максимальной производительности бенчмарка.
# Также используем clang-20, который мы установили в setup.sh
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_COMPILER=clang++-20 \
      -DCMAKE_C_COMPILER=clang-20 \
      ..

# 4. Компиляция
# Флаг -j$(nproc) позволяет использовать все доступные ядра процессора,
# что значительно ускоряет сборку в Docker.
make -j$(nproc)

echo "Build finished successfully!"