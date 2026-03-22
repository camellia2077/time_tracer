# tracer_core Handoff

Use this document when a new agent takes over work in `libs/tracer_core/**`
or `apps/tracer_core_shell/**`.

## Current Snapshot
- Snapshot date:
  `2026-03-16`
- Current status:
  `Phase 3`
  closeout is complete;
  `pch`
  shrink line now ends with retained
  `core_responses`
  boundary

## Scope
- Goal:
  expand remaining safe
  `direct module consumption`
  after the earlier module-boundary closeout
- Non-goal:
  reopen ecosystem compat-removal cleanup
- Non-goal:
  force every header consumer onto `import`

## Current Judgment
- Main direct-module candidates now live in
  shell/core test `TU`s,
  not in production runtime boundaries
- `pch.hpp`
  first shrink wave is now complete:
  `6` immediate project headers were removed
  and focused validate passed
- `pch.hpp`
  second narrow shrink wave is now complete:
  `logger`, `diagnostics`, `report_types`, and `date_check_mode`
  were removed from PCH
- `core_responses.hpp`
  remains deferred after dedicated boundary read
- manual-review bucket read/classification
  is now complete:
  `9` headers were reclassified as
  `PCH-only shrink candidates`
- `pch Wave 3A`
  is now complete:
  `6` shell-PCH-only leftovers
  were removed from PCH
- remaining manual-review slice is now:
  empty
- `pch Wave 3B`
  is now complete:
  the remaining `3` boundary-owned headers were removed from PCH
- remaining shrink-candidate set is now:
  `application/dto/core_responses.hpp`
- `Phase 3`
  closeout is now complete:
  `done with retained core_responses boundary`
- `android_runtime` test/common
  and transport public-header consumers
  still stay deferred by default
- `tracer_transport` public headers remain canonical API
- `C ABI`, `JNI`, and host bridge paths remain stable-boundary-first

## Default Next Step
- no default
  `pch`
  code patch
- if this line is reopened,
  start with
  `core_responses`
  canonical surface reassessment

### Narrow target set
- `temp/core_responses_phase3_boundary_read.md`
- `temp/pch_phase3_closeout.md`
- `temp/pch_shrink_inventory_phase3_triage.md`
- `temp/pch_wave2_pch_only_candidate_read.md`
- `temp/pch_manual_review_phase3_read.md`
- `temp/pch_wave3b_boundary_owned_read.md`
- `apps/tracer_core_shell/pch.hpp`
- `temp/pch_shrink_inventory.md`

### Phase 3 Checkpoint
- `apps/tracer_core_shell/pch.hpp`
  first shrink wave completed
- `apps/tracer_core_shell/pch.hpp`
  second narrow shrink wave completed
- `apps/tracer_core_shell/pch.hpp`
  Wave 3A manual-review shrink completed
- `core_responses.hpp`
  boundary read completed and remains deferred
- manual-review bucket read completed:
  `C:\code\time_tracer\temp\pch_manual_review_phase3_read.md`
- Wave 3B read completed:
  `C:\code\time_tracer\temp\pch_wave3b_boundary_owned_read.md`
- Phase 3 closeout summary:
  `C:\code\time_tracer\temp\pch_phase3_closeout.md`
- Wave 3B validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave3b\summary.json`
- Wave 3A validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave3a\summary.json`
- Wave 2 validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave2\summary.json`
- validate summary:
  `C:\code\time_tracer\out\validate\pch_shrink_wave1\summary.json`
- remaining PCH shrink candidate inventory:
  `C:\code\time_tracer\temp\pch_shrink_inventory.md`

## Completed In Wave 1
- `apps/tracer_core_shell/tests/platform/infrastructure/tests/data_query/**`
  now consumes
  `tracer.core.infrastructure.query.data.*`
  module surfaces
- `apps/tracer_core_shell/tests/platform/infrastructure/tests/validation_issue_reporter_tests.cpp`
  now consumes
  `tracer.core.infrastructure.logging`
- `libs/tracer_core/tests/infra/tests/report_formatter/report_formatter_parity_fixture_tests.cpp`
  now consumes
  `tracer.core.infrastructure.config.loader`
  and
  `tracer.core.infrastructure.reports.dto`

## Do Not Pull Into Wave 1
- `apps/tracer_core_shell/tests/platform/infrastructure/tests/android_runtime/**`
- `apps/tracer_core_shell/api/c_api/**`
- `apps/tracer_core_shell/api/android_jni/**`
- `apps/tracer_core_shell/host/**`
- any `tracer/transport/*.hpp` consumer that exists because of transport public contract ownership

## Mandatory Read Order
Before editing:
1. `docs/time_tracer/architecture/library_dependency_map.md`
2. `docs/time_tracer/architecture/libraries/tracer_core.md`
3. `docs/time_tracer/core/specs/AGENT_ONBOARDING.md`
4. `.agent/guides/docs/tracer_core-read-first.md`

Read extra contract docs only when the touched path crosses that boundary:
- `docs/time_tracer/core/contracts/c_abi.md`
- `docs/time_tracer/core/contracts/stats/README.md`
- `docs/time_tracer/core/contracts/stats/report_chart_contract_v1.md`
- `docs/time_tracer/clients/android_ui/runtime-protocol.md`
- `docs/time_tracer/architecture/libraries/tracer_transport.md`

## Working Rules
- Keep patches narrow.
- Prefer `.cpp` / test `TU` consumer cutover before touching helper headers.
- Do not mix behavior changes into import-expansion patches.
- If a path still wants a stable boundary, keep the boundary; do not “win” by deleting it.

## Validation Default
- Docs-only changes:
  skip build/test by default
- Core code changes:
  `python tools/run.py validate --plan <plan> --paths-file <paths>`
- Shell/runtime integration touched:
  `python tools/run.py verify --app tracer_core_shell --scope batch --concise`
- Android host/runtime touched:
  `python tools/run.py build --app tracer_android --profile android_edit`
