---
description: Safe Clang-Tidy Refactoring Workflow
---

This workflow guides the process of applying Clang-Tidy fixes and verifying them using the fast build/test pipeline.

1. **Select Refactoring Tasks (Batch Strategy)**:
   - Check the `time_tracer_cpp/apps/time_tracer/build_tidy/tasks/` directory.
   - **Bulk Trivial Strategy**: For highly uniform, low-risk changes (e.g., `[[nodiscard]]`, `trailing return type` across many files), select **10+ task logs** to process in one go.
   - **Mixed Batch Strategy**: For mixed cosmetic changes, select **3-5 task logs**.
   - **Complex Strategy**: For logic changes, header renames, or public API changes, select **ONLY 1 task log** to process atomically.
   - **C++23 Style Guidelines**:
     - **Names**: `PascalCase` for Functions/Classes, `snake_case` for Variables/Params, `snake_case_` for Class Members, and `kPascalCase` for Constants.
     - **Types**: Use trailing return types: `auto FunctionName(...) -> ReturnType`.
     - **Safety**: Prefer `std::string_view` for inputs and `std::expected`/`std::optional` for results.

2. **Impact Analysis**:
   - Before renaming any function or class found in the task logs, **MUST** search for all call sites using `grep` or `ripgrep`.
   - Identify all affected modules (adapters, application, domain, etc.).

3. **Code Refactoring & Safety**:
   - Apply suggested changes based on the selected task logs.
   - **Edit Safety**: If a tool fails due to "content not found", re-read the file via `view_file` to update context.

4. **Risk-Based Verification**:
   - **Level 1: Build Verification (Strictly Limited Scope)**:
     - **Use ONLY for**: Changes in `.cpp` files, or changes to `private` members/methods in `.hpp` files.
     - **Command**:
       // turbo
       `C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -where . -c "./time_tracer_cpp/apps/time_tracer/scripts/build_fast.sh"`
     
   - **Level 2: Full Regression Test (Mandatory for Public Headers)**:
     - **Use for**: ANY change to a `.hpp` file (unless strictly private), and all logic changes.
     - **Command**:
       // turbo
       - Run Build Verification first.
       - Then run business logic tests (using input redirection to avoid hangs):
         `cmd /c "cd my_test/test_executables && echo N | run_fast.bat"`
     - Ensure the script returns exit code 0 and "SUCCESS".

5. **Cleanup**:
   - Only if verification passes, use the cleaning script to remove processed logs.
   - **Command**:
     // turbo
     `C:\msys64\msys2_shell.cmd -ucrt64 -defterm -no-start -where . -c "./time_tracer_cpp/apps/time_tracer/scripts/tidy_clean.sh <ID1> <ID2> ..."`
   - *Example*: `./time_tracer_cpp/apps/time_tracer/scripts/tidy_clean.sh 020 021 022`