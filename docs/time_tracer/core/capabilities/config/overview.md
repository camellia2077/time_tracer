# Config Overview

## Purpose

Config owns runtime config loading, snapshotting, validators, and
report/converter config assembly in `tracer_core`.

## Responsibility Boundary

Config owns:
1. config file loading and normalization at the config boundary
2. config snapshot assembly for downstream capabilities
3. config validators and config-backed option providers
4. internal parse helpers that belong to config itself

Config does not own:
1. ingest/query/reporting/exchange orchestration
2. host-side path resolution UX
3. SQLite write or read repositories

## Main Owner Paths
1. `libs/tracer_core/src/infra/config/**`

## Allowed Direct Dependencies
1. shared/domain/application DTOs and option types
2. config-owned internal parse helpers

## Forbidden Direct Dependencies
1. pipeline/query/reporting/exchange orchestration
2. capability-specific business flows

## Read Next
1. [alias_mapping_rules.md](./alias_mapping_rules.md)
2. [../../capabilities/validation/config_validation.md](../../capabilities/validation/config_validation.md)
3. [../../overview/capability_map.md](../../overview/capability_map.md)
4. [../../overview/module_boundaries.md](../../overview/module_boundaries.md)
