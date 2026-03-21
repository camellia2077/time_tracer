# Android Activity Doc Rules

## Purpose

Define how active Android documents should be written and maintained.

## When To Open

- Open this when you create or heavily rewrite Android documentation.
- Use it to decide whether content belongs in navigation docs, reference docs, or history.

## What This Doc Does Not Cover

- Product behavior itself
- Runtime wire format details
- Historical release notes

## Documentation Layers

Use these layers consistently:

1. Local entry
   - `apps/android/agent.md`
   - `apps/android/README.md`
2. Navigation and rules
   - `docs/time_tracer/presentation/android/README.md`
   - `docs/time_tracer/presentation/android/specs/*.md`
3. Behavior reference
   - `docs/time_tracer/presentation/android/reference/*.md`
4. History
   - `docs/time_tracer/presentation/android/history/**`
   - `docs/time_tracer/presentation/android/HISTORY.md`
   - `docs/time_tracer/presentation/android/CHANGELOG.md`

## Activity Doc Rules

- Activity docs are English-first.
- Start every activity doc with:
  - `Purpose`
  - `When To Open`
  - `What This Doc Does Not Cover`
- Prefer boundaries, entrypoints, constraints, and validation over implementation detail.
- Prefer task routing over full file inventories.
- Prefer 1 to 3 first-entry files, not exhaustive lists.
- Keep time-sensitive refactor narratives out of activity docs.
- Move design history and old alternatives into `history/`.

## Soft Size Targets

- Local entry docs:
  - short
- Onboarding, structure, and edit routing:
  - medium
- Deep behavior detail:
  - move to `reference/`

## Update Rules

- If code structure changes, update:
  - `specs/STRUCTURE.md`
  - `specs/EDIT_ROUTING.md`
- If user-visible behavior changes, update the matching `reference/` page.
- If commands or validation change, update `specs/BUILD_WORKFLOW.md`.
- If the change is historical only, write it in `history/` or `CHANGELOG.md`, not in the navigation docs.

## Anti-Patterns

Do not let activity docs become:

- a changelog
- a private implementation notebook
- a giant file inventory
- a replacement for reading code
