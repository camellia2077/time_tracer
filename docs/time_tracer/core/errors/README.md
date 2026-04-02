# Core Errors Docs

This directory is the canonical home for core-wide error semantics,
machine-readable error codes, and shared diagnostics/logging contracts.

## Put Docs Here When
1. The document defines shared error behavior used by more than one capability.
2. The document defines stable machine-readable error fields or code vocabularies.
3. The document is about diagnostics/logging ownership rather than a single
   capability workflow.

## Current Docs
1. [error-codes.md](error-codes.md)
   - Shared error-code vocabulary, naming rules, and mapping guidance.
2. [error-model.md](error-model.md)
   - Shared error/logging model across runtime hosts.

## Read Next
1. [../shared/c_abi.md](../shared/c_abi.md)
   - Stable `tracer_core_*` ABI envelope fields that carry `error_code`,
     `error_category`, and `hints`.
2. [../capabilities/validation/diagnostics.md](../capabilities/validation/diagnostics.md)
   - Validation-owned interpretation of the shared error vocabulary.
