# Security boundary map

These are intended authority boundaries. Only the A00 bootstrap and A01 core rows exist at runtime.
A process boundary is stronger isolation than a C++ module; modules in a single
compromised process cannot protect secrets from one another by convention alone.

| Component | Plaintext | Private keys | Endpoints | Persistent records |
|---|---|---|---|---|
| A00 bootstrap | Nonsecret config and local CLI arguments only | None | None used | Empty app directories; explicit config reads only |
| A01 lifecycle/admission | Immutable nonsecret settings, local paths, bounded structural diagnostics | None | None used | No additional records; the owned paths service preserves A00 directory behavior |
| Future unlocked client messaging/vault | Authorized user's selected chats | Scoped key handles; vault owns wrapping material | Via policy facade only | Encrypted history, indexes, ratchet state |
| Future identity/crypto service | Only requested crypto inputs | Root/device/key-handle operations with separate lifecycles | No reason to receive routing endpoints | Protected key envelopes, no raw key exports by default |
| Future UI | Authorized view model while unlocked | No exportable root key or generic vault handle | Only explicit diagnostics/invites as policy permits | Preferences; private drafts through vault |
| Future community host | Public channels or content deliberately provided to it | Its own community/service keys | Connecting endpoints visible to its network stack | Server policy and bounded encrypted envelopes |
| Future relay | No payload plaintext | Its own transport/service keys, not participant E2EE keys | Ingress/egress hop addresses and timing | Minimal operational records, encrypted/access-controlled if needed |
| Future discovery node | Signed, bounded reachability metadata as authorized | Own service key only | Policy-approved reachability, never private direct endpoints in protected records | Expiring records with quotas |
| Future mailbox/storage node | No message/file plaintext | Own service credentials only | Its neighbors/clients | Ciphertext envelopes/chunks with TTL and quota |
| Future local AI | Explicitly granted context | Its own device/agent membership keys if needed | Local provider connection details | Only permitted context/cache through protected storage |
| Future remote AI provider | Explicitly disclosed authorized context | No user's root/vault key | Client/relay endpoint as routing permits | External retention policy must be disclosed; not controlled by GhostWire |
| Future plugins | Scoped capability input only | No raw application key store | Policy-mediated only | Quota-limited namespace; process sandbox where needed |
| Future diagnostics | Redacted aggregates by default | Never | Redacted by default | No automatic raw dumps or telemetry |

A host operator is not automatically an E2EE member. Public server-readable
channels and private E2EE channels must be visibly distinguishable. Account
root identity, device authorization, community identity, alias and performance
capability discovery are independent objects.

A relay conceals a peer's direct endpoint from the other peer only if every
related subsystem obeys the route policy. The relay and ISP still observe
metadata. Correlation across colluding relays or a global observer is not
solved by naming a mode “privacy-first”. That future mode stays unavailable
until a concrete implementation and leak tests justify its guarantee.
