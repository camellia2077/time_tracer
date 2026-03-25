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
2. [docs/time_tracer/core/README.md](../docs/time_tracer/core/README.md)
   - core-side routing and boundary docs
3. [docs/time_tracer/core/specs/AGENT_ONBOARDING.md](../docs/time_tracer/core/specs/AGENT_ONBOARDING.md)
   - fastest route for core/shell changes
4. [docs/time_tracer/presentation/cli/README.md](../docs/time_tracer/presentation/cli/README.md)
   - Windows CLI artifact/runtime expectations
5. [docs/time_tracer/presentation/android/README.md](../docs/time_tracer/presentation/android/README.md)
   - Android-side presentation/runtime verification

Do not treat this README as the detailed test spec. Keep detailed structure,
commands, and contract rules in the docs tree or in suite-local READMEs.
