# GhostWire provisional effort forecast and Windows 10% milestone gates

Based on the *complete currently agreed A00–A25 roadmap*, not checkpoint counting.

- 26 functional modules are weighted in `PROJECT_EFFORT_FORECAST.json` using an
  explicit, subjective **110 point** engineering effort model. Large future modules
  (vault, E2EE, server, desktop, voice/video, UI, release) receive more points
  than early scaffolding. This is **not** a duration estimate or a promise of future velocity.
- A00–A04 are verified and owner accepted: 11/110 = **10.0% verified weighted
  floor** under this model. A05 remains partial: no provisional fractional credit.
  Historical milestone number and 5/26 counts are not functional completion.
- The official registry completion is **null** until the weight model and functional
  acceptance criteria are explicitly agreed. The calculator refuses to write
  official completion, change runtime, or authorize transport or persistence.
- Owner-operated Windows checks are due when the verified, owner-accepted weighted
  floor first reaches each **10%, 20%, … 100%** gate, with additional security
  gates whenever cryptography, persistence, network authorization or migration
  introduces new risk. One full cumulative ZIP per gate, but incremental
  coordinator-driven source/CI tests before each gate.
- The **10% native gate is due now** against this cumulative package. Linux checks
  do not replace it; the Windows owner is the acceptance authority.
- The next **20%** requires another 11 verified/accepted effort points. Under
  the present planning weights, A05+A06+A07 would bring the floor to 24/110
  (~21.8%) *if fully implemented, securely reviewed and owner accepted*. This
  is a possible milestone sequence, **not authorization** to implement A06/A07.
- Work is gated by functionality and evidence, not elapsed time, pass count or
  engineering confidence. A05's independent parser security review remains open.
- A calendar ETA cannot be estimated responsibly until real throughput is measured
  across several accepted development gates.

Use `python tools/development/effort_forecast.py` for the provisional report and
`python tools/development/module_registry.py --json` for authoritative task readiness.
