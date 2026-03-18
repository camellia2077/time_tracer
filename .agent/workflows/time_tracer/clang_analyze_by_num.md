---
description: Agent policy for one numbered time_tracer clang-analyzer issue
---

## Fixed Contract (MUST)
- Official app anchor: `tracer_core_shell`
- Official source scope: `core_family`
- Official analyze workspace: `build_analyze_core_family`
- Run from repo root only: `C:\code\time_tracer`

## Fixed Paths (MUST)
- Issue queue: `out/analyze/tracer_core_shell/build_analyze_core_family/issues/batch_*/issue_*.json|toon`
- Raw report: `out/analyze/tracer_core_shell/build_analyze_core_family/reports/run.sarif`
- Machine summary: `out/analyze/tracer_core_shell/build_analyze_core_family/summary.json`

## Input Policy (MUST)
- Input is exactly one pending `<ISSUE_ID>`.
- Resolve the real `issue_<ISSUE_ID>` artifact path first.
- Derive `<BATCH_ID>` from that path before doing anything else.
- Work on one issue only.

## Required Order (MUST)
- Use this order:
  1. Resolve the real issue artifact path.
  2. Read `issue_<ISSUE_ID>.toon` first when it exists.
  3. Fall back to `issue_<ISSUE_ID>.json` for full structured fields.
  4. Read the referenced source file and nearby context.
  5. Form one triage outcome only.

## Triage Outcomes (MUST)
- End with exactly one of these outcomes:
  - `fix-now`
  - `needs-human-triage`
  - `rerun-analyze-after-build`
- Do not mix several outcomes for one issue.

## Thin Workflow Policy (MUST)
- This workflow is triage-only.
- It does not imply:
  - auto-fix
  - auto-close
  - issue archive/remove
  - analyzer queue refresh
- If code changes are made, rerun:
  - `python tools/run.py analyze --app tracer_core_shell --source-scope core_family --build-dir build_analyze_core_family`
  - `python tools/run.py analyze-split --app tracer_core_shell --source-scope core_family --build-dir build_analyze_core_family`

## Guidance (MUST)
- Prefer bug-path events over guessing from the top-line message.
- Treat `issue_*.toon` as the first reading view, not the source of truth.
- Use `issue_*.json` when you need:
  - fingerprint
  - full event list
  - stable field names
- If the issue is clearly caused by stale artifacts or a changed build graph, choose `rerun-analyze-after-build`.

## Completion Rule (MUST)
- This workflow is done for the requested number only when:
  - the real `issue_<ISSUE_ID>` artifact was resolved
  - one triage outcome was chosen
  - any proposed fix is scoped to that one issue only
