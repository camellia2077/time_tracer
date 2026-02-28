# CLI Output Style Guide

This guide defines text/format rules for `apps/tracer_cli/windows` CLI output.
Use it when changing command output, help text, step logs, or error summaries.

## Scope

- Applies to all user-facing terminal output in `apps/tracer_cli/windows/src/...`.
- Applies to runtime pipeline output forwarded from core (`apps/tracer_core`).

## Single Source References

- Style and structure rules: `docs/time_tracer/clients/windows_cli/specs/cli-output-style.md`
- Console color reference: `docs/time_tracer/clients/windows_cli/specs/console-color.md`
- Shared ANSI constants: `apps/tracer_core/src/shared/types/ansi_colors.hpp`

## Output Structure Rules

1. Keep one clear flow per command:
   - start banner (optional)
   - step/progress lines
   - details (only when needed)
   - one final summary line
2. Do not print duplicate final summaries.
3. Do not print the same detailed error block in both detail and summary sections.
4. Use blank lines only between major sections, not between every line.

## Severity And Prefix Rules

1. Severity prefix ownership belongs to logger/sink.
2. Business messages must not embed duplicated prefixes such as `[INFO] [INFO] ...`.
3. Summary failures should be short and actionable:
   - what failed
   - where to find full report/log path

## Text Style Rules

1. Prefer imperative and consistent verbs:
   - `Loading configuration...`
   - `Validating source structure...`
   - `Saving processed output...`
2. Keep punctuation consistent (`...` for progress, full stop optional for summaries).
3. Avoid mixing too many synonyms for the same phase (`loading/init/read`).
4. Keep bilingual text style stable within the same command path.

## Color Rules

1. Use only `time_tracer::common::colors::*` constants.
2. Always terminate styled segments with `kReset`.
3. Recommended semantic mapping:
   - error: `kRed`
   - warning: `kYellow`
   - success: `kGreen` or `kBrightGreen`
   - step/header: `kCyan` or `kBrightCyan`
   - metadata/help hint: `kGray`
4. Color is enhancement only; text must remain understandable without color.

See detailed palette and usage references in:

- `docs/time_tracer/clients/windows_cli/specs/console-color.md`

## Error Presentation Rules

1. Show detailed validation errors once.
2. Keep final `Logic Error`/top-level error summary concise.
3. Include report destination when available (`errors.log` path).
4. Avoid re-wrapping the same diagnostics payload at multiple layers.

## Review Checklist

- No duplicated severity prefixes.
- No duplicated detailed error blocks.
- Final summary appears once.
- Color usage follows palette mapping and resets correctly.
- Output remains clear with color disabled.

