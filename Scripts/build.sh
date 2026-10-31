#!/usr/bin/env sh
# Compiles the project: build.sh [editor|game] [-c CONFIG] [--if-needed]
exec "$(cd "$(dirname "$0")" && pwd)/ue.sh" build "$@"
