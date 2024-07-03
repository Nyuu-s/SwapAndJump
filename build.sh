#!/bin/bash
libs=-luser32
warnings=-Wno-writable-strings

clang src/main.cpp -oJumpAndSwap.exe $libs $warnings