#!/usr/bin/env sh
# Runs the project automation tests: test.sh [FILTER] [--build auto|always|never]
exec "$(cd "$(dirname "$0")" && pwd)/ue.sh" test "$@"
