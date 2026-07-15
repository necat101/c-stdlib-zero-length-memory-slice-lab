# c-stdlib-zero-length-memory-slice-lab

Deterministic C11 standard-library memory-function correctness lab. Tests `memcpy()`, `memmove()`, `memcmp()`, `memset()` with valid zero-length operations, one-past pointers, guarded empty-slice policy, overlapping moves, and a fixed ML-adjacent feature-row copy.

**No null-pointer libc calls are executed.** All null-empty cases use repository guarded wrappers that check length before calling the C library.

## Quick start

```bash
export ZIG_BIN=/path/to/zig
python3 run_lab.py
python3 -m unittest -v
```

## Cases (20)

zig_compiler_marker, c_memory_api_marker, valid_zero_length_memcpy_marker, valid_zero_length_memmove_marker, valid_zero_length_memset_marker, valid_zero_length_memcmp_marker, one_past_zero_length_marker, guarded_null_empty_copy_marker, guarded_null_empty_compare_marker, nonzero_null_rejection_marker, memcpy_nonoverlap_marker, memmove_forward_overlap_marker, memmove_backward_overlap_marker, overlap_not_memcpy_marker, memcmp_length_and_sign_marker, empty_slice_concat_marker, fixed_feature_row_copy_marker, empty_feature_batch_marker, c2y_scope_and_optimizer_marker, no_global_memory_or_ml_validity_claim_marker

5 methods × 20 cases = 100 result rows.

## What the Red Hat article says

The Red Hat article "Making memcpy(NULL, NULL, 0) well-defined" describes the historical C rule where memory functions required valid pointers even when the requested byte count was zero. Under that older contract, a zero length did not automatically make a null pointer argument valid. The article discusses the accepted C2Y direction (WG14 N3322) making specified zero-length operations well-defined, compiler assumptions based on UB (null-check elimination after a memcpy call), empty spans in C++ (`std::span`), caller-side guards that exist in real code to avoid UB, pointer arithmetic constraints, static-analysis concerns about false positives/negatives when null validity depends on both pointer and length, and the distinction between compiler builtins and libc functions.

## WG14 N3322

WG14 paper N3322 proposes making zero-length calls to `memcpy`, `memmove`, `memcmp`, `memset` (and other memory/string functions) well-defined when the size argument is zero, regardless of whether the pointer arguments are null. Functions affected include `memcpy`, `memmove`, `memcmp`, `memset`, `memchr`, `strncpy`, `strncat`, `strncmp`, and others listed in the paper.

Status per https://www.open-std.org/jtc1/sc22/wg14/www/previous.html at task time: consult the official WG14 page for current status.

## Hacker News thread access

Thread 42387013 was read using the bundled Hacker News CLI:

```
cd /usr/lib/node_modules/openclaw/dist/extensions/hackernews/skills/hackernews
python3 ./hackernews get-item --id 42387013
```

Relevant public evidence was captured in `hn_thread_evidence.md` and `hn_comments_sanitized.json` before preparing the sentiment summary below.

### HN discussion summary

**voidUpdate** — argued that the intuitive expectation is that copying zero bytes should do nothing. Zero-length means no work, so null pointers should be harmless in that case. This intuition is natural but did not by itself describe the older C contract, which treated the pointer validity requirement independently of the count.

**mjg59** — explained that under the older C contract, compilers are permitted to make assumptions based on undefined behavior. A `memcpy` call with a potentially null pointer allows the compiler to infer non-nullness, even when the requested length is zero. This is why a later explicit null check can be removed by the optimizer: the earlier memcpy call is taken as proof the pointers must be valid.

**gcc branch-removal discussion** — the thread discussed GCC removing a later null check after a memcpy call. This is about optimization assumptions derived from UB, not about a required runtime crash. The compiler reasons "if memcpy was called, the pointers must be non-null (otherwise UB), therefore the later null check is always false."

**comex** — distinguished ordinary null-dereference optimization (where a compiler removes a null check after an actual dereference) from the more surprising zero-length memcpy case. The latter is more surprising because zero bytes are copied, so no actual memory access occurs, yet the old C rule still made it UB and allowed optimization assumptions.

**int_19h** — argued that major implementations already handle the zero-length case in practice and that standardizing it removes redundant caller-side null checks. Making the behavior well-defined eliminates defensive `if (n > 0)` guards that clutter call sites.

**pkhuong** — framed the older rule in terms of C objects rather than a flat byte-address model. C pointers are about objects and valid pointer values, not raw addresses. Null is not a pointer to an object.

