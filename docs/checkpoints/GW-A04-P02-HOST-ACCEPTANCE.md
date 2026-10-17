# P02 Windows host evidence and continuation

Received 2026-10-16. The owner supplied the PowerShell build/test transcript and
instructed “we can continue.” This authorizes the bounded P03 review/hardening
proposal. It does not authorize networking, persistence or automatic A05 work.

Input source: delivered P02 cumulative archive, also uploaded with the suffix
`-source-astra.zip`; exact SHA-256 verified before fresh P03 extraction:

```
e755c9b994541dbf66e061d799b31621b2f91034cda7741d1473699426783b59
```

| Evidence | Actual observation |
|---|---|
| Host | Owner's Windows 10 x64 / Visual Studio 2026 Community; transcript identifies MSVC 19.51.36260.0, toolset 14.51.36231, HostX64/x64 |
| Root | `C:\Cumpiling-shit\GhostWire` |
| Dependency mode | Portable `-OfflineTests`; pinned static Botan 3.13.0 Windows subset |
| Debug | `-Action all -Configuration Debug -OfflineTests -Fresh`; configure/generate/build successful; **40/40 CTest passed**, 5.72 s |
| Release | Corresponding Release command; configure/generate/build successful; **40/40 CTest passed**, 5.11 s |
| Regressions | Identity unit/fault/session, Unicode paths and Windows tooling all pass in both runs |
| Interactive launch | Release `--identity-session` prints **GW-A04-P02-IDENTITY-DEVICE-TRUST** startup and volatile-session banner |
| Manual interaction | Transcript ends during help-prompt text; no manually entered command workflow or clean quit is evidenced |
| TEMP/TMP | No override appears in the submitted commands; environment values were not independently captured |
| Screenshot | Shows an older **GW-A03-HF01-MACOS-RECOVERY** IDE bootstrap exiting zero; not P02 execution evidence |

The Windows pthread feature-probe failures are followed by `Found Threads: TRUE`
and successful compilation/testing; they are not failed application tests.
No separate Windows unit-case/assertion totals are inferred from CTest counts.
No P02 hosted/macOS, vcpkg, LeakSanitizer, clang-tidy or external security-audit
result is established by this transcript. Prior historical records remain intact.

Attachment provenance (files remain owner evidence, not source dependencies):

- `Pasted text(10).txt` SHA-256: `9faac38bfeaeaaaec53af68a071e960620d35786b1b89d3b842c96771d18ec5f`
- `image(20261016-151411).png` SHA-256: `fa145bebcc714043fd4a8d7821e3f8c349335c500b1f5f0304410425063f9e03`

P03 current results and native retest instructions are recorded separately.
