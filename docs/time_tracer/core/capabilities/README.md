# Core Capabilities

This directory groups capability-owned docs for `tracer_core`.

## Current Capabilities
1. [validation/README.md](validation/README.md)
   - Validation ownership, SOP, canonical text usage, and diagnostics.
2. [ingest/README.md](ingest/README.md)
   - Ingest ownership, persistence boundary, and legacy ingest-topic routing.
3. [query/README.md](query/README.md)
   - Query ownership, data-query routing, and stats-contract entry.
4. [reporting/README.md](reporting/README.md)
   - Reporting ownership and reporting-contract entry.
5. [exchange/README.md](exchange/README.md)
   - Exchange ownership and `.tracer` contract routing.
6. [config/README.md](config/README.md)
   - Config ownership and validation/config boundary routing.
7. [persistence/README.md](persistence/README.md)
   - Persistence write/runtime split and SQLite boundary routing.

## Migration Rule
When a doc clearly belongs to one capability, prefer placing it under
`capabilities/<name>/` instead of scattering it across `architecture/`,
`design/`, and `contracts/`.
