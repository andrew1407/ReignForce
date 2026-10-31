#!/usr/bin/env sh
# Linux / macOS launcher for ue.py: prefers the engine bundled Python (when UE_ROOT is set), else system python3
DIR="$(cd "$(dirname "$0")" && pwd)"

case "$(uname -s)" in
    Darwin) HOST=Mac ;;
    *) HOST=Linux ;;
esac

PYTHON="python3"
if [ -n "$UE_ROOT" ] && [ -x "$UE_ROOT/Engine/Binaries/ThirdParty/Python3/$HOST/bin/python3" ]; then
    PYTHON="$UE_ROOT/Engine/Binaries/ThirdParty/Python3/$HOST/bin/python3"
fi

exec "$PYTHON" -I "$DIR/ue.py" "$@"
