# Future feature proposals after A00

Captured from the owner's A01 directive on 2026-10-11. **All proposals here are
unimplemented.** This backlog neither renumbers A00–A25 nor authorizes an extra
checkpoint. It records requirements to evaluate when their existing prerequisite
gates are ready. No new runtime dependency or optional-language toolchain lands
in A01. Scope and acceptance criteria must be approved before implementation.

| ID | Proposal | Intended checkpoint alignment | Required boundaries and future acceptance |
|---|---|---|---|
| FUT-WEB-01 | Local browser gateway with ComfyUI-style node/frontend separation; friends, chat and administration | A13/A18; explicit platform and UI feasibility decision | C++ node remains authoritative and headless. Authenticated local sessions; explicit bind policy; origin/CSRF/DNS-rebinding checks; scoped commands; no assumption that loopback is trusted. Optional remote access requires authenticated protected transport and explicit enablement. AUTH-PRE before a listener. |
| FUT-AI-01 | Optional native inference with llama.cpp/GGUF as initial candidates | A20 after AUTH-PRE and required E2EE membership integration | Model/backend/version/format compatibility matrix; capability checks for CPU, CUDA, Vulkan and Metal on actual hardware; explicit resource reservations and cancellation; no required GPU or cloud dependency. Backend choice and pins are future decisions. |
| FUT-HF-01 | Hugging Face discovery/import and custom runtime adapters | A20/A22 | Validate provenance, model license, format, architecture, tokenizer and backend support before loading. No promise that every model runs in one backend. Custom/untrusted model code requires isolated execution, scoped filesystem/network permissions and explicit consent; never run it in the trusted core. |
| FUT-TRAIN-01 | Optional training studio, retrieval memory, LoRA/QLoRA, scheduled workers, checkpoints and evaluation | A20/A22/A23; scoped follow-up proposal needed | Python environments and training packages remain optional and separate from C++ core. Retrieval memory is distinct from weight training. Explicit data provenance/consent, limits, cancellable jobs, checkpoint integrity/recovery and evaluation before promotion. No automatic training on private chat history. |
| FUT-AUTH-01 | AI identities and per-action authority | AUTH-PRE/A14/A20 | Separate grants for chat reading, replies, history, tools and files; explicit E2EE participant authorization; expiry/revocation; tool authority cannot be granted by prompt text. Remote inference/training disclosure must be authorized, and local models receive no implicit blanket permission. |
| FUT-MEDIA-01 | Advanced camera/microphone/audio/screenshare, codec/bitrate and GPU controls | A19 after A06/A14/A18 | Consent, device/mute/capture state, encoder capability, bounded buffers, bandwidth/VRAM admission and measured consumer-hardware behavior. Every media route obeys protected/privacy-first policy; no silent ICE/direct fallback or diagnostic endpoint leakage. |
| FUT-RESOURCE-01 | Shared CPU/RAM/GPU budgets for inference, training and media | A19/A20 as real workloads arrive | Extend A01 accounting with backend-specific reservations and scheduling. Measure fairness, cancellation, VRAM fragmentation and CPU headroom. Admission is not proof of OS or GPU isolation; hardware details remain local by default. |

The existing A11-PRE protected-store and AUTH-PRE permission gates remain
unchanged. Browser access and AI training do not bypass vault, identity,
transport, privacy or licensing requirements. A01 only supplies lifecycle,
cancellation, structural diagnostics and resource-admission foundations.
