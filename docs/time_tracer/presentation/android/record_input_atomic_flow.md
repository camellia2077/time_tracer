# Android Record Input Atomic Flow

## Purpose

Explain the Android `Record Input` business flow from single-line authoring to
official TXT update and database sync.

This page focuses on:

1. what Android validates before record submission
2. what the runtime and core pipeline validate authoritatively
3. how one record becomes a candidate TXT update
4. why the flow is atomic instead of "write TXT first, fix later"

## Scope

This page covers the `Record` tab single-line input flow.

It does not cover:

1. full TXT editor save semantics
2. report rendering behavior
3. config asset packaging details

## Core Idea

Android single-line recording is not a "local append then best-effort sync"
flow.

It is an atomic authoring flow:

`author input -> candidate month TXT -> full core validation -> official TXT replace -> single-month ingest -> success or rollback`

That means:

1. Android may do lightweight authoring checks first
2. the final authority still lives in core atomic record
3. TXT and DB should stay aligned after a successful record action

## Flow Summary

The effective flow is:

1. User types an activity token and optional remark in `Record Input`
2. Android resolves the logical target day (`today` or `yesterday`)
3. Android calls runtime `recordNow(...)`
4. Runtime delegates to core atomic record
5. Core builds a candidate month TXT in memory
6. Core validates the candidate through the ingest pipeline
7. If validation succeeds, core replaces the official TXT and re-imports that month
8. If import fails, core rolls back TXT

## 1. Android Authoring Layer

### 1.1 Authorable Token Set

When the user enters the `Record` or `TXT` tab, Android loads:

`authorable_event_tokens = alias_mapping.keys ∪ wake_keywords`

See:

1. [TracerTabs.kt](/C:/code/time_tracer/apps/android/app/src/main/java/com/example/tracer/ui/screen/tracer/TracerTabs.kt)
2. [QueryGateway.kt](/C:/code/time_tracer/apps/android/contract/src/main/java/com/example/tracer/QueryGateway.kt)

Business meaning:

1. ordinary activity tokens are authorable
2. wake tokens are also authorable
3. Android must not hardcode wake strings locally

### 1.2 What Android Uses This Set For

Android uses that set for lightweight authoring-side checks in:

1. Quick Access search candidates
2. Quick Access save validation
3. suggested-activity click validation

See:

1. [RecordTabContent.kt](/C:/code/time_tracer/apps/android/feature-record/src/main/java/com/example/tracer/ui/screen/RecordTabContent.kt)

Important boundary:

`Record Input` free typing is not finally accepted or rejected by Compose UI
alone. The authoritative acceptance check happens in core atomic record.

### 1.3 Current Haptic Feedback

Current Android `Record` interactions provide haptic feedback in:

1. `Quick Access`: long press to enter drag-reorder triggers one haptic feedback
2. `Record Activity`: triggering the record action provides one haptic feedback

## 2. ViewModel And Use Case Layer

When the user taps record:

1. `RecordViewModel.recordNow()` is called
2. `RecordUseCases.recordNow(...)` resolves the target logical date
3. `RecordUseCases` also resolves the time-order mode

See:

1. [RecordViewModel.kt](/C:/code/time_tracer/apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordViewModel.kt)
2. [RecordUseCases.kt](/C:/code/time_tracer/apps/android/feature-record/src/main/java/com/example/tracer/ui/viewmodel/RecordUseCases.kt)

Current business rule:

1. `TODAY` uses `STRICT_CALENDAR`
2. `YESTERDAY` uses `LOGICAL_DAY_0600`

This allows late-night logical-day recording to stay consistent with the
Record/TXT shared target-day model.

## 3. Runtime Entry

Android runtime receives:

1. `activityName`
2. `remark`
3. `targetDateIso`
4. optional preferred TXT path
5. selected time-order mode

Then [RuntimeRecordDelegate.kt](/C:/code/time_tracer/apps/android/runtime/src/main/java/com/example/tracer/runtime/controller/RuntimeRecordDelegate.kt)
calls the native atomic record bridge instead of maintaining a separate local
"append text then sync" implementation.

This is deliberate:

1. one authority for validation
2. one authority for candidate TXT generation
3. one authority for rollback behavior

## 4. Core Atomic Record Authority

The core authority lives in:

1. [pipeline_record_alias_text_support_impl.inc](/C:/code/time_tracer/libs/tracer_core/src/application/pipeline/detail/pipeline_record_alias_text_support_impl.inc)
2. [pipeline_record_atomic_support_impl.inc](/C:/code/time_tracer/libs/tracer_core/src/application/pipeline/detail/pipeline_record_atomic_support_impl.inc)

