# apps/tracer_core

`apps/tracer_core` is the core implementation and wiring entry (source code lives in `src/`).

This README is a navigation entry only.  
Specifications and contracts are maintained under `docs/` to avoid duplicate maintenance and drift.

## Authoritative Entry Docs
1. Top-level docs: `docs/time_tracer/README.md`
2. Core docs domain: `docs/time_tracer/core/README.md`
3. Core agent onboarding: `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`
4. Contracts index: `docs/time_tracer/core/contracts/README.md`
5. C ABI: `docs/time_tracer/core/contracts/c_abi.md`
6. Local agent entry: `apps/tracer_core/agent.md`

## Conventions
1. For core contract/spec changes, update `docs/time_tracer/core/*` first.
2. `apps/*/README.md` should remain index-only and not host cross-app spec details.
3. Platform-specific notes belong to each platform docs domain, not core specs.
