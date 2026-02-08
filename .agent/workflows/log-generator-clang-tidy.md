---
description: Log Generator Clang-Tidy Workflow (Python-first)
---

This workflow uses Python commands for agent execution. Shell scripts are kept for manual use.

0. **Generate Tasks (Python Only)**:
   - Run tidy to generate `build.log`, split tasks, and summaries.
   - **Command**:
     // turbo
     `python ./time_tracer_cpp/apps/log_generator/scripts/workflow.py tidy`

1. **Select Refactoring Tasks (Batch Strategy)**:
   - Check `time_tracer_cpp/apps/log_generator/build_tidy/tasks/`.
   - **Bulk Trivial Strategy**: 10+ tasks.
   - **Mixed Batch Strategy**: 3–5 tasks.
   - **Complex Strategy**: 1 task only.

2. **Impact Analysis**:
   - Before renaming functions or types, search call sites using `rg`.

3. **Code Refactoring**:
   - Apply changes based on selected task logs.

4. **Verify + Cleanup (Preferred)**:
   - Runs fast build first. Only cleans logs if build succeeds.
   - **Command**:
     // turbo
     `python ./time_tracer_cpp/apps/log_generator/scripts/workflow.py verify <ID1> <ID2> ...`
   - *Example*: `python ./time_tracer_cpp/apps/log_generator/scripts/workflow.py verify 010 011 012`

5. **Manual Cleanup (No Build)**:
   - Use only if you already verified separately.
   - **Command**:
     // turbo
     `python ./time_tracer_cpp/apps/log_generator/scripts/workflow.py clean <ID1> <ID2> ...`
   - *Example*: `python ./time_tracer_cpp/apps/log_generator/scripts/workflow.py clean 010 011 012`

---
**For humans (optional shortcuts)**:
- `sh ./time_tracer_cpp/apps/log_generator/scripts/sh/refactor/build_tidy.sh`
- `sh ./time_tracer_cpp/apps/log_generator/scripts/sh/build-sh/build_fast.sh`
