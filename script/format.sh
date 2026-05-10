#!/bin/bash

CF="clang-format-18"

if ! command -v $CF &> /dev/null; then
    echo "Error: $CF is not installed."
    echo "Please run: sudo apt-get install clang-format-18"
    exit 1
fi

if [[ "$1" == "--fix" ]]; then
    find src/ include/ tests/ -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.cc' \) -print0 | xargs -0 $CF -i
    echo "Done!"
else
    find src/ include/ tests/ -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.cc' \) -print0 | xargs -0 $CF --dry-run -Werror
fi