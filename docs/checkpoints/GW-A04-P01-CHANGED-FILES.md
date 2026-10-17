# GW-A04-P01 changed-file inventory

Compared with the accepted HF01 source ZIP, SHA-256
`5d203d63341deeb14f2128e86bb8cf5773c19c7084ef6a83caefdc273c1747a8`,
and exact main `1f92c5539f7eb3f6f8e3833f27df9384f260251a`.

15 modified, 8 added, 151 unchanged, 0 removed.
The generated SOURCE_MANIFEST.json is additional packaging metadata.

## Modified

- AGENTS.md
- ARCHITECTURE.md
- BUILD.md
- README.md
- ROADMAP.md
- TODO.md
- docs/CHANGELOG.md
- docs/COMPLETED.md
- docs/architecture/DEPENDENCY_GRAPH.md
- docs/architecture/FUTURE_INTERFACES.md
- docs/architecture/MODULE_MAP.md
- docs/development/VALIDATION.md
- docs/security/SECURITY_INVARIANTS.md
- docs/security/THREAT_MODEL.md
- tools/packaging/package_source.py

## Added

- docs/architecture/IDENTITY_DESIGN.md
- docs/checkpoints/GW-A03-HF01-ACCEPTANCE-EVIDENCE.json
- docs/checkpoints/GW-A03-HF01-ACCEPTANCE.md
- docs/checkpoints/GW-A04-P01-CHANGED-FILES.md
- docs/checkpoints/GW-A04-P01.md
- docs/checkpoints/GW-A04-P02-PROMPT.md
- docs/checkpoints/GW-A04-TEST-PLAN.md
- docs/decisions/ADR-008-IDENTITY-AND-DEVICE-TRUST.md

## Preservation

All runtime headers/sources, executable/build inputs, dependencies, presets,
compiler settings, workflows and tests are byte-identical to the accepted HF01
source. MASTER_CONTRACT, existing decisions and all prior checkpoint documents
are unchanged. The only tool change is packager output/manifest/date metadata.
No identity code, private storage, network feature or new runtime version.
