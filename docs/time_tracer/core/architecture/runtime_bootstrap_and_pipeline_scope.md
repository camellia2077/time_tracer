# Runtime Bootstrap and Pipeline Scope

## Purpose

This document records the boundary between the complete application Runtime
and the pipeline-only Runtime used for TXT validation, conversion, and ingest.
The boundary is important because program resources and user data have
different ownership and lifecycle rules.

## Configuration Ownership

| Configuration | Owner | Used by TXT pipeline | Used by complete Runtime |
| --- | --- | --- | --- |
| `config/user/behavior.toml` | user data | yes | yes |
| `config/user/activity_hierarchy/*.toml` | user data | yes | yes |
| `config/program/meta/bundle.toml` | presentation/runtime | no | yes |
| `config/program/reports/**` | presentation/runtime | no | yes, for reports |
| `config/program/charts/**` | presentation/runtime | no | yes, for charts |

`config/program/**` is not exchange data. It contains immutable or
presentation-owned resources delivered by the host, such as report templates,
chart definitions, and the program resource index.

## Runtime Families

### Complete application Runtime

The complete Runtime composes the application capabilities needed by the
presentation, including:

- TXT pipeline and persistence;
- query services;
- report catalog and formatters;
- chart/report program resources;
- exchange services.

Its bootstrap may require `config/program/meta/bundle.toml` because report and
formatter capabilities use the program resource index.

### Pipeline-only Runtime

The pipeline-only Runtime composes only the Core pipeline capability required
for candidate validation and ingest. It loads:

- `config/user/behavior.toml`;
- the sibling `config/user/activity_hierarchy/**` TOML tree;
- TXT input and persistence dependencies.

It must not require, copy, or validate `config/program/**`. A candidate import
directory containing only user configuration and TXT input is a valid input to
this Runtime family.

## Android Candidate Flow

Android uses the pipeline-only Runtime when it builds a candidate database for:

1. TXT+TOML data-folder import;
2. activity hierarchy migration and TXT rewrite.

After candidate validation and ingest succeed, Android shuts down the
pipeline-only Runtime, atomically activates the candidate data, and starts the
complete Runtime for normal application use.

This avoids treating an import staging directory as a complete application
installation and keeps program resources out of the exchange/import contract.

## Invariants

1. TXT structure and logic validation depend on valid user converter config and
   activity hierarchy data, not on report or chart resources.
2. `bundle.toml` is a program-resource index, not a requirement of TXT data
   exchange.
3. Exchange content is limited to the exchange contract, currently
   `config/user/**` and `payload/**`; `config/program/**` is excluded.
4. A change to report resources must not make TXT validation or data import
   fail when the pipeline-only Runtime has the required user configuration.

## Implementation References

- Core factory: `apps/tracer_core_shell/host/bootstrap/android_runtime_factory.cpp`
- Pipeline-only C API: `apps/tracer_core_shell/api/c_api/capabilities/pipeline/tracer_core_c_api_pipeline_runtime.cpp`
- Android JNI bridge: `apps/tracer_core_shell/api/android_jni/native_bridge_calls.cpp`
- Android candidate import: `apps/android/runtime/src/main/java/com/example/tracer/runtime/services/RuntimeDataFolderSnapshotService.kt`
