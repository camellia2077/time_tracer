# tracer_transport Local Contract

## Scope

Applies to `libs/tracer_transport/**`. This library owns shared transport and
codec implementation.

## Read By Task

- Any implementation or public-surface change:
  `docs/time_tracer/architecture/libraries/tracer_transport.md`
- Dependency or ownership change:
  `docs/time_tracer/architecture/library_dependency_map.md`
- Exported runtime/C ABI payload change:
  `docs/time_tracer/core/shared/c_abi.md`
- TXT runtime payload shape:
  `docs/time_tracer/core/contracts/text/runtime_txt_day_block_json_contract_v1.md`

## Ownership Boundaries

- Own JSON envelope encoding/decoding, codec mechanics, default-field fallback,
  and transport capability projection.
- Own transport encoding for the canonical temporal-insights runtime surface and
  the separate insights-batch helper payload.
- Do not define TXT action meaning, month-TXT day-block rules, insights business
  semantics, or application policy. Change those in `tracer_core` first.
- Do not move filesystem behavior into this library; that belongs to
  `tracer_adapters_io`.

## Tests And Assets

- Keep codec and envelope tests under `libs/tracer_transport/tests/**`.
- Prefer inline payloads for library-local codec cases.
- Use `test/fixtures/**` only for small payloads shared across layers.
- Do not use `test/data/**` or `test/golden/**` as transport-owned test pools.

## Validation

Required for transport code, config, or test changes:

```powershell
python tools/run.py verify --app tracer_core_shell --profile fast --concise
```

## Local Completion Bar

- A public payload change updates the owning ABI/contract document.
- A changed codec path has request/response coverage plus malformed or missing
  field coverage where fallback behavior is part of the contract.
- Transport code, config, or test changes pass the focused validation above.
- No business-semantic ownership moved into the transport layer.
