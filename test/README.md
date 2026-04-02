# test

Thin shell for the repository test workspace.

Use this directory as the runtime/test root only:

- `suites/`
  - canonical artifact suite definitions
- `data/`
  - shared integration/e2e input data
- `fixtures/`
  - small focused fixtures
- `golden/`
  - snapshot/golden baselines
- `run.py`
  - test workspace entrypoint used by `tools/run.py`

Read docs before broad search:

1. [test/suites/README.md](./suites/README.md)
   - suite layout and naming
2. [test/data/README.md](./data/README.md)
   - shared fixture date range and input-target guidance
3. [test/AGENTS.md](./AGENTS.md)
   - test scope reading order for agents
4. [docs/toolchain/test/README.md](../docs/toolchain/test/README.md)
   - toolchain-side test and result contract
5. [docs/toolchain/workflows/README.md](../docs/toolchain/workflows/README.md)
   - shared toolchain workflow and cadence notes
6. [docs/toolchain/test/history/README.md](../docs/toolchain/test/history/README.md)
   - test contract and framework evolution notes
7. [docs/time_tracer/core/README.md](../docs/time_tracer/core/README.md)
   - core-side routing and boundary docs
8. [docs/time_tracer/core/specs/AGENT_ONBOARDING.md](../docs/time_tracer/core/specs/AGENT_ONBOARDING.md)
   - fastest route for core/shell changes
9. [docs/time_tracer/presentation/cli/README.md](../docs/time_tracer/presentation/cli/README.md)
   - Windows CLI artifact/runtime expectations
10. [docs/time_tracer/presentation/android/README.md](../docs/time_tracer/presentation/android/README.md)
   - Android-side presentation/runtime verification

Do not treat this README as the detailed test spec. Keep detailed structure,
commands, and contract rules in the docs tree, `test/AGENTS.md`, or suite-local
READMEs.
