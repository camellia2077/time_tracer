# Exchange Overview

## Purpose

Exchange owns tracer-exchange package flows and file-crypto-backed exchange
implementation in `tracer_core`.

## Responsibility Boundary

Exchange owns:
1. export of complete tracer exchange packages
2. inspect of `.tracer` files and decrypted package summaries
3. transactional import of exchange packages
4. package assembly/decoding and file-crypto integration

Exchange does not own:
1. generic ingest pipeline orchestration
2. generic query/reporting flows
3. standalone config ownership

## Main Owner Paths
1. `libs/tracer_core/src/application/use_cases/tracer_exchange_api.*`
2. `libs/tracer_core/src/infra/exchange/**`
3. `libs/tracer_core/src/infra/crypto/**`

## Allowed Direct Dependencies
1. `config`

## Forbidden Direct Dependencies
1. `pipeline`
2. `query`
3. `reporting`

## Read Next
1. [contracts.md](contracts.md)
2. [../../capabilities/validation/sop.md](../../capabilities/validation/sop.md)
