# A03-HF01 acceptance and A04 continuation

Status: recovery candidate. A04 implementation has not started. The owner asked
to continue after supplying A03 Windows acceptance and pushing main; this pass
first repairs the macOS failures discovered in that push. Do not require the
owner to repeat their already-proven A03 acceptance. Validate the changed HF01
source separately and retain its exact checksum before advancing.

## Recovery acceptance

Use the full GHOSTWIRE-GW-A03-HF01-MACOS-RECOVERY-source.zip and its external
SHA-256 sidecar. Verify CRC and every embedded source hash; replace the source
folder as usual and use fresh build caches. GitHub pushes remain owner-controlled.

Windows 10 / VS 2026 / ordinary PowerShell, with original TEMP/TMP:

```powershell
Set-Location 'C:\Cumpiling-shit\GhostWire'
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Debug -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Debug validation failed' }
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\windows\dev-windows.ps1 -Action all -Configuration Release -OfflineTests -Fresh
if ($LASTEXITCODE -ne 0) { throw 'Release validation failed' }
.\out\build\portable-release\bin\ghostwire-cli.exe --version
if ($LASTEXITCODE -ne 0) { throw 'CLI launch failed' }
```

Expected Windows registrations remain **37** per configuration; the new process
case runs inside platform-unit. Version must be **0.3.1 / GW-A03-HF01-MACOS-RECOVERY**.
These are acceptance expectations, not recorded HF01 Windows results.

On macOS 26 Intel and ARM64, repeat the existing pinned-vcpkg workflow matrix or
run the preserved macos-debug / macos-release presets with the pinned VCPKG_ROOT.
The portable route is also preserved:

```bash
cmake --preset portable-debug --fresh
cmake --build --preset portable-debug -j1
ctest --preset portable-debug --verbose
cmake --preset portable-release --fresh
cmake --build --preset portable-release -j1
ctest --preset portable-release --verbose
```

Acceptance checklist:

- [ ] macOS ARM64 Debug and Release: **37/37** each, no skips or suppression.
- [ ] macOS Intel Debug and Release: **37/37** each, no skips or suppression.
- [ ] platform-unit: completed-child cleanup and all prior native cases pass.
- [ ] crypto-unit and crypto-symlink-temp both pass under original TEMP.
- [ ] Descendant cleanup, cancellation, reaping, concurrent waits and foreign FD isolation pass.
- [ ] Windows Debug and Release: **37/37** each under original TEMP/TMP.
- [ ] Native source review: failed/incomplete group inspection and live EPERM remain errors.
- [ ] Record exact source checksum/commit, compiler, OS and logs; accept or report failures.

No source push, workflow dispatch, tag or merge was performed by the agent.
Do not mark this candidate's native checks passed from the previous A03 commit.
Owner-local Windows vcpkg, local full LSan, clang-tidy and independent crypto review
remain separate. A03's default hosted sanitizer run is recorded as its own evidence.

## A04 first pass after recovery review

Continue using [GW-A04-PROMPT.md](GW-A04-PROMPT.md); its historical bytes are
preserved. This supplement updates the authoritative input and acceptance state.
Read the current owner identity discussion and produce a concrete identity ADR
for review before implementing identity behavior, as that prompt requires.

Carry these owner requirements forward:

- Multiple isolated local profiles. One profile's use must not expose another.
- Separate keys/authorization for joined servers; a copied public ID cannot authenticate.
- Two legitimate users displaying the same short ID remain separate people with
  isolated memberships, chats, permissions and visible data.
- Human aliases, full cryptographic identity and server membership are separate concepts.

Root/device separation is required by the master contract. The precise per-server
credential roles, enrollment proof, root rotation/recovery, alias policy and trust
record semantics still need design review. The discussion of two independent
proof/signature keys is a requirement to resolve, not approval of an invented
authentication protocol or a claim that two keys alone provide two-factor security.

Propose public canonical records, bounded validation, domain separation, versioning,
expiry and revocation policy, explicit authorization and volatile ownership.
Keep profile isolation and collision tests central. No implicit identity creation
during ordinary diagnostic startup. A raw valid signature does not grant device
or community authority. Botan sign_public remains public-message-only.

A11-PRE blocks persistent profiles, credentials, private trust/session state and
keys until a separately authorized protected-store design and implementation is
accepted. AUTH-PRE blocks relevant listeners/actions. Do not introduce networking,
GUI, discovery, messaging, AI, remote enrollment or A05 functionality in the design
pass. Resolve owner design choices with a concrete reviewable proposal; do not
ask again about requirements already stated. Stop at the authorized checkpoint.
