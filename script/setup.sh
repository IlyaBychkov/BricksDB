#!/bin/bash

# Прерывать выполнение при любой ошибке
set -e  

echo "Starting setup..."

# 1. Обновляем списки пакетов
apt-get update

# 2. Устанавливаем базовые инструменты сборки
# Нам нужны: cmake, компилятор (GCC или Clang), git
apt-get install -y \
    build-essential \
    cmake \
    clang-20 \
    libgtest-dev

# 3. Устанавливаем инструменты проверки (те самые, что мы фиксировали)
sudo apt-get install -y \
    clang-format-18 \
    clang-tidy-18

echo "Setup finished successfully!"