# Android Local Contract

## Scope

Applies to `apps/android/**`. Android owns presentation, app-local state, and
host/runtime integration; shared business semantics remain in `tracer_core`.

## Required Read Set

1. `docs/time_tracer/presentation/android/README.md`
2. `docs/time_tracer/presentation/android/specs/AGENT_ONBOARDING.md`
3. `docs/time_tracer/presentation/android/specs/EDIT_ROUTING.md`

## Read By Task

- Structure or dependency direction:
  `docs/time_tracer/presentation/android/specs/STRUCTURE.md`
- Build or validation selection:
  `docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md`
- Runtime/config lifecycle:
  `docs/time_tracer/presentation/android/specs/CONFIG_ASSET_LIFECYCLE.md`
- Runtime payloads:
  `docs/time_tracer/presentation/android/runtime-protocol.md`
- User-visible behavior:
  `docs/time_tracer/presentation/android/features.md`
- Exchange, TXT import/export, SAF/document, or fd export:
  - `docs/time_tracer/presentation/android/reference/data-import-export.md`
  - `docs/time_tracer/core/contracts/crypto/tracer_exchange_package_v6.md`
  - `docs/time_tracer/core/contracts/crypto/runtime_crypto_json_contract_v1.md`
- Activity documentation changes:
  `docs/time_tracer/presentation/android/specs/DOC_RULES.md`

## Ownership And Routing

- `app`: composition root and app-local wiring.
- `feature-data`, `feature-record`, `feature-report`: feature UI and state.
- `runtime`: runtime/JNI implementation.
- `contract`: gateway interfaces and shared presentation models.
- UI routes and app-side tests should depend on the smallest gateway interface;
  `RuntimeGateway` remains an aggregate composition boundary.
- Do not reimplement core TXT, validation, report, query, or exchange semantics
  in Kotlin presentation code.

## Local Invariants

- Android `config/program` assets are generated during Gradle builds from the
  repository root `config/program`; generated output is not checked in.
- Android version source: `apps/android/meta/version.properties`.
- Core version source: `libs/tracer_core/src/shared/types/version.hpp`.
- Never run Gradle-backed commands concurrently in this workspace.
- Hostless `setContent { ... }` composable checks belong in local/unit coverage,
  not the real-device profile.

## Validation

`docs/time_tracer/presentation/android/specs/BUILD_WORKFLOW.md` is authoritative
for validation commands, minimum checks, trigger conditions, prerequisites, and
outputs. For normal Android code changes, do not run `android_style`; select the
smallest targeted compile or unit-test path instead and keep Gradle-backed work
serial. Run `android_style` only when the user explicitly requests it.

## Tests And Assets

- Share `test/data/**` canonical TXT input with the Windows CLI.
- Prefer `test/fixtures/config/**` and `test/fixtures/text/**` for focused error
  and compatibility cases.
- Use `test/golden/**` only for stable final baselines.
- Write runtime results, databases, and exports under `out/test/**`, not
  `test/**`.

## Local Completion Bar

- UI state and gateway ownership remain in the correct module.
- A UI behavior change has focused state/render coverage and passes the smallest
  applicable targeted check. `android_style` is not part of the default Android
  change validation.
- A runtime, config, or contract change has the relevant targeted build/unit
  coverage and synchronizes the runtime protocol or core contract.
- Every check triggered by the workflow matrix and task scope passes before
  completion.
