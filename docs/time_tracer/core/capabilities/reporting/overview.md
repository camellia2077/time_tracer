# Reporting Overview

## Purpose

Reporting owns report query, report-data assembly, formatter flows, and report
export behavior in `tracer_core`.

## Responsibility Boundary

Reporting owns:
1. report query orchestration
2. report-data query flows
3. report formatter selection and output text generation
4. report export behavior within the reporting capability boundary

Reporting does not own:
1. generic query/data-query stats surfaces
2. write-side ingest/import
3. exchange packaging/import
4. config loading as a standalone capability

## Main Owner Paths
1. `libs/tracer_core/src/application/reporting/**`
2. `libs/tracer_core/src/application/use_cases/report_api*`
3. `libs/tracer_core/src/infra/reporting/**`

## Allowed Direct Dependencies
1. `config`
2. `persistence_runtime`

## Forbidden Direct Dependencies
1. `query`

## Read Next
1. [contracts.md](contracts.md)
2. [../../contracts/reporting/report_data_consistency_spec_v1.md](../../contracts/reporting/report_data_consistency_spec_v1.md)
3. [../../contracts/reporting/report_output_text_contract_v1.md](../../contracts/reporting/report_output_text_contract_v1.md)
