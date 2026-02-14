# Platform Sync Contract (Windows + Android)

## Goal

Define mandatory cross-platform sync rules before large Android migration.
All feature work touching platform capabilities must keep Windows and Android
states explicit and reviewable.

## Rules

1. Change port first, then update implementations.
2. A platform port cannot be marked done unless both platform statuses are
   explicitly updated.
3. New platform ports must be registered in this document in the same change.
4. Merge blocking rule: if any required platform implementation/check fails,
   the change is not ready to merge.

## Port Ownership Matrix

| Port | Layer | Windows Impl | Android Impl | Windows Status | Android Status | Notes |
| --- | --- | --- | --- | --- | --- | --- |
| `IPlatformClock` | `application/ports` | `infrastructure/platform/windows/windows_platform_clock.*` | `infrastructure/platform/android/android_platform_clock.*` | Done | Compile baseline | Android keeps compile-only baseline for now. |

## Update Workflow

1. Update port contract (`application/ports/*`).
2. Update Windows adapter.
3. Update Android adapter (placeholder is acceptable only if contract says so).
4. Run sync checks and platform build checks.
5. Update PR checklist and this contract status table.
