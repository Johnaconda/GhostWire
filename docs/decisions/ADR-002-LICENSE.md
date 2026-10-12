# ADR-002: MIT license for project-owned A00 source

Status: chosen under the contract's authority to select a license.

MIT is selected for a simple permissive foundation that independently operated
clients and servers can reuse. Retain copyright and permission notices.
The project uses the contributor collective attribution “GhostWire contributors”;
this does not transfer copyright ownership or invent a legal company.

Apache-2.0 would add an explicit contributor patent license and related terms,
with more notice handling. MPL-2.0 uses file-level source-sharing obligations
when covered code is distributed, while permitting larger mixed-license works.
MIT has no equivalent explicit patent grant or copyleft requirement. These
trade-offs are recorded, not hidden behind an unspecified placeholder license.

Doctest remains under its own included MIT notice. License approval of each
future crypto/UI/media dependency is a separate gate. In particular, the
reviewed libsignal repository's AGPL-3.0 license is not treated as permissive
merely because GhostWire's own files use MIT. Adoption, distribution and any
license-policy changes require an explicit documented review.

Primary texts are linked in TECHNOLOGY_DECISIONS.md. This is a project selection
record, not a comprehensive interpretation of every distribution scenario.
