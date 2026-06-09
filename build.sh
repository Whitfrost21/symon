#!/bin/bash
set -e

g++ -std=c++20 \
    -Isrc \
    -Iexternal \
    src/main.cpp \
    -o symon
