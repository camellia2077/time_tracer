---
description: Build and test time_tracer after code changes
---

0. **Incremental Build Rule (required)**:
   - Always reuse `apps/time_tracer/build_agent` for incremental builds.
   - **Do not delete** `apps/time_tracer/build_agent` unless explicitly required.

1. **Configure (required)**:
   - `python scripts/run.py configure --app time_tracer --build-dir build_agent -- -DCMAKE_BUILD_TYPE=Release -DDISABLE_OPTIMIZATION=OFF -DENABLE_LTO=OFF -DWARNING_LEVEL=2`

2. **Build (required)**:
   - `python scripts/run.py build --app time_tracer --build-dir build_agent`

3. **Run Full Test Suite (required)**:
   - `python test/run.py --suite time_tracer --agent --build-dir build_agent --concise`

4. **Validate Test Result (required)**:
   - Check `test/output/time_tracer/result.json`.
   - Expected: `"success": true`.

5. **Check Logs When Failed (required on failure)**:
   - Main aggregated log: `test/output/time_tracer/logs/output.log`
   - Case logs: `test/output/time_tracer/logs/**`
   - Use these logs to locate the exact failed step before editing code again.

6. **Optional Fast Runtime Smoke (recommended after passing tests)**:
   - `apps/time_tracer/build_agent/bin/time_tracker_cli.exe query day 2026-01-02 -f md --db test/output/time_tracer/workspace/output/db/time_data.sqlite3 -o test/output/time_tracer/artifacts/reports`
   - `apps/time_tracer/build_agent/bin/time_tracker_cli.exe export day 2026-01-02 -f md --db test/output/time_tracer/workspace/output/db/time_data.sqlite3 -o test/output/time_tracer/artifacts/reports`

7. **Completion Criteria**:
   - Build succeeds.
   - Full suite succeeds (`result.json -> success: true`).
   - (Optional) smoke commands run successfully for quick runtime confidence.
