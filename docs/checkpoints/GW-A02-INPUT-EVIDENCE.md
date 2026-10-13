# A02 authoritative inputs and prior hosted evidence

Observed 2026-10-12. These results describe the input, not execution of A02.

## Accepted archive

The owner's accepted `GHOSTWIRE-GW-A01-HF01-WINDOWS-UNICODE-RECOVERY-source(1).zip`
was materialized and its SHA-256 checked before implementation:

`751583206efef4170b9d28addf6c6d0f203b466c2fcb9f3aff33ac36c5da2c41`

Archive CRC, safe entry paths and all 106 embedded source hashes passed. Untouched
HF01 portable Debug configured/built and passed 33/33 CTest on this Linux executor.
The owner separately accepted Windows 10 x64/VS2026 portable Debug/Release,
34/34 each, with original TEMP/TMP, dedicated Unicode/tooling tests and CLI exit.
The GitHub-supplied [host record](GW-A01-HF01-HOST-ACCEPTANCE.md) preserves that
evidence, including its more detailed reported short-form temp path spelling.

## Additional repository supplied during A02

The owner subsequently supplied https://github.com/Johnaconda/GhostWire and
authorized continuation from its updated copy. Read-only inspection pinned:

- Commit: `713750550d6fa7bbde7200c494554d1c4f9e008f` (main at inspection).
- Git tree: `7913ddbd8a7e8d8cd559c2a6217b8da335ace6d3`.
- The full recursive tree was not truncated; all file bytes were checked against
  Git blob hashes. Unchanged files matched the verified archive. Nineteen added
  or modified files were retrieved exactly; no source was reconstructed from memory.
- The tree adds HF02's macOS 26 ARM64/Intel workflow, three explicit unsigned-byte
  conversions, version metadata and historical documentation, plus owner README
  branding/badges. These changes are preserved in A02; A02 version/metadata advances
  to 0.2.0. The source manifest, absent from GitHub, is regenerated in the ZIP.
- No remote file, branch, PR, workflow or setting was changed or triggered.

## Hosted jobs actually inspected at that commit

Run status, job/step outcomes and decoded job logs were retrieved. Exact IDs and
concise observed log evidence are in [GW-A02-INPUT-CI.json](GW-A02-INPUT-CI.json).

| Input run | Host / compiler | Actual result |
|---|---|---|
| [Linux 38254708287](https://github.com/Johnaconda/GhostWire/actions/runs/38254708287) | ubuntu-24.04, GNU 13.3.0 | vcpkg Debug and Release: 33/33 CTest each; default portable ASan/UBSan workflow: 33/33 |
| [Windows 38254708270](https://github.com/Johnaconda/GhostWire/actions/runs/38254708270) | hosted Windows, MSVC 19.44.35229.0 | vcpkg Debug and Release: 34/34 CTest each |
| [macOS 38254708593](https://github.com/Johnaconda/GhostWire/actions/runs/38254708593) | macos-26 ARM64 and macos-26-intel x64; AppleClang 21.0.0.21000101 | vcpkg Debug and Release on each architecture: all four jobs 33/33 CTest |

Thus HF02's earlier report saying a hosted rerun was pending is historical; a
successful rerun is now directly evidenced for this commit. Its default sanitizer
workflow contains no leak-disable setting. This is useful prior evidence, but it
does not establish full A02 sanitizer/native acceptance. No separate standalone
leak audit is inferred from a workflow summary.

The owner's Windows 10/MSVC19.51 vcpkg/doctest issue remains unresolved: successful
MSVC19.44 hosted jobs are a different environment. A02 introduces new native
process/stop/capability implementations, so new Windows/macOS runs, A02 hosted CI,
clang-tidy and independent owner acceptance remain outstanding. A02 local results
are in [GW-A02.md](GW-A02.md). Earlier checkpoint records are preserved verbatim.
