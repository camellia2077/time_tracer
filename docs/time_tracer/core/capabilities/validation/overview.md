# Validation Overview

## Purpose

Validation is responsible for deciding whether runtime-managed config and TXT
inputs are acceptable for downstream ingest, exchange, and host reuse.

It is the place where the core enforces:
1. config shape and rule correctness
2. canonical text normalization
3. TXT structure correctness
4. TXT semantic correctness

## Responsibility Boundary

Validation owns:
1. `TOML` config parsing and rule checks
2. canonical text normalization before parse/validate
3. TXT line and header interpretation
4. semantic checks that depend on config and parsed activity meaning
5. validation diagnostics, error categorization, and stable failure semantics

Validation does not own:
1. write-side persistence
2. query/report data access
3. UI copy and interaction details
4. host-specific staging directories

## Validation Layers

### 1. Config validation

Converter `TOML` files must parse and satisfy field/type/rule constraints before
their values are trusted by downstream TXT validation.

### 2. Canonical text normalization

Validation consumes canonical text only:
1. `UTF-8`
2. no effective BOM
3. `LF` newlines

This normalization happens at the outer boundary and is not reimplemented by
every downstream parser.

### 3. TXT structure validation

Structure validation is line-oriented and state-machine-driven.

It is not just regex checking, and it is not fully config-free. It already
depends on configuration values such as:
1. `remark_prefix`
2. `wake_keywords`
3. alias-derived valid event keywords

### 4. TXT logic validation

Logic validation reasons about parsed activities and day semantics:
1. date continuity
2. duration constraints
3. wake-anchor rules
4. cross-day continuity
5. activity recognition based on config and alias mapping

## Ingest Boundary

Validation is upstream of the ingest persistence gate.

That means:
1. validation failure must not create a new SQLite database
2. validation failure must not mutate persisted data
3. write repositories must stay lazy until validation has passed

The persistence-gate rule is documented in:
1. [../ingest/persistence_boundary.md](../ingest/persistence_boundary.md)
2. [sop.md](sop.md)

## Code Entry References
1. `libs/tracer_core/src/infra/config/loader/converter_config_loader.cpp`
2. `libs/tracer_core/src/infra/config/validator/converter/rules/converter_rules.cpp`
3. `libs/tracer_core/src/domain/logic/validator/structure/structure_validator.cpp`
4. `libs/tracer_core/src/domain/logic/validator/txt/facade/text_validator.cpp`
5. `libs/tracer_core/src/domain/logic/validator/txt/rules/txt_rules.cpp`
6. `libs/tracer_core/src/application/pipeline/pipeline_workflow.cpp`
7. `libs/tracer_core/src/application/pipeline/detail/pipeline_workflow_support_impl.inc`
