# graph_generator -> Core Contract Migration

Updated on: 2026-02-24

## Goal
Move graph generation from direct SQL access to Core semantic JSON contracts, so chart/stat logic stays aligned with Core and does not drift.

## Current Status
1. Heatmap (numeric/project duration): `in_progress`
2. Heatmap (boolean day flags): `pending`
3. Timeline: `pending`
4. Tree: `pending`

## Phased Plan
1. Phase 1 (done in this change)
   - Add source-mode switch to heatmap config (`sqlite` / `core_contract`).
   - Add `CoreContractSource` for `report-chart` consumption.
   - Keep SQL fallback for migration safety.
2. Phase 2
   - Extend Core contract for boolean day flags (`sleep/status/exercise`) or provide dedicated action.
   - Switch heatmap boolean charts to contract source.
3. Phase 3
   - Migrate tree generator to consume `query data tree --data-output json`.
   - Keep rendering logic in graph_generator, remove tree SQL coupling.
4. Phase 4
   - Migrate timeline generator to a stable semantic action (new contract if needed).
   - Remove timeline SQL query reconstruction logic from Python.
5. Phase 5
   - Add CI checks that compare SQL and contract outputs during transition.
   - When parity is stable, disable SQL mode by default.

## Policy
1. Core owns metrics/stat formulas and semantic field meaning.
2. graph_generator only maps contract payload -> chart view model.
3. If a needed field is missing, extend contract first, then migrate adapters.

