# Android Structure

## Purpose

Describe stable Android module boundaries and dependency direction.

## When To Open

- Open this when the task changes architecture, layering, or dependency direction.
- Use it to confirm which layer owns a change before editing code.

## What This Doc Does Not Cover

- Full per-file inventories
- Temporary refactor notes
- User-visible feature details

## Stable Module Roles

- `app`
  - composition root, screen-level orchestration, cross-feature wiring
- `feature-data`
  - Data tab presentation
- `feature-record`
  - Record/TXT presentation and shared record-side UI state
- `feature-ui-common`
  - shared reusable Compose presentation primitives for multiple feature modules
- `feature-report`
  - report/query/chart presentation
- `runtime`
  - runtime implementation, JNI integration, services, coreadapter, translators
- `contract`
  - gateway interfaces and cross-module models

## Dependency Direction

- `app -> contract + feature-* + runtime`
- `feature-data -> contract`
- `feature-record -> contract + feature-ui-common`
- `feature-report -> contract + feature-ui-common`
- `feature-ui-common -> (no Android project-module dependency)`
- `runtime -> contract`

Within the app layer, prefer the smallest gateway interface needed by the UI route.
App-side tests should follow the same rule and avoid implementing `RuntimeGateway` unless a test is explicitly validating the aggregate boundary itself.

## Composition Root Boundaries

- `TracerScreen` is the Android composition root for the tab host.
- `TracerTabs` is the tab registry and shell contract owner.
- App-local route helpers under `ui/screen/tracer` may own:
  - action wiring
  - route-local state
  - transfer-flow skeletons
- Feature modules own tab-specific presentation and view-model behavior.

## Runtime Boundaries

- `NativeRuntimeController`
  - facade and composition root for runtime-facing contracts
  - aggregate `RuntimeGateway` stays here, not in app-layer route dependencies
- `runtime/coreadapter`
  - native-init and native-query execution path
- `runtime/services`
  - capability-oriented runtime behavior
- `runtime/controller`
  - delegate logic where validation and domain mapping need a narrower boundary
- `runtime/translators`
  - raw/native response translation into domain results

## Change By Layer

- Change cross-feature Android screen wiring:
  - `app`
- Change tab-specific UI behavior:
  - matching `feature-*`
- Change reusable shared UI primitives used by multiple tabs:
  - `feature-ui-common`
- Change runtime capability behavior:
  - `runtime/services` or `runtime/coreadapter`
- Change raw/native response translation:
  - `runtime/translators`
- Change contract shape:
  - `contract`, then sync runtime and UI consumers

## Stable Constraints

- UI should not depend on JNI or native-call details.
- Feature modules should not parse raw native JSON directly.
- Shared config source of truth remains `assets/tracer_core/config`.
- Android runtime config snapshot is not the canonical source.
