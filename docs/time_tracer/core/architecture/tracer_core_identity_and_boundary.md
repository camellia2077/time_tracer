# `tracer_core` Identity And Boundary

This doc is a routing aid for active code. It should help an agent decide where
to search first, not restate detailed implementation inventories.

## Identity

1. External compatibility app id stays `tracer_core`.
2. The active shell/runtime root is `apps/tracer_core_shell`.
3. `libs/tracer_core` remains the business-logic owner.
4. `libs/tracer_core_bridge_common` holds shared bridge helpers for shell/JNI
   code.

## Top-Level Routing

Use path family first:

1. `libs/tracer_core/**`
   - capability-owned business logic, DTOs, ports, modules, and infra
2. `apps/tracer_core_shell/api/c_api/capabilities/**`
   - shell-facing capability facades
3. `apps/tracer_core_shell/api/c_api/runtime/**`
   - non-owner C ABI runtime glue
4. `apps/tracer_core_shell/api/android_jni/**`
   - Android JNI entrypoints and registration
5. `apps/tracer_core_shell/host/bootstrap/**`
   - runtime/bootstrap assembly
6. `apps/tracer_core_shell/host/exchange/**`
   - exchange/file-crypto host bridges
7. `apps/tracer_core_shell/tests/**`
   - shell/platform/integration regressions

If a change does not fit one of these families, stop and confirm the boundary
before adding a new path family.

## Shell Boundary Rules

1. New business logic does not enter `apps/tracer_core_shell/**`; it belongs in
   `libs/**`.
2. New shell entrypoints must not introduce new flat `api/c_api/*.cpp` or
   `host/*.cpp` roots.
3. The retained root-level C ABI facade is:
   - `apps/tracer_core_shell/api/c_api/tracer_core_c_api.cpp`
   - `apps/tracer_core_shell/api/c_api/tracer_core_c_api.h`
4. New shell files must fit an existing family and must also update:
   - `tools/toolchain/commands/cmd_quality/verify_internal/verify_profile_inference.py`
   - the matching verify/profile inference tests

## Read Next

1. [Core Agent Onboarding](../specs/AGENT_ONBOARDING.md)
2. [tracer_core Capability Boundary Contract](../design/tracer_core_capability_boundary_contract.md)
3. [tracer_core Library Routing](../../architecture/libraries/tracer_core.md)
4. [C ABI Contract](../contracts/c_abi.md)

## Maintenance Rule

Keep this doc coarse-grained. If a detail is specific to one capability,
specific to one contract, or specific to one test suite, put it in the
capability doc or contract doc instead of expanding this file.
