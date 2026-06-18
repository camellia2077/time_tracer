# Validation Terms

This glossary section defines correctness and completeness terms.

## Validity

`validity` means whether input or generated data satisfies hard rules.

Validity errors include:

1. Invalid event-line format.
2. Unknown activity token where the input contract requires known tokens.
3. Wake anchor in an invalid position.
4. Overlap.
5. Zero duration.
6. Invalid backward range.

Validity errors may block ingest, conversion, or persistence.

## Completeness

`completeness` means whether the authored record contains enough information for
the expected recording style or authoring workflow.

Completeness is not the same as validity.

Examples:

1. A single point event may be valid but incomplete because its start boundary
   is unknown.
2. A single interval event may be valid and statistically useful because it
   already has start and end boundaries.
3. Sparse interval-event recording may be intentionally incomplete relative to a
   civil-day span while still valid.

Completeness should usually produce authoring warnings, not hard ingest errors.
