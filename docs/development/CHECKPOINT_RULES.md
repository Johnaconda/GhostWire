# Checkpoint rules

A checkpoint has one authoritative cumulative input, one scoped output and a
recorded identifier. A00 is clean-room; later inputs are supplied source archives
or the exact accepted repository state, never remembered reconstructions.

Sequence: inspect input → document risks/choices → implement scoped functionality
→ validate actual available environments → update work/decision records → package
cumulative source and checksum → report gaps → await user acceptance.

The source ZIP contains one GhostWire root plus an embedded source hash
inventory. The external SHA-256 cannot be embedded into the archive it hashes;
provide it in the delivery response and sidecar instead. Each package includes
build instructions, validation report, changed file list and next prompt.

State axes: implementation (planned/partial/implemented), validation (not run,
pass/fail/blocked with environment), and design/user acceptance (proposed/accepted)
are distinct. Configured GitHub Actions do not count as executed validation.
A00 prepared CI, but no remote was created or pushed.

main stores accepted checkpoints. develop holds integration; feature/*, fix/*
and security/* branches carry reviewable changes. Recommend protected branches
and required three-OS checks after authorized repository creation. Do not mark
A00 accepted or create its acceptance tag before owner sign-off.
