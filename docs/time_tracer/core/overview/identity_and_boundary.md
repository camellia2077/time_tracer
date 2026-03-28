# tracer_core Identity And Boundary

This document is a routing aid for active code. It should help a reader decide
where to search first without restating every detailed implementation inventory.

## Identity
1. External compatibility app id stays `tracer_core`.
2. The active shell/runtime root is `apps/tracer_core_shell`.
3. `libs/tracer_core` remains the business-logic owner.
4. `libs/tracer_core_bridge_common` holds shared bridge helpers for shell/JNI.

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

## Read Next
1. [capability_map.md](capability_map.md)
2. [module_boundaries.md](module_boundaries.md)
3. [../shared/c_abi.md](../shared/c_abi.md)
4. [../capabilities/validation/README.md](../capabilities/validation/README.md)

## Maintenance Rule
Keep this doc coarse-grained. If a detail is specific to one capability, one
contract, or one test suite, move it to that narrower doc instead.
