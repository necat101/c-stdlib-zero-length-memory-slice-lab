# RESULTS

- zig: /portable-zig/.local/zig/zig — 0.14.0
- zig cc target: x86_64-unknown-linux-musl
- zig cc version: clang version 19.1.7 (https://github.com/ziglang/zig-bootstrap 1c3c59435891bc9caf8cd1d3783773369d191c5f)
- compile flags: -std=c11 -O2 -Wall -Wextra -Wpedantic -Werror
- c_language_mode: c11
- python: /usr/bin/python3 — 3.12.3
- platform: linux
- STDC_VERSION: 201112
- CHAR_BIT: 8, sizeof(void*): 8, sizeof(size_t): 8
- cases: 20
- methods: 5
- rows: 100

## Classification summary

- pass: 25
- expected_error: 1
- local_observation: 0
- implementation_skip: 0
- toolchain_skip: 0
- context_only: 4
- not_applicable: 70
- fail: 0

## Observations

- valid zero-length memcpy: destination unchanged, ret == dst
- valid zero-length memmove: buffer unchanged
- valid zero-length memset: buffer unchanged
- valid zero-length memcmp: result 0
- one-past zero-length: arrays unchanged, memcmp 0
- guarded null empty copy: success, no libc call (C)
- guarded null empty compare: equality true, no libc call (C)
- nonzero null rejection: all 5 cases rejected before libc (C)
- memcpy non-overlap: 9 lengths 0..8 verified against C evidence
- memmove forward overlap: ababcdef (matches C)
- memmove backward overlap: cdefghgh (matches C)
- overlap policy: memmove selected
- memcmp signs: lengths 0,1,2 equal; 3,4 negative (matches C)
- empty slice concat: abcd, null slices guarded (C)
- feature row copy: 0.25, -0.0, 1.5, 2.0 preserved, signbit true
- empty feature batch: 0 rows, 0 elements, 0 bytes, checked multiplication, no overflow, no memcpy (C)
- nonempty feature batch: 2 rows × 3 cols, 6 elements, 24 bytes, values 0.0, 0.5, 1.0, -1.0, 2.0, 3.5 copied (C)
- n3322 / C2Y scope documented, no null-pointer UB executed

Total runtime: 0.34s

## Narrow conclusions

Byte-level memory contracts validated locally under C11. No null-pointer libc calls executed. Guarded wrappers (C implementation) provide C11-safe empty-slice policy. No ML model validated.
