# VERIFY.md

Clean-clone verification for c-stdlib-zero-length-memory-slice-lab v2

## Repository

- URL: https://github.com/necat101/c-stdlib-zero-length-memory-slice-lab
- Implementation SHA: a2d5d3d0a6efd6dee1da9d5f897ba5ee255dcfce
- Documentation SHA: 56d8e179cd6d1cdb00e87a41630fe25d8c45aadc

Implementation commit is direct parent of documentation commit.

## Clone

```
git clone https://github.com/necat101/c-stdlib-zero-length-memory-slice-lab.git verify_v2
cd verify_v2
git checkout a2d5d3d0a6efd6dee1da9d5f897ba5ee255dcfce
```

## Toolchain

- Zig candidates attempted: $ZIG_BIN, $(command -v zig), ~/.local/bin/zig, ~/bin/zig, /home/ubuntu/.local/zig/zig, /opt/zig/zig, /usr/local/bin/zig, $OPENCLAW_TOOL_ZIG
- Selected zig: /portable-zig/.local/zig/zig
- Zig version: 0.14.0
- zig cc version: clang version 19.1.7 (https://github.com/ziglang/zig-bootstrap 1c3c59435891bc9caf8cd1d3783773369d191c5f)
- Compiler target: x86_64-unknown-linux-musl
- Python candidates: $PYTHON_BIN, python3, python
- Selected python: /usr/bin/python3
- Python version: 3.12.3
- Platform: linux

## C representation markers

- STDC_VERSION: 201112
- CHAR_BIT: 8
- sizeof(char): 1
- sizeof(void*): 8
- sizeof(size_t): 8
- SIZE_MAX: 18446744073709551615

## Validation commands

```
$ZIG_BIN cc -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror memory_lab.c -o memory_lab_check -lm
exit code: 0

python3 -m py_compile run_lab.py test_lab.py
exit code: 0

python3 run_lab.py
exit code: 0
rows=100
classifications: pass=25, expected_error=1, context_only=4, not_applicable=70, fail=0, local_observation=0, implementation_skip=0, toolchain_skip=0

python3 -m unittest -v
exit code: 0
Ran 22 tests — OK
```

## Results

- cases: 20
- methods: 5
- rows: 100
- classifications sum to 100, zero buckets reported
- valid zero-length memcpy: destination unchanged, ret == dst — pass
- valid zero-length memmove: buffer unchanged — pass
- valid zero-length memset: buffer unchanged — pass
- valid zero-length memcmp: result 0 — pass
- one-past zero-length: arrays unchanged, memcmp 0 — pass
- guarded null empty copy: success, no libc call (C) — pass
- guarded null empty compare: equality true, no libc call (C) — pass
- nonzero null rejection: all 5 cases rejected before libc (C) — expected_error
- memcpy non-overlap: 9 lengths 0..8 verified against C evidence — pass
- memmove forward overlap: ababcdef (matches C) — pass
- memmove backward overlap: cdefghgh (matches C) — pass
- overlap policy: memmove selected — pass
- memcmp signs: lengths 0,1,2 equal; 3,4 negative (matches C) — pass
- empty slice concat: abcd, null slices guarded (C) — pass
- feature row copy: 0.25, -0.0, 1.5, 2.0 preserved, signbit true — pass
- empty feature batch: 0 rows, 0 elements, 0 bytes, checked multiplication, no overflow, no memcpy (C) — pass
- nonempty feature batch: 2 rows × 3 cols, 6 elements, 24 bytes, 0.0, 0.5, 1.0, -1.0, 2.0, 3.5 (C) — pass
- n3322 / C2Y scope documented

JSON, CSV, and RESULTS.md agree. Committed vs regenerated evidence matches when normalizing elapsed_time fields only.

Normalization command:
```
python3 -c "import json; d=json.load(open('results_rows.json')); [r.pop('elapsed_time',None) for r in d]; print(json.dumps(d,sort_keys=True))"
# diff against committed results_rows.json with elapsed_time stripped
```

Comparison: identical (100 rows).

## Working tree

After regeneration:
```
M RESULTS.md
```
(diff: Total runtime 0.34s → 0.32s)

Restored with `git checkout -- RESULTS.md`

Final `git status --porcelain`: empty

## Timing

Full verification wall-clock (clone, checkout, discovery, compilation, execution, comparison, testing, restoration, final status): ~45 seconds

## Summary

- toolchain_skip: 0
- implementation_skip: 0
- fail: 0
- unittest: 22 tests, OK
- artifact scanner: pass
- clean-verification: PASS
