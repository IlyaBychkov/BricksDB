#!/bin/bash

find src/ include/ tests/ -type f \( -name '*.c' -o -name '*.h' -o -name '*.cpp' -o -name '*.cc' \) -print0 | xargs -0 clang-format --dry-run -Werror