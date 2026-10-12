# GitHub preparation (not executed remotely)

The A00 report records local Git initialization on develop. The A01 working tree
was extracted from the authoritative source ZIP and was not initialized as a new
repository. Archives exclude .git; users may initialize the delivered source.
No remote repository, issue, PR, branch protection or release was created.

After reviewing the source and selecting the destination repository yourself:

```bash
git init -b develop
git add .
git commit -m "Import cumulative GhostWire GW-A01 source"
```

Use your existing Git author identity. Before any push, verify the destination
and obtain owner authorization. Connect the chosen remote only then; do not
copy an unrelated TASE origin. Configure main for accepted checkpoints,
develop for integration, and feature/*, fix/* and security/* for work. Tag
GW-A01-CORE-LIFECYCLE-SERVICES only after user acceptance. Enable private
vulnerability reporting and set the real URL/contact in SECURITY.md.

The supplied workflow files propose Windows, Linux and macOS Debug/Release
checks plus Linux sanitizers. Require relevant checks and review before merging
once those workflows have actually run. Do not interpret the files' presence as
successful CI or applied repository policy. Automated jobs have contents:read
only and execute PR code without deployment credentials.
