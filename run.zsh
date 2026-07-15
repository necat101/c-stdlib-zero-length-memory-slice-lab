#!/usr/bin/env zsh
set -euo pipefail

# Resolve Zig
if [[ -n "${ZIG_BIN:-}" && -x "$ZIG_BIN" ]]; then
  :
elif (( $+commands[zig] )); then
  ZIG_BIN="$(command -v zig)"
elif [[ -x "$HOME/.local/bin/zig" ]]; then
  ZIG_BIN="$HOME/.local/bin/zig"
elif [[ -x "$HOME/bin/zig" ]]; then
  ZIG_BIN="$HOME/bin/zig"
elif [[ -x "/home/ubuntu/.local/zig/zig" ]]; then
  ZIG_BIN="/home/ubuntu/.local/zig/zig"
fi
export ZIG_BIN

# Resolve Python
if [[ -n "${PYTHON_BIN:-}" && -x "$PYTHON_BIN" ]]; then
  :
elif (( $+commands[python3] )); then
  PYTHON_BIN="$(command -v python3)"
elif (( $+commands[python] )); then
  PYTHON_BIN="$(command -v python)"
fi
export PYTHON_BIN

if [[ -z "${ZIG_BIN:-}" ]]; then
  print -u2 "warning: zig not found – c-dependent rows will be toolchain_skip"
fi

print "==> run_lab.py"
"$PYTHON_BIN" run_lab.py
print
print "==> test_lab.py"
"$PYTHON_BIN" -m unittest -v
