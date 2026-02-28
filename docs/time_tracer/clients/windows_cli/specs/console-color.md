# Core Console Color Reference

This note is for agents working in `apps/tracer_cli/windows` who need to adjust CLI text colors.

## Source Of Truth

The shared console color palette is defined in core:

- `apps/tracer_core/src/shared/types/ansi_colors.hpp`

`apps/tracer_cli/windows` consumes these constants directly via:

- `#include "shared/types/ansi_colors.hpp"`

## Defined Color Constants

From `time_tracer::common::colors`:

| Constant | ANSI value | Typical use |
| --- | --- | --- |
| `kReset` | `\033[0m` | Reset style/color after colored output |
| `kBold` | `\033[1m` | Emphasis for titles/headings |
| `kItalic` | `\033[3m` | Motto/info text |
| `kRed` | `\033[31m` | Errors/failures |
| `kGreen` | `\033[32m` | Success messages |
| `kYellow` | `\033[33m` | Warnings/partial status |
| `kCyan` | `\033[36m` | Command names/steps |
| `kGray` | `\033[90m` | Secondary metadata |
| `kBrightGreen` | `\033[92m` | Important success banner/version |
| `kBrightCyan` | `\033[96m` | Pipeline start banner |

## Main Usage Locations

Core side (produces many colored pipeline/status lines):

- `apps/tracer_core/src/application/pipeline/pipeline_manager.cpp`
- `apps/tracer_core/src/application/pipeline/steps/pipeline_stages.cpp`
- `apps/tracer_core/src/application/workflow_handler.cpp`

Windows CLI side (help/version/top-level errors):

- `apps/tracer_cli/windows/src/api/cli/main.cpp`
- `apps/tracer_cli/windows/src/api/cli/impl/app/cli_application.cpp`
- `apps/tracer_cli/windows/src/api/cli/impl/app/app_runner.cpp`
- `apps/tracer_cli/windows/src/api/cli/impl/utils/console_helper.cpp`

## Change Rules

1. If you want a global palette change, edit `apps/tracer_core/src/shared/types/ansi_colors.hpp`.
2. If you want only one command/page to look different, change call sites in `apps/tracer_cli/windows/src/...`.
3. Always terminate styled output with `kReset` to avoid color bleed into later lines.
4. Do not add manual severity prefixes like `"[INFO]"` if logger/sink already emits severity tags.
5. Keep report formatter color settings (`keyword_colors` in report TOML) separate from console ANSI colors.

## Windows Console Requirement

ANSI escape rendering on Windows relies on:

- `apps/tracer_cli/windows/src/api/cli/impl/utils/console_helper.cpp`

`SetupConsole()` enables `ENABLE_VIRTUAL_TERMINAL_PROCESSING`.

## Quick Audit Commands

From repository root:

```powershell
rg -n "shared/types/ansi_colors.hpp|colors::k" apps/tracer_core/src apps/tracer_cli/windows/src -S
rg -n "kReset|kRed|kGreen|kYellow|kCyan|kGray|kBright" apps/tracer_cli/windows/src -S
```