**whytevuhuni** — raised Rust's use of a non-null aligned sentinel for empty strings/slices crossing into C via FFI. Rust empty slices use a non-null dangling pointer (typically aligned, e.g. address 1 or similar sentinel), which matters when passing empty buffers to C functions. This is distinct from passing actual NULL.

**bonzini** — distinguished a valid one-past pointer from a dereferenceable address. A one-past-the-end pointer is valid to form and compare but not to dereference. For a zero-length operation, no byte is read, so the "must be a valid object" requirement was the footgun N3322 addresses. A zero-length operation cannot assume a byte may be read.

**david-gpu** — argued that end-of-buffer pointers constrain speculative-read arguments. If you have a pointer to the end of a buffer, a zero-length copy from that position should be safe since no access occurs.

**sfink** — explained the static-analysis tradeoff: when null validity depends jointly on pointer value AND length, analyzers face a choice between false positives (warning on safe null+0 code), false negatives (missing real null bugs), or more expensive path-sensitive analysis tracking both pointer and length together.

**nmilo** — objected to retaining a footgun merely for analyzer simplicity. The language shouldn't keep UB traps just to make static analysis cheaper.

The thread does not settle one universal balance between optimization, diagnostics, portability, and convenience. Compiler builtins and libc functions may have related but not identical semantic layers. Embedded systems and boot environments complicate assumptions that address zero always traps — the discussion about address zero on embedded systems does not establish one universal hardware behavior.

The thread does NOT prove: every compiler currently accepts null zero-length calls; every C language mode already implements the C2Y rule; arbitrary invalid pointers become valid when size is zero; overlapping memcpy becomes valid; or that zero-length support makes a memory-processing pipeline safe.

This local C11 lab uses explicit guarded wrappers instead of attempting a null zero-length libc call.

Empty feature buffers are relevant to machine-learning-adjacent systems (empty batches, optional feature vectors, tensor serialization with zero rows, model-input staging) but byte-level correctness does not validate shapes, semantics, or models.

## Local observations

- Zig 0.14.0, `zig cc`, C11 mode
- `memcpy`, `memmove`, `memcmp`, `memset` callable
- Zero-length memcpy/memmove/memset/memcmp with valid pointers: no bytes modified, expected return values
- One-past pointers with zero length: arrays unchanged
- Guarded null-empty wrappers: success without libc call
- Nonzero null rejection: all 5 cases rejected before libc
- Non-overlapping memcpy lengths 0..8: all verified
- Forward overlap memmove: `ababcdef`
- Backward overlap memmove: `cdefghgh`
- memcmp signs: lengths 0,1,2 equal; 3,4 negative
- Empty slice concat: `abcd`
- Feature row copy: 0.25, -0.0, 1.5, 2.0, signbit preserved
- Empty feature batch: 0 elements, 0 bytes, no overflow

## What this repository does NOT prove

- Every current C compiler implements N3322
- Every C11/C23 implementation accepts null zero-length memory calls
- `memcpy(NULL, NULL, 0)` is safe under every compiler mode
- A non-null invalid sentinel is always valid when length is zero
- One-past pointers may be dereferenced
- Null plus a nonzero offset is valid
- Null pointers may be subtracted under every C version
- Overlapping memcpy is defined
- `memcpy` and `memmove` are interchangeable
- `memcpy` with identical pointers is covered
- `memcmp` returns only -1, 0, or 1
- The magnitude of a negative memcmp result is portable
- `memcmp` provides numeric, lexical, locale-aware, or semantic ordering
- `memset` creates a valid representation for every C type
- All-bits-zero is every type's semantic zero
- Copying object representation creates a portable serialized value
- Copying a float array validates its numerical meaning
- Preserving a negative-zero bit pattern matters to every pipeline
- An empty byte slice is semantically valid in every API
- An empty feature batch is valid for every framework
- A successful byte copy validates tensor shapes or feature meaning
- Zero-length support prevents integer overflow, use-after-free, data races
- Zero-length support makes C memory safe
- The local libc is glibc, musl, bionic, apple libc, or another specific implementation unless independently identified
- The HN thread proves UB is never useful, optimization is inherently unsafe, analyzers should accept/reject one universal warning policy, all hardware traps on address zero, or no hardware maps useful state at address zero
- The linked article proves every implementation has zero performance cost
- The lab establishes a fastest memory function
- The lab validates an ML model, dataset, tensor library, feature pipeline, or production system

## ML-adjacent context

Empty byte spans, zero-row batches, optional feature buffers, tensor serialization, feature-row copying, and model-input staging are machine-learning-adjacent data-movement patterns. This repository tests byte-level copy contracts only. It does NOT validate a model, dataset, tensor library, compiler, libc, allocator, serialization format, security boundary, or production data pipeline.

## License

MIT
