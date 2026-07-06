# tracer_core Read-First Guide

Use this guide before editing `libs/tracer_core/**` or `apps/tracer_core_shell/**`.

## Base Read Set
Read these first for any core task:
1. `docs/time_tracer/architecture/library_dependency_map.md`
2. `docs/time_tracer/architecture/libraries/tracer_core.md`
3. `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`

Do not treat `libs/tracer_core/README.md` as the primary implementation guide.
It is only a short local index.

## Current Focus
- Wave 1 completed:
  `apps/tracer_core_shell/tests/platform/infrastructure/tests/data_query/**`
- Wave 1 completed:
  `apps/tracer_core_shell/tests/platform/infrastructure/tests/validation_issue_reporter_tests.cpp`
- Wave 1 completed:
  `libs/tracer_core/tests/infra/tests/report_formatter/report_formatter_parity_fixture_tests.cpp`
- Phase 3 completed:
  `apps/tracer_core_shell/pch.hpp`
  first shrink wave (`6` immediate removals)
- Phase 3 completed:
  `apps/tracer_core_shell/pch.hpp`
  second narrow shrink wave (`4` PCH-only removals)
- Phase 3 completed:
  `apps/tracer_core_shell/pch.hpp`
  Wave 3A manual-review shrink (`6` removals)
- Phase 3 completed:
  `apps/tracer_core_shell/pch.hpp`
  Wave 3B manual-review shrink (`3` removals)
- Phase 3 completed:
  `pch`
  closeout with retained
  `core_responses`
  boundary
- Next reassessment focus:
  `application/dto/core_responses.hpp`
- Next reassessment focus:
  only if the canonical response surface changes;
  otherwise keep current defer

## Defer By Default
- `apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/**`
- `C ABI` / `JNI` / host bridge production code
- `tracer_transport` public-header consumers

## Phase 3 Checkpoint
- PCH Wave 1 validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave1\summary.json`
- PCH Wave 2 validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave2\summary.json`
- Remaining deferred PCH candidate inventory:
  `C:\code\time_tracer\temp\pch_shrink_inventory.md`
- `core_responses.hpp`
  boundary read summary:
  `C:\code\time_tracer\temp\core_responses_phase3_boundary_read.md`
- Wave 2 PCH-only candidate read:
  `C:\code\time_tracer\temp\pch_wave2_pch_only_candidate_read.md`
- Manual-review bucket classification:
  `C:\code\time_tracer\temp\pch_manual_review_phase3_read.md`
- Wave 3B boundary-owned read:
  `C:\code\time_tracer\temp\pch_wave3b_boundary_owned_read.md`
- Phase 3 closeout summary:
  `C:\code\time_tracer\temp\pch_phase3_closeout.md`
- Wave 3B validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave3b\summary.json`
- Wave 3A validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave3a\summary.json`
- `android_runtime` test-common
  was used in the boundary read for `core_responses`,
  but is not the next patch target by itself

## Boundary Reminders
- Do not mechanically replace every `#include` with `import`.
- Do not push stable boundaries onto pure `import` paths just because a module exists.
- `tracer_transport` public headers remain canonical API, not compat debt.
- `C ABI`, `JNI`, and host bridge paths still prefer explicit stable boundaries.

## Extra Contract Reads
- C ABI / shell runtime:
  `docs/time_tracer/core/contracts/c_abi.md`
- Android runtime / JNI:
  `docs/time_tracer/clients/android_ui/runtime-protocol.md`
- Stats / report / chart semantics:
  `docs/time_tracer/core/contracts/stats/README.md`
  and
  `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md`
- Transport envelope / codec behavior:
  `docs/time_tracer/architecture/libraries/tracer_transport.md`

## Validation Default
- Docs-only changes:
  skip build/test by default
- Core code changes:
  `python tools/run.py validate --plan <plan> --paths-file <paths>`
- Shell/runtime integration touched:
  `python tools/run.py verify --app tracer_core_shell --concise`
- Android host/runtime touched:
  `python tools/run.py build --app tracer_android --profile android_edit`
