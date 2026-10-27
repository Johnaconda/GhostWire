# Proposal only — GW-A05-P03 security review and codec interoperability

**NOT AUTHORIZED by this document.** Requires owner acceptance of A05-P02.
Do not start networking, remote login, vault, messaging, AI, discovery or GUI.

Use the exact accepted P02 source ZIP as the authoritative input, validate ZIP
manifest/hash and compare against GitHub current HEAD before modifying anything.
Audit the minimal CBOR parser in ADR-010 against a pinned maintained external
CBOR implementation, verify QCBOR/TinyCBOR licenses, supported profiles,
allocation and recursion limits, and independently exercise all frozen positive
and negative corpus data on Windows/MSVC, Linux/GCC+Clang and macOS/Clang.
Resolve implementation exceptions by independent review, maintained dependency
migration if warranted, fault allocation tests and libFuzzer/ASan/UBSan corpus
coverage; no unaudited parser network exposure. Tighten deterministic behavior,
no raw errors, deadlines, budget ownership, and A04 signed-carrier fidelity.
Do not alter A04 key/signature/trust semantics. Document findings, exact test
results, residual gates, known vulnerabilities and stop for independent review.
