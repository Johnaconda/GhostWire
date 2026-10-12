# ADR-001: C++20 host language

Status: chosen for A00. Date: 2026-10-11.

| Option | Safety | Tooling / portability | Decision |
|---|---|---|---|
| C++20 | RAII and types help; use-after-free/data races still possible | Matches user MSVC; GCC/Clang available; broad native libraries | Use now; sanitizers, narrow interfaces and bounded parsers required |
| C++23 | Incremental library improvements, not general memory safety | Could simplify expected/result later; standard-library support must be tested | Defer until all supported compilers justify migration |
| Rust | Ownership/borrowing reduce many memory hazards; unsafe/FFI still need review | Adds Cargo, rustc and cross-language deployment knowledge | No A00 runtime dependency; reassess for concrete security library need |
| C++ with Rust libraries | Host UI/tooling stay C++; reviewed crypto can remain in upstream language | Requires stable narrow C ABI, allocator/lifetime/error/cancellation rules, pinned Rust build | Acceptable later with evidence; never invent weaker crypto to avoid it |

A00 Result is a small variant-based error carrier, not a homegrown crypto or
serialization framework. No arbitrary raw owning pointers; exceptions are
caught at the executable boundary without printing potentially private text.
Prefer owned values and RAII. std::stop_source/token supplies cancellation.
A01 must design resource ownership before adding asynchronous services.
