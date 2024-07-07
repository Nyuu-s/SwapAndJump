#!/bin/bash
libs=-luser32
warnings="-Wno-writable-strings -Wno-format-security"

clang++ -g src/main.cpp -oJumpAndSwap.exe $libs $warnings --target=x86_64-w64-windows-gnu