### 4.1 Activity Name Validation

Core first validates the raw activity token.

Current rule:

1. token is valid if it is an `alias_mapping.toml` left key, or
2. token is valid if it is in `wake_keywords`

Business meaning:

1. wake classification is config-driven
2. authorable validation is aligned with Android's `authorable_event_tokens`
3. TXT keeps the user-entered raw token text unchanged

The alias-to-canonical mapping still happens later during full TXT parse and
ingest, not during record-time text authoring.

### 4.2 Candidate TXT Construction

Core does not write directly into the official TXT first.

It builds a candidate month TXT in memory:

1. resolve target month and target day marker
2. create the month template if the file does not exist
3. generate current local `HHMM`
4. build one raw event line:
   `HHMM + activity + optional remark`
5. insert that line into the target day block
6. if the day does not exist yet, create the day block

Business meaning:

1. one single-line authoring action is always judged in whole-document context
2. record validation is still month-TXT validation, not isolated line regex validation

### 4.3 Record-Time Checks

During candidate construction, core applies record-time checks such as:

1. activity token must be valid
2. target date must be valid
3. preferred TXT path must stay inside input root
4. new event time must be strictly later than the last event time in that day,
   under the selected time-order mode
5. same-day duplicate `HHMM + activity` yields a warning

If the day remains incomplete after insertion:

1. generic completeness warning may be added
2. overnight continuation warning may replace it when the first authored event
   is not wake-related

These are warnings, not failures.

## 5. Full Candidate Validation

After candidate TXT is built, core runs the candidate through the normal ingest
validation stages:

1. structure validation
2. conversion
3. cross-month link
4. logic validation

This means a single record action is still checked against the full business
rules of the TXT-first ingest model.

Examples of blocking failures:

1. invalid target date
2. invalid activity token
3. impossible time ordering in the target day
4. wake appearing in a non-first semantic position after candidate insertion
5. any other structure/logic rule violated by the candidate month document

## 6. Official TXT Replace And DB Sync

If candidate validation succeeds:

1. core backs up the official TXT if it already exists
2. core writes the candidate content into the official TXT
3. core runs `single_txt_replace_month` ingest for that month

If ingest succeeds:

1. the action is committed
2. TXT and DB are both updated

If ingest fails:

1. core restores the old TXT from backup
2. the record action fails
3. TXT and DB remain consistent with the pre-action state

This is why the flow is called atomic in practice.

## 7. What The Single Line Means Business-Wise

In Android `Record Input`, the user only authors:

1. activity token
2. optional remark

The runtime supplies:

1. current `HHMM`
2. target logical day
3. target month TXT path

So the single-line action means:

1. append one new event point into the logical day
2. re-evaluate the whole month TXT under ingest rules
3. if valid, persist the updated month and sync DB

The line itself is not yet a complete duration fact.

Its final duration semantics still depend on:

1. previous event time in the day
2. cross-day previous context
3. wake semantics
4. generated activities such as `sleep_night`

## 8. Android Vs Core Responsibility Boundary

### 8.1 Android Responsibilities

Android is responsible for:

1. collecting author input
2. loading authorable token sets from core/runtime
3. lightweight authoring-side validation for Quick Access and suggestions
4. resolving target logical day and time-order mode
5. surfacing success, warning, and failure messages

### 8.2 Core Responsibilities

Core is responsible for:

1. authoritative activity-token validation
2. candidate TXT construction
3. month-document validation
4. official TXT update
5. month re-import into DB
6. rollback on failure

This boundary is intentional:

1. Android stays thin
2. business logic remains centralized
3. TXT-first semantics are preserved across CLI, Android, and future surfaces

## 9. Why This Design Exists

This design avoids several failure modes:

1. Android and core disagreeing on what counts as a valid activity token
2. Android appending text locally that core later rejects
3. official TXT changing while DB stays stale
4. partial success where a record appears in TXT but not in DB
5. duplicated business logic across UI and ingest codepaths

## 10. Related Documents

For adjacent topics, open:

1. [runtime-protocol.md](/C:/code/time_tracer/docs/time_tracer/presentation/android/runtime-protocol.md)
2. [txt_to_db_business_logic.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/txt_to_db_business_logic.md)
3. [record_input_and_day_completeness_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/record_input_and_day_completeness_semantics.md)
4. [day_bucket_and_wake_anchor_semantics.md](/C:/code/time_tracer/docs/time_tracer/core/ingest/day_bucket_and_wake_anchor_semantics.md)
