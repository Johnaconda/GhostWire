# GitHub repository publication and contribution

The A00 report records local Git initialization on develop. The A01 working tree
was extracted from the authoritative source ZIP and was not initialized as a new
repository. Archives exclude .git; users may initialize the delivered source.
The owner published https://github.com/Johnaconda/GhostWire on 2026-10-12.
The original no-remote statement in the A00 record is historical and accurate
at that time. Do not treat old checkpoint reports as current repository state.

If **starting a new independent repository** from a source archive, first
review its contents. Do not reinitialize the already published GhostWire tree:


```bash
git init -b develop
git add .
git commit -m "Import cumulative GhostWire GW-A01 source"
```

Use your existing Git author identity. For the published repository, extract
future cumulative source ZIP files over the existing root while preserving .git
and excluding local out/build and untracked user data. Review `git status` and
`git diff` before committing or pushing. Never overwrite .git or copy an
unrelated TASE origin. Only the repository owner authorizes publication. Configure main for accepted checkpoints,
develop for integration, and feature/*, fix/* and security/* for work. Tag
GW-A01-CORE-LIFECYCLE-SERVICES only after user acceptance. Enable private
vulnerability reporting and set the real URL/contact in SECURITY.md.

The supplied workflow files propose Windows, Linux and macOS Debug/Release
checks plus Linux sanitizers. Require relevant checks and review before merging
once those workflows have actually run. Do not interpret the files' presence as
successful CI or applied repository policy. Automated jobs have contents:read
only and execute PR code without deployment credentials.

## Published CI evidence (2026-10-12)

The initial GitHub Actions Windows and Linux jobs completed successfully.
The initial macOS-14 Debug/Release jobs failed compiling C++20 cancellation
(std::stop_token/std::stop_source) with AppleClang 15/libc++ and two explicit
char-to-unsigned-char conversions rejected under -Wsign-conversion.
HF02 is a proposed repair: macos-26 ARM64/Intel with supported C++20 library
and source-level conversion corrections. A green rerun is required before
macOS acceptance; publishing the workflow edit is not validation.

## A02 input update

The owner later supplied main commit `713750550d6fa7bbde7200c494554d1c4f9e008f`.
Its Linux, Windows and macOS26 ARM64/Intel jobs now have successful observed logs;
see [exact input evidence](../checkpoints/GW-A02-INPUT-EVIDENCE.md). Earlier
candidate statements above preserve the initial failure/repair sequence.
A02 preserves these repairs and the README changes. No A02 remote branch, push,
PR, run or setting change was performed. The cumulative A02 ZIP needs independent
review/native host acceptance; apply it only to the intended checkout after
reviewing local changes. Do not overwrite .git or untracked private data.
