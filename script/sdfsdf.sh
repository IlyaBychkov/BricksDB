#!/bin/bash

set -e

echo "===== clang-format check ====="
find src/ include/ tests/ -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.cc' \) -print0 | xargs -0 clang-format --dry-run -Werror
echo ""

echo "===== clang-tidy check ====="
find src/ tests/ -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.cc' \) -print0 | xargs -0 clang-tidy -p build
echo ""

echo "All checks passed!"
