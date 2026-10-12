# GW-A01-HF02 — GitHub publication and macOS CI recovery (candidate)

**Status: proposed patch, not macOS-validated or owner-accepted.**
Checkpoint: `GW-A01-HF02-MACOS-CI-RECOVERY`. Version: `0.1.2`.
Date: 2026-10-12. Accepted input: HF01 source ZIP. This is a narrowly scoped
repository/portability repair; A02 is separately authorized but **not**
implemented here. All A00/A01/HF01 service/lifecycle/privacy gates remain.

## Observed evidence and actual failure

- Owner native Windows HF01 Debug: 34/34 CTest passed, 3.37s.
- Owner native Windows HF01 Release: 34/34 CTest passed, 3.23s.
- Hosted initial Linux/Windows GitHub Actions on `main`: success.
- Hosted initial macOS `macos-14` Debug and Release: failed in compile, **not**
  a failed runtime/CTest run. AppleClang 15 on arm64 lacked `std::stop_token`
  and `std::stop_source` in the selected C++20 libc++; `-Werror` rejected
  implicit `char` -> `unsigned char` conversion in `config.cpp` and `bootstrap.cpp`;
  local strict Clang 17 also identified the same warning in `logger.cpp`.
  Initial failed run: https://github.com/Johnaconda/GhostWire/actions/runs/38251672951

## Scoped changes

1. Change the macOS workflow to the supported `macos-26` and
   `macos-26-intel` runner images (ARM64/Intel, Debug/Release). Newer Xcode
   provides nonexperimental C++20 cancellation library types. Add an actual
   compile-and-run `std::stop_source` check before the full build, show Xcode
   identity, and use the same explicit Apple clang path for CMake. No disabling
   warning-as-error, dropping C++20, or custom cancellation mechanism.
2. Explicitly convert each char byte to unsigned char in the control-character
   validation loops and the logger JSON escaping loop. Existing error behavior
   and redaction remain unchanged.
3. Bump checkpoint/version and corresponding unit/integration expectations and
   deterministic packaging metadata; preserve the old HF01 package and reports.
4. Update root README, developer/roadmap/TODO status and GitHub setup guidance.
   Add the actual owner Windows acceptance record rather than editing earlier
   historical evidence as if it occurred during implementation.

## Local validation of the HF02 candidate

Executor: Linux x86_64, GCC 14.2, Clang 17 (Swift LLVM toolchain),
CMake/Ninja; vendored doctest only. These runs are **not macOS** builds.

| Check | Observed result |
|---|---|
| GCC 14.2 Debug configure/build/CTest | PASS, 33/33 |
| GCC 14.2 Release configure/build/CTest | PASS, 33/33 |
| Clang 17 Debug configure/build/CTest with -Werror | PASS, 33/33 |
| Architecture boundary script | PASS |
| GitHub Actions macOS 26 ARM64/Intel Debug/Release | NOT RUN; requires push/PR |
| Windows/MSVC HF02 rerun | NOT RUN; HF01 owner evidence preserved separately |
| Clang Release | NOT COMPLETED |
| clang-format check | BLOCKED (formatter executable not installed in local environment) |
| macOS workflow syntax | YAML parsed locally; does not prove Actions execution |
| ZIP internal CRC and SHA-256 source inventory | PASS at packaging |

Current errors under local strict Clang were three sign-conversion warnings
which have been corrected without suppressions; tests passed afterward.

## Acceptance gates

- Package from the full HF01 accepted source tree and verify all source hashes.
- Linux GCC and Clang Debug/Release CTest, source format and architecture check:
  record actual results separately before accepting this candidate.
- GitHub Actions must build/test **both** macOS ARM64 and Intel, Debug and Release,
  with compiler warnings-as-errors; no ``-fexperimental-library`` workaround.
- Rerun Windows/Linux hosted CI after publication; require no regressions.
- Owner may rerun Windows Debug/Release to verify the new CLI checkpoint value.
- Windows vcpkg on the owner's host, full LSan, clang-tidy and independent
  macOS physical-host acceptance remain separate gates.

None of these gates may be marked passed from a drafted workflow alone.
No UI, crypto, identity system, networking, directory or MainNet behavior added.

## Toolchain rationale

Apple's Xcode 26 release notes explicitly list `std::jthread` and `<stop_token>`
as no longer gated behind `-fexperimental-library`. GitHub supports the
`macos-26` (ARM64) and `macos-26-intel` (x64) runner labels. References:

- https://developer.apple.com/documentation/Xcode-Release-Notes/xcode-26-release-notes
- https://github.blog/changelog/2026-02-26-macos-26-is-now-generally-available-for-github-hosted-runners/

Deployment/runtime behavior on older macOS releases remains unverified.
