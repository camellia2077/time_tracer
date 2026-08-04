# Refactoring Guidance

This is the canonical refactoring workflow for reusable libraries and
presentation/client modules. The LOC Scanner, agents, and code reviews should
use this document as the shared decision standard.

## Decision Workflow

```text
LOC Scanner finds candidates
    ↓
Agent analyzes responsibilities, coupling, and reasons for change
    ↓
Confirm that a real boundary exists
    ↓
Split only by responsibility
    ↓
Check whether dependencies decreased and cohesion improved
```

The scanner only finds candidates. It does not decide whether a file violates
single responsibility, whether business rules are duplicated, whether the
dependency direction is wrong, whether a Runtime protocol would be broken,
whether test coverage is sufficient, or whether the refactoring benefit is
higher than its risk.

## Rules

- LOC thresholds are triage signals, not refactoring criteria.
- A cohesive unit may remain together even when it is large.
- Do not split code merely to reduce the line count or make a hotspot move.
- A split is justified only when the extracted unit has a distinct
  responsibility, an explicit owner, and a meaningful reason to change
  independently.
- Prefer fewer and clearer dependency edges. Do not introduce a second model,
  facade, protocol representation, or duplicated business rule just to create
  smaller files.
- Preserve public, wire, C ABI, JNI, and Runtime protocol contracts unless the
  change explicitly includes a contract update.

## Refactoring Purpose

The purpose of a split is to reduce mixed responsibilities and make different
reasons for change independently owned. A successful split should clarify the
ownership of state, business operations, and persistence coordination, reduce
unnecessary dependencies or duplicated rules, and improve cohesion and
independent testability.

## Do Not Split Mechanically

- Do not create a class for every method or CRUD operation.
- Do not move methods into another large class without improving ownership or
  dependency direction.
- Do not split code that is already highly cohesive merely because the file is
  large.
- Do not introduce pass-through wrappers, duplicate state, duplicate DTOs, or
  a second Runtime protocol representation just to create smaller files.
- Do not use a lower LOC count or a moved hotspot as the acceptance criterion.

## Required Evidence

Before editing, the agent should record the responsibility and change-reason
hypothesis, current ownership, important callers and dependencies, the real
boundary, relevant contracts and tests, expected dependency/cohesion impact,
and the validation and risk plan.

After editing, run focused tests and contract/runtime tests, check ownership,
dependency direction, and cohesion, then re-run the LOC Scanner. The baseline
delta is supporting evidence only; a lower line count is not a success metric
by itself. The split is successful only when responsibility boundaries are
clearer, reasons for change are more independent, dependencies are simpler or
more intentional, and cohesion is improved or preserved intentionally.

## Scope Routing

- Library-specific ownership and boundary details: [Library Architecture Docs](libraries/README.md) and [Library Refactoring Architecture Guidance](libraries/libs_refactoring_guidance.md).
- Android presentation and Runtime details: [Android architecture](../presentation/android/architecture.md), [Android structure](../presentation/android/specs/STRUCTURE.md), and the Android local agent rules.
- Other client-specific rules remain in their component documentation, but
  their refactoring decisions still follow this workflow.
