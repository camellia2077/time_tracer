# Insights Overview

## Purpose

Insights owns insights query, insights-data assembly, formatter flows, and insights
export behavior in `tracer_core`.

## Responsibility Boundary

Insights owns:
1. insights query orchestration
2. insights-data query flows
3. insights formatter selection and output text generation
4. insights export behavior within the insights capability boundary

Insights does not own:
1. generic query/data-query stats surfaces
2. write-side ingest/import
3. exchange packaging/import
4. config loading as a standalone capability

## Main Owner Paths
1. `libs/tracer_core/src/application/insights/**`
2. `libs/tracer_core/src/application/use_cases/insights_api*`
3. `libs/tracer_core/src/infra/insights/**`

## Allowed Direct Dependencies
1. `config`
2. `persistence_runtime`

## Forbidden Direct Dependencies
1. `query`

## Read Next
1. [contracts.md](contracts.md)
2. [../../contracts/insights/insights_data_consistency_spec_v1.md](../../contracts/insights/insights_data_consistency_spec_v1.md)
3. [../../contracts/insights/insights_output_text_contract_v1.md](../../contracts/insights/insights_output_text_contract_v1.md)
