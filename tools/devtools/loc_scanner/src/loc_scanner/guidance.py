from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Guidance:
    summary: str
    path: str | None
    validation: tuple[str, ...]


_GUIDANCE: dict[str, Guidance] = {
    "tracer_core": Guidance(
        summary="Check core capability boundaries and separate orchestration, parsing, validation, query, and persistence responsibilities.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("Relevant cap_* profile", "Fast cross-boundary profile"),
    ),
    "tracer_adapters_io": Guidance(
        summary="Separate readers, writers, file discovery, and processed-data persistence; keep business semantics out of the IO adapter.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("Adapter module tests", "Related core integration path"),
    ),
    "tracer_core_bridge_common": Guidance(
        summary="Extract only duplicated C API/JNI boundary mapping and transport delegation; do not add business orchestration or a second DTO layer.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("Shell/runtime tests", "Android runtime tests"),
    ),
    "tracer_transport": Guidance(
        summary="Separate envelopes, field readers, and operation codecs while preserving missing-field, default-value, and round-trip contracts.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("Transport contract tests", "Shell/runtime round-trip tests"),
    ),
    "tracer_core_shell": Guidance(
        summary="Keep the shell as a thin adapter; core business rules and runtime semantics belong in tracer_core.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("tracer_core_shell fast profile", "C API integration tests"),
    ),
    "android": Guidance(
        summary="Keep Android in the presentation/client layer; reuse capabilities through the core runtime instead of duplicating business rules.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("Android runtime tests", "Core contract tests"),
    ),
    "windows_cli": Guidance(
        summary="Keep the Rust CLI thin: isolate command models, orchestration, presentation, and the single Runtime/ABI seam; Core remains the owner of business semantics.",
        path="docs/time_tracer/clients/windows_cli/specs/REFACTORING.md",
        validation=("CLI command tests", "tracer_core_shell integration tests", "Windows CLI black-box suite"),
    ),
    "loc_scanner": Guidance(
        summary="Keep scanner configuration, traversal, reporting, and CLI orchestration explicit; split only when a real responsibility boundary exists.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("LOC Scanner pytest suite", "Profile scan smoke test"),
    ),
    "tidy_workflow": Guidance(
        summary="Confirm workflow phases, state transitions, and failure-recovery boundaries before splitting; do not split a stateful tidy command mechanically by line count.",
        path="docs/tools/toolchain/tidy/architecture.md",
        validation=("Tidy platform tests", "Focused tidy workflow smoke test"),
    ),
    "clang_adapters": Guidance(
        summary="Keep clang tool invocation, protocol translation, and result models at the adapter boundary; split only when a tool protocol or independent test seam is distinct.",
        path="docs/tools/toolchain/tidy/architecture.md",
        validation=("Clang adapter tests", "Focused tidy integration path"),
    ),
}

_CATEGORY_GUIDANCE: dict[str, Guidance] = {
    "presentation": Guidance(
        summary="Separate client parameters, adapters, and presentation; do not duplicate core business rules in the presentation layer.",
        path="docs/time_tracer/architecture/refactoring_guidance.md",
        validation=("Relevant client tests", "Core contract tests"),
    ),
    "tests": Guidance(
        summary="Treat tests as a separate inventory; classify them by smoke, contract, integration, and regression intent before splitting.",
        path=None,
        validation=("Relevant test target",),
    ),
}


def guidance_for(component: str, category: str) -> Guidance:
    # Tests have their own guidance. All production files use the shared
    # refactoring workflow; component-specific details are linked from it.
    if category == "tests":
        return _CATEGORY_GUIDANCE[category]
    if component in _GUIDANCE:
        return _GUIDANCE[component]
    if category in _CATEGORY_GUIDANCE:
        return _CATEGORY_GUIDANCE[category]
    return Guidance(
        summary="Confirm module responsibilities and call boundaries before splitting by responsibility.",
        path=None,
        validation=("Relevant component tests",),
    )


def guidance_payload(guidance: Guidance) -> dict:
    return {
        "summary": guidance.summary,
        "path": guidance.path,
        "validation": list(guidance.validation),
    }


def read_guidance_content(workspace_root: Path, guidance: Guidance) -> str | None:
    if not guidance.path:
        return None
    path = workspace_root / guidance.path
    if not path.exists():
        return None
    return path.read_text(encoding="utf-8")
