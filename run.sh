#!/usr/bin/env bash
set -euo pipefail

# Resolve Zig
if [ -n "${ZIG_BIN:-}" ] && [ -x "$ZIG_BIN" ]; then
  :
elif command -v zig >/dev/null 2>&1; then
  ZIG_BIN="$(command -v zig)"
elif [ -x "$HOME/.local/bin/zig" ]; then
  ZIG_BIN="$HOME/.local/bin/zig"
elif [ -x "$HOME/bin/zig" ]; then
  ZIG_BIN="$HOME/bin/zig"
elif [ -x "/home/ubuntu/.local/zig/zig" ]; then
  ZIG_BIN="/home/ubuntu/.local/zig/zig"
fi
export ZIG_BIN

# Resolve Python
if [ -n "${PYTHON_BIN:-}" ] && [ -x "$PYTHON_BIN" ]; then
  :
elif command -v python3 >/dev/null 2>&1; then
  PYTHON_BIN="$(command -v python3)"
elif command -v python >/dev/null 2>&1; then
  PYTHON_BIN="$(command -v python)"
fi
export PYTHON_BIN

if [ -z "${ZIG_BIN:-}" ]; then
  echo "warning: zig not found – c-dependent rows will be toolchain_skip" >&2
fi

echo "==> run_lab.py"
"$PYTHON_BIN" run_lab.py
echo
echo "==> test_lab.py"
"$PYTHON_BIN" -m unittest -v